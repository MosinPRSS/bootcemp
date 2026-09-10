#include "optimizer.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <memory>
#include <numeric>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "ortools/sat/cp_model.h"
#include "ortools/sat/cp_model.pb.h"
#include "ortools/sat/cp_model_solver.h"
#include "ortools/sat/model.h"
#include "ortools/sat/sat_parameters.pb.h"
#include "ortools/util/sorted_interval_list.h"

namespace transport {
namespace {

    using json = nlohmann::json;
    using operations_research::Domain;
    using operations_research::sat::CpModelBuilder;
    using operations_research::sat::CpSolverResponse;
    using operations_research::sat::CpSolverStatus;
    using operations_research::sat::IntVar;
    using operations_research::sat::LinearExpr;
    using operations_research::sat::Model;
    using operations_research::sat::NewSatParameters;
    using operations_research::sat::SatParameters;
    using operations_research::sat::SolutionIntegerValue;
    using operations_research::sat::SolveCpModel;

    constexpr int64_t kShiftLimitMinutes = 8 * 60;

    // Математическая модель и переменные, значения которых подбирает решатель.
    struct ModelState {
        CpModelBuilder model; // Модель CP-SAT: переменные, ограничения и целевая функция.
        std::vector<std::vector<IntVar>> trips; // Переменные числа рейсов: [слот отправления][модель автобуса].
        std::vector<IntVar> shortage; // Переменные дефицита пассажиров по часам; пусто в модели закупки.
        std::vector<IntVar> buy; // Переменные числа закупаемых автобусов по моделям; пусто для текущего парка.
    };

    // Результат расчета; при отсутствии допустимого решения массивы остаются пустыми.
    struct PlanSolution {
        CpSolverStatus status = CpSolverStatus::UNKNOWN; // Статус решения, из которого получен возвращаемый план.
        std::vector<std::vector<int64_t>> trips; // Число рейсов: [час 0–23][модель автобуса].
        std::vector<std::vector<int64_t>> departures; // Число рейсов: [слот отправления][модель автобуса].
        std::vector<int64_t> shortage; // Количество неперевезенных пассажиров по часам.
        std::vector<int64_t> buy; // Количество дополнительных автобусов по моделям.
        CpSolverStatus primary_status = CpSolverStatus::UNKNOWN; // Статус минимизации дефицита или количества закупок.
        CpSolverStatus cost_status = CpSolverStatus::UNKNOWN; // Статус этапа минимизации стоимости.
        bool optimality_proven = false; // Доказана ли оптимальность обоих этапов расчета.
        int64_t total_shortage = 0; // Общее количество неперевезенных пассажиров за сутки.
        int64_t total_cost_kopecks = 0; // Общая эксплуатационная стоимость рейсов, копейки.
    };

    // Проверяет, найдено ли допустимое решение.
    // Принимает: Статус решателя CP-SAT.
    // Возвращает: true для OPTIMAL или FEASIBLE, иначе false.
    bool IsFeasible(const CpSolverStatus status) {
        return status == CpSolverStatus::OPTIMAL ||
            status == CpSolverStatus::FEASIBLE;
    }

    // Преобразует статус решателя в строку для JSON.
    // Принимает: Статус CP-SAT.
    // Возвращает: Имя статуса; для остальных значений UNKNOWN.
    std::string StatusName(const CpSolverStatus status) {
        switch (status) {
        case CpSolverStatus::OPTIMAL:
            return "OPTIMAL";
        case CpSolverStatus::FEASIBLE:
            return "FEASIBLE";
        case CpSolverStatus::INFEASIBLE:
            return "INFEASIBLE";
        case CpSolverStatus::MODEL_INVALID:
            return "MODEL_INVALID";
        default:
            return "UNKNOWN";
        }
    }

    // Формирует подпись часового интервала.
    // Принимает: Час начала от 0 до 23.
    // Возвращает: Строку вида 06-07.
    std::string HourLabel(const int hour) {
        std::ostringstream out;
        out << std::setfill('0') << std::setw(2) << hour << '-'
            << std::setfill('0') << std::setw(2) << (hour + 1);
        return out.str();
    }

    // Рассчитывает стоимость рейса с учетом длительности, региона и сезона.
    // Принимает: Входные данные и описание модели транспорта.
    // Возвращает: Стоимость одного рейса в копейках с округлением.
    int64_t TripCostKopecks(const InputData& data, const FleetType& type) {
        const long double adjusted =
            static_cast<long double>(type.base_cost_kopecks_per_hour) *
            static_cast<long double>(data.route_minutes) / 60.0L *
            static_cast<long double>(data.region_multiplier) *
            static_cast<long double>(data.season_multiplier);
        return static_cast<int64_t>(std::llround(adjusted));
    }

    // Суммирует пассажиропоток за сутки.
    // Принимает: Входные данные с уже скорректированным спросом.
    // Возвращает: Общее число пассажиров.
    int64_t TotalDemand(const InputData& data) {
        int64_t result = 0;
        for (const int64_t value : data.demand) {
            result += value;
        }
        return result;
    }

    // Вычисляет верхнюю границу количества рейсов по спросу и наименьшей вместимости.
    // Принимает: Входные данные с непустым парком и положительной вместимостью; индекс часа.
    // Возвращает: Ноль при отсутствии спроса, иначе округленное вверх отношение спроса к вместимости.
    int64_t MaxTripsAtHour(const InputData& data, const int hour) {
        if (data.demand[hour] == 0) {
            return 0;
        }
        int64_t min_capacity = std::numeric_limits<int64_t>::max();
        for (const auto& type : data.fleet) {
            min_capacity = std::min(min_capacity, type.capacity);
        }
        return (data.demand[hour] + min_capacity - 1) / min_capacity;
    }

    // Определяет сетку отправлений, совместимую с часовыми границами и длительностью рейса.
    // Принимает: Входные данные с положительной длительностью рейса в минутах.
    // Возвращает: НОД длительности рейса и 60 минут.
    int DepartureStepMinutes(const InputData& data) {

        return static_cast<int>(std::gcd(data.route_minutes, int64_t{60}));
    }

    // Вычисляет число моментов отправления внутри часа.
    // Принимает: Входные данные с положительной длительностью рейса.
    // Возвращает: Число слотов за час.
    int SlotsPerHour(const InputData& data) {
        return 60 / DepartureStepMinutes(data);
    }

    // Форматирует время отправления.
    // Принимает: Число минут от начала суток.
    // Возвращает: Строку вида 07:30.
    std::string DepartureLabel(const int minute) {
        std::ostringstream out;
        out << std::setfill('0') << std::setw(2) << minute / 60 << ':'
            << std::setw(2) << minute % 60;
        return out.str();
    }

    // Создает целочисленные переменные числа рейсов для каждого слота и модели автобуса.
    // Принимает: Входные данные и изменяемое состояние модели.
    // Возвращает: Ничего; заполняет state.trips и добавляет переменные в state.model.
    void AddDepartureVariables(const InputData& data, ModelState& state) {
        const int slots_per_hour = SlotsPerHour(data);
        const int slot_count = kHoursPerDay * slots_per_hour;
        state.trips.resize(slot_count);
        for (int slot = 0; slot < slot_count; ++slot) {
            const int64_t upper = MaxTripsAtHour(data, slot / slots_per_hour);
            for (int m = 0; m < static_cast<int>(data.fleet.size()); ++m) {
                state.trips[slot].push_back(
                    state.model.NewIntVar(Domain(0, upper))
                    .WithName("trips_" + std::to_string(slot) + "_" + std::to_string(m)));
            }
        }
    }

    // Строит выражение суммарной провозной способности за час.
    // Принимает: Входные данные, состояние модели с переменными рейсов и индекс часа.
    // Возвращает: Линейное выражение вместимости всех отправлений за час.
    LinearExpr HourCapacity(const InputData& data, const ModelState& state,
        const int hour) {
        LinearExpr served;
        const int slots_per_hour = SlotsPerHour(data);
        for (int slot = hour * slots_per_hour; slot < (hour + 1) * slots_per_hour; ++slot) {
            for (int m = 0; m < static_cast<int>(data.fleet.size()); ++m) {
                served += data.fleet[m].capacity * state.trips[slot][m];
            }
        }
        return served;
    }

    // Ограничивает одновременную занятость автобусов и суммарную работу парка: 480 минут на автобус в сутки. Автобус доступен в момент завершения рейса.
    // Принимает: Входные данные и изменяемую модель; учитывает закупку, если state.buy заполнен.
    // Возвращает: Ничего; добавляет ограничения занятости и суточного бюджета работы.
    void AddFleetLimits(const InputData& data, ModelState& state) {
        const int slot_count = static_cast<int>(state.trips.size());
        const int64_t duration_slots = data.route_minutes / DepartureStepMinutes(data);
        for (int m = 0; m < static_cast<int>(data.fleet.size()); ++m) {
            LinearExpr available(data.fleet[m].max_available);
            if (!state.buy.empty()) {
                available += state.buy[m];
            }


            for (int check = 0; check < slot_count; ++check) {
                LinearExpr active;
                const int first = static_cast<int>(std::max<int64_t>(0, check - duration_slots + 1));
                for (int departure = first; departure <= check; ++departure) {
                    active += state.trips[departure][m];
                }
                state.model.AddLessOrEqual(active, available);
            }

            LinearExpr daily_trips;
            for (int slot = 0; slot < slot_count; ++slot) {
                daily_trips += state.trips[slot][m];
            }
            state.model.AddLessOrEqual(data.route_minutes * daily_trips,
                kShiftLimitMinutes * available);
        }
    }

    // Строит выражение общей эксплуатационной стоимости.
    // Принимает: Входные данные и состояние модели с переменными рейсов.
    // Возвращает: Линейное выражение стоимости в копейках.
    LinearExpr OperatingCost(const InputData& data, const ModelState& state) {
        LinearExpr cost;
        for (const auto& departure : state.trips) {
            for (int m = 0; m < static_cast<int>(data.fleet.size()); ++m) {
                cost += TripCostKopecks(data, data.fleet[m]) * departure[m];
            }
        }
        return cost;
    }

    // Запускает CP-SAT с заданными лимитом времени и числом потоков.
    // Принимает: Построенную модель и входные настройки; лимит времени применяется к этому запуску.
    // Возвращает: Ответ решателя со статусом и значениями переменных.
    CpSolverResponse Solve(const CpModelBuilder& builder, const InputData& data) {
        SatParameters parameters;
        parameters.set_max_time_in_seconds(data.time_limit_seconds);
        parameters.set_num_search_workers(data.workers);
        parameters.set_log_search_progress(false);
        parameters.set_log_to_stdout(false);

        Model model;
        model.Add(NewSatParameters(parameters));
        return SolveCpModel(builder.Build(), &model);
    }

    // Строит модель текущего парка для минимизации дефицита либо стоимости.
    // Принимает: Данные, флаг minimize_shortage и необязательное фиксированное значение дефицита fixed_shortage.
    // Возвращает: Уникальный указатель на модель с переменными рейсов и дефицита.
    std::unique_ptr<ModelState> BuildCurrentFleetModel(
        const InputData& data, const bool minimize_shortage,
        const std::optional<int64_t> fixed_shortage) {
        auto state = std::make_unique<ModelState>();
        AddDepartureVariables(data, *state);
        LinearExpr total_shortage;
        for (int hour = 0; hour < kHoursPerDay; ++hour) {
            state->shortage.push_back(
                state->model.NewIntVar(Domain(0, data.demand[hour]))
                .WithName("shortage_" + std::to_string(hour)));
            state->model.AddGreaterOrEqual(
                HourCapacity(data, *state, hour) + state->shortage[hour], data.demand[hour]);
            total_shortage += state->shortage[hour];
        }
        AddFleetLimits(data, *state);
        if (fixed_shortage.has_value()) {
            state->model.AddEquality(total_shortage, *fixed_shortage);
        }
        state->model.Minimize(minimize_shortage ? total_shortage : OperatingCost(data, *state));
        return state;
    }
    // Извлекает количества рейсов, дефицит, закупку и стоимость из ответа решателя.
    // Принимает: Входные данные, состояние решенной модели и соответствующий ответ CP-SAT.
    // Возвращает: PlanSolution; при отсутствии допустимого решения массивы остаются пустыми.
    PlanSolution DecodeSolution(const InputData& data, const ModelState& state,
        const CpSolverResponse& response) {
        PlanSolution result;
        result.status = response.status();
        if (!IsFeasible(result.status)) {
            return result;
        }

        const int model_count = static_cast<int>(data.fleet.size());
        result.trips.assign(kHoursPerDay, std::vector<int64_t>(model_count, 0));
        result.departures.assign(state.trips.size(), std::vector<int64_t>(model_count, 0));
        result.shortage.assign(kHoursPerDay, 0);

        for (int hour = 0; hour < kHoursPerDay; ++hour) {
            if (!state.shortage.empty()) {
                result.shortage[hour] =
                    SolutionIntegerValue(response, state.shortage[hour]);
                result.total_shortage += result.shortage[hour];
            }
        }
        for (int slot = 0; slot < static_cast<int>(state.trips.size()); ++slot) {
            const int hour = slot / SlotsPerHour(data);
            for (int model_index = 0; model_index < model_count; ++model_index) {
                const int64_t value =
                    SolutionIntegerValue(response, state.trips[slot][model_index]);
                result.departures[slot][model_index] = value;
                result.trips[hour][model_index] += value;
                result.total_cost_kopecks +=
                    value * TripCostKopecks(data, data.fleet[model_index]);
            }
        }
        for (const auto& variable : state.buy) {
            result.buy.push_back(SolutionIntegerValue(response, variable));
        }
        return result;
    }

    // Сначала минимизирует дефицит, затем стоимость при доказанном минимуме дефицита. При неудаче второго этапа сохраняет первый план.
    // Принимает: Подготовленные входные данные.
    // Возвращает: План текущего парка со статусами этапов и признаком доказанной оптимальности.
    PlanSolution SolveCurrentFleet(const InputData& data) {
        auto first = BuildCurrentFleetModel(data, true, std::nullopt);
        const CpSolverResponse first_response = Solve(first->model, data);
        if (!IsFeasible(first_response.status())) {
            PlanSolution result;
            result.status = first_response.status(); result.primary_status = first_response.status();
            return result;
        }
        PlanSolution shortage_solution =
            DecodeSolution(data, *first, first_response);
        shortage_solution.primary_status = first_response.status();



        if (first_response.status() != CpSolverStatus::OPTIMAL) {
            return shortage_solution;
        }

        auto second =
            BuildCurrentFleetModel(data, false, shortage_solution.total_shortage);
        const CpSolverResponse second_response = Solve(second->model, data);
        if (!IsFeasible(second_response.status())) {
            shortage_solution.cost_status = second_response.status();
            return shortage_solution;
        }
        PlanSolution result = DecodeSolution(data, *second, second_response);
        result.primary_status = first_response.status();
        result.cost_status = second_response.status();
        result.optimality_proven = result.primary_status == CpSolverStatus::OPTIMAL &&
            result.cost_status == CpSolverStatus::OPTIMAL;
        return result;
    }

    // Оценивает верхнюю границу закупки через сумму необходимых рейсов при минимальной вместимости.
    // Принимает: Входные данные с непустым парком и положительной вместимостью.
    // Возвращает: Верхнюю границу количества дополнительных автобусов, не менее 1.
    int64_t PurchaseUpperBound(const InputData& data) {
        int64_t min_capacity = std::numeric_limits<int64_t>::max();
        for (const auto& type : data.fleet) {
            min_capacity = std::min(min_capacity, type.capacity);
        }
        int64_t upper = 0;
        for (const int64_t demand : data.demand) {
            upper += (demand + min_capacity - 1) / min_capacity;
        }
        return std::max<int64_t>(upper, 1);
    }

    // Строит модель полного покрытия спроса с возможностью закупки автобусов.
    // Принимает: Данные, флаг minimize_purchase_count и необязательное фиксированное количество закупок fixed_purchase_count.
    // Возвращает: Уникальный указатель на модель с переменными рейсов и закупки.
    std::unique_ptr<ModelState> BuildPurchaseModel(
        const InputData& data, const bool minimize_purchase_count,
        const std::optional<int64_t> fixed_purchase_count) {
        auto state = std::make_unique<ModelState>();
        const int64_t buy_upper = PurchaseUpperBound(data);
        LinearExpr total_purchase;
        for (int m = 0; m < static_cast<int>(data.fleet.size()); ++m) {
            state->buy.push_back(state->model.NewIntVar(Domain(0, buy_upper))
                .WithName("buy_" + std::to_string(m)));
            total_purchase += state->buy[m];
        }
        AddDepartureVariables(data, *state);
        for (int hour = 0; hour < kHoursPerDay; ++hour) {
            state->model.AddGreaterOrEqual(HourCapacity(data, *state, hour), data.demand[hour]);
        }
        AddFleetLimits(data, *state);
        if (fixed_purchase_count.has_value()) {
            state->model.AddEquality(total_purchase, *fixed_purchase_count);
        }
        state->model.Minimize(minimize_purchase_count ? total_purchase : OperatingCost(data, *state));
        return state;
    }
    // Сначала минимизирует число закупаемых автобусов, затем стоимость при доказанном минимуме закупки. При неудаче второго этапа сохраняет первый план.
    // Принимает: Подготовленные входные данные.
    // Возвращает: План расширенного парка со статусами этапов и признаком доказанной оптимальности.
    PlanSolution SolvePurchase(const InputData& data) {
        auto first = BuildPurchaseModel(data, true, std::nullopt);
        const CpSolverResponse first_response = Solve(first->model, data);
        if (!IsFeasible(first_response.status())) {
            PlanSolution result;
            result.status = first_response.status(); result.primary_status = first_response.status();
            return result;
        }
        PlanSolution count_solution = DecodeSolution(data, *first, first_response);
        count_solution.primary_status = first_response.status();
        const int64_t purchase_count =
            std::accumulate(count_solution.buy.begin(), count_solution.buy.end(),
                int64_t{ 0 });
        if (first_response.status() != CpSolverStatus::OPTIMAL) {
            return count_solution;
        }

        auto second = BuildPurchaseModel(data, false, purchase_count);
        const CpSolverResponse second_response = Solve(second->model, data);
        if (!IsFeasible(second_response.status())) {
            count_solution.cost_status = second_response.status();
            return count_solution;
        }
        PlanSolution result = DecodeSolution(data, *second, second_response);
        result.primary_status = first_response.status();
        result.cost_status = second_response.status();
        result.optimality_proven = result.primary_status == CpSolverStatus::OPTIMAL &&
            result.cost_status == CpSolverStatus::OPTIMAL;
        return result;
    }

    // Формирует расписание по часам с ненулевым спросом, включая отправления каждой модели.
    // Принимает: Входные данные и рассчитанный план.
    // Возвращает: JSON-массив расписания; пустой массив, если допустимое решение не найдено.
    json PlanToJson(const InputData& data, const PlanSolution& solution) {
        json result = json::array();
        if (!IsFeasible(solution.status)) {
            return result;
        }
        for (int hour = 0; hour < kHoursPerDay; ++hour) {
            if (data.demand[hour] == 0) {
                continue;
            }
            json row;
            row["interval"] = HourLabel(hour);
            row["demand"] = data.demand[hour];
            row["shortage"] = solution.shortage.empty() ? 0 : solution.shortage[hour];
            row["assigned"] = json::array();
            int64_t offered_capacity = 0;
            for (int model_index = 0;
                model_index < static_cast<int>(data.fleet.size()); ++model_index) {
                const int64_t trips = solution.trips[hour][model_index];
                if (trips == 0) {
                    continue;
                }
                offered_capacity += trips * data.fleet[model_index].capacity;
                json departures = json::array();
                const int slots_per_hour = SlotsPerHour(data);
                for (int slot = hour * slots_per_hour; slot < (hour + 1) * slots_per_hour; ++slot) {
                    const int64_t count = solution.departures[slot][model_index];
                    if (count > 0) {
                        departures.push_back({
                            {"time", DepartureLabel(slot * DepartureStepMinutes(data))},
                            {"trips", count},
                        });
                    }
                }
                row["assigned"].push_back({
                    {"model", data.fleet[model_index].model},
                    {"trips", trips},
                    {"capacity", data.fleet[model_index].capacity},
                    {"offered_capacity", trips * data.fleet[model_index].capacity},
                    {"departures", std::move(departures)},
                    });
            }
            row["offered_capacity"] = offered_capacity;
            result.push_back(std::move(row));
        }
        return result;
    }

    // Формирует согласованный выходной JSON со статусами, итогами, расписанием, использованием парка и доступной рекомендацией по закупке.
    // Принимает: Входные данные, план текущего парка и необязательный план закупки.
    // Возвращает: JSON результата; при отсутствии текущего решения только статусы, параметры и сообщение.
    json BuildOutput(const InputData& data, const PlanSolution& current,
        const std::optional<PlanSolution>& purchase) {
        json output;
        output["status"] = StatusName(current.status);
        output["primary_status"] = StatusName(current.primary_status);
        output["cost_status"] = StatusName(current.cost_status);
        output["optimality_proven"] = current.optimality_proven;
        output["parameters"] = {
            {"route_cycle_minutes", data.route_minutes},
            {"daily_coefficient", data.daily_coefficient},
            {"region_multiplier", data.region_multiplier},
            {"season_multiplier", data.season_multiplier},
        };

        if (!IsFeasible(current.status)) {
            output["message"] = "Решение не найдено";
            return output;
        }

        const int64_t total_demand = TotalDemand(data);
        output["summary"] = {
            {"total_demand", total_demand},
            {"transported_passengers", total_demand - current.total_shortage},
            {"shortage_passengers", current.total_shortage},
            {"all_demand_satisfied", current.total_shortage == 0},
            {"operating_cost_rub", current.total_cost_kopecks / 100.0},
        };
        output["schedule"] = PlanToJson(data, current);

        output["fleet_usage"] = json::array();
        for (int model_index = 0;
            model_index < static_cast<int>(data.fleet.size()); ++model_index) {
            int64_t trips = 0;
            for (int hour = 0; hour < kHoursPerDay; ++hour) {
                trips += current.trips[hour][model_index];
            }
            output["fleet_usage"].push_back({
                {"model", data.fleet[model_index].model},
                {"max_available", data.fleet[model_index].max_available},
                {"base_cost_per_hour",
                 data.fleet[model_index].base_cost_kopecks_per_hour / 100.0},
                {"trips", trips},
                {"busy_minutes", trips * data.route_minutes},
                {"operating_cost_rub",
                 trips * TripCostKopecks(data, data.fleet[model_index]) / 100.0},
                });
        }

        if (purchase.has_value() && IsFeasible(purchase->status)) {
            json items = json::array();
            int64_t total_to_buy = 0;
            for (int model_index = 0;
                model_index < static_cast<int>(data.fleet.size()); ++model_index) {
                if (purchase->buy[model_index] == 0) {
                    continue;
                }
                total_to_buy += purchase->buy[model_index];
                items.push_back({
                    {"model", data.fleet[model_index].model},
                    {"additional_buses", purchase->buy[model_index]},
                    });
            }
            output["purchase_recommendation"] = {
                {"status", StatusName(purchase->status)},
                {"optimality_proven", purchase->optimality_proven},
                {"total_additional_buses", total_to_buy},
                {"items", items},
            };
        }
        return output;
    }

}

// Проверяет основные параметры и рассчитывает текущий план и рекомендацию по закупке.
// Принимает: Подготовленные InputData по контракту optimizer.h.
// Возвращает: JSON; при ошибочных параметрах выбрасывает std::invalid_argument.
nlohmann::json Optimize(const InputData& data) {
    if (data.fleet.empty() || data.route_minutes <= 0 ||
        !std::isfinite(data.time_limit_seconds) || data.time_limit_seconds <= 0 ||
        data.workers <= 0 ||
        !std::isfinite(data.daily_coefficient) || data.daily_coefficient < 0 ||
        !std::isfinite(data.region_multiplier) || data.region_multiplier < 0 ||
        !std::isfinite(data.season_multiplier) || data.season_multiplier < 0) {
        throw std::invalid_argument("Invalid optimizer parameters");
    }
    for (const auto& type : data.fleet) {
        if (type.capacity <= 0 || type.max_available < 0 || type.base_cost_kopecks_per_hour < 0) {
            throw std::invalid_argument("Invalid fleet parameters");
        }
    }
    for (const auto demand : data.demand) {
        if (demand < 0) {
            throw std::invalid_argument("Demand must be nonnegative");
        }
    }
    const PlanSolution current = SolveCurrentFleet(data);
    std::optional<PlanSolution> purchase;
    if (IsFeasible(current.status) && current.total_shortage > 0) {
        purchase = SolvePurchase(data);
    }
    return BuildOutput(data, current, purchase);
}
}