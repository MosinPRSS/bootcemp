#include "bridge.hpp"
#include "optimizer.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

using namespace transport;

using json = nlohmann::json;

namespace {

const double WINTER_COEF = 1.1;
const double FAR_COEF = 1.3;

int ParseHour(const std::string& interval) {
    const auto separator = interval.find('-');

    if (separator == std::string::npos) {
        throw std::invalid_argument(
            "Invalid time interval: " + interval
        );
    }

    const std::string hour_string =
        interval.substr(0, separator);

    if (hour_string.empty()) {
        throw std::invalid_argument(
            "Invalid time interval: " + interval
        );
    }

    const int hour = std::stoi(hour_string);

    if (hour < 0 || hour >= kHoursPerDay) {
        throw std::invalid_argument(
            "Invalid hour: " + interval
        );
    }

    return hour;
}

} // namespace

const char* exec_math_magic(const char* raw_json) {
    try {
        if (raw_json == nullptr) {
            throw std::invalid_argument(
                "Input JSON is null"
            );
        }

        const json input = json::parse(raw_json);

        InputData data;

        // ==================================
        // Fleet
        // ==================================

        const auto& fleet = input.at("fleet");

        if (!fleet.is_array()) {
            throw std::invalid_argument(
                "'fleet' must be an array"
            );
        }

        for (const auto& item : fleet) {
            FleetType type;

            type.model =
                item.at("model").get<std::string>();

            type.vehicle_class =
                item.at("class").get<std::string>();

            type.max_available =
                item.at("max_available").get<int64_t>();

            type.capacity =
                item.at("capacity").get<int64_t>();

            // JSON содержит РУБЛИ.
            // InputData ожидает КОПЕЙКИ.
            const double cost_rubles =
                item.at("base_cost_per_hour").get<double>();

            if (!std::isfinite(cost_rubles) ||
                cost_rubles < 0) {
                throw std::invalid_argument(
                    "Invalid base_cost_per_hour"
                );
            }

            type.base_cost_kopecks_per_hour =
                static_cast<int64_t>(
                    std::llround(cost_rubles * 100.0)
                );

            data.fleet.push_back(std::move(type));
        }

        // ==================================
        // Modifiers
        // ==================================

        const auto& modifiers =
            input.at("modifiers");

        data.region =
            modifiers.at("region").get<std::string>();

        data.season =
            modifiers.at("season").get<std::string>();

        if (data.region == "Крайний Север" || data.region == "Дальний Восток") {
            data.region_multiplier = FAR_COEF;
        }

        if (data.season == "Зима") {
            data.season_multiplier = WINTER_COEF;
        }

        const double route_cycle_hours =
            modifiers
                .at("route_cycle_hours")
                .get<double>();

        if (!std::isfinite(route_cycle_hours) ||
            route_cycle_hours <= 0) {
            throw std::invalid_argument(
                "Invalid route_cycle_hours"
            );
        }

        data.route_minutes =
            static_cast<int64_t>(
                std::llround(route_cycle_hours * 60.0)
            );

        // ==================================
        // Schedule
        // ==================================

        const auto& schedule =
            input.at("schedule");

        data.day_of_week =
            schedule.at("day_of_week")
                .get<std::string>();

        data.daily_coefficient =
            schedule.at("daily_coefficient")
                .get<double>();

        // ==================================
        // Demand
        // ==================================

        data.demand.fill(0);

        const auto& hourly_flow =
            schedule.at("hourly_flow");

        if (!hourly_flow.is_object()) {
            throw std::invalid_argument(
                "'hourly_flow' must be an object"
            );
        }

        for (const auto& [interval, value] :
             hourly_flow.items()) {

            const int hour = ParseHour(interval);

            const int64_t demand =
                static_cast<int64_t>(std::llround(
                    value.get<double>() * data.daily_coefficient
                ));

            if (demand < 0) {
                throw std::invalid_argument(
                    "Demand must be nonnegative"
                );
            }

            data.demand[hour] = demand;
        }

        // ==================================
        // Optimize
        // ==================================

        const json result = Optimize(data);

        // ==================================
        // JSON -> C string
        // ==================================

        const std::string output =
            result.dump();

        char* buffer =
            new char[output.size() + 1];

        std::copy(
            output.begin(),
            output.end(),
            buffer
        );

        buffer[output.size()] = '\0';

        return buffer;
    }
    catch (...) {
        return nullptr;
    }
}

void free_mem(char* ptr) {
    delete[] ptr;
}