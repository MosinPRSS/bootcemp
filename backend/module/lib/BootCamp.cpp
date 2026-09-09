#include "header.h"

// Используется для проверки корректности полученных данных
#define DEBUG

// Три основные переменные с данными:
// - Массив ТС vechiclePark
// - Модификаторы стоимости modifiers
// - Расписание пассажиропотока schedule
int main() {


    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    setlocale(LC_ALL, "ru_RU.UTF-8");

    try {

        // Для работы с json-файлом
        Json::Value root;
        Json::CharReaderBuilder builder;
        JSONCPP_STRING errs;

        // Пока берём из текущей папки
        ifstream file("data.json");

        // Проверка на корректное открытие json-файла
        bool successOpen = Json::parseFromStream(builder, file, &root, &errs);


        if (!successOpen) {
            throw runtime_error(errs);
        }

        // В файле нет машинопарка
        if (!root.isMember("fleet") || !root["fleet"].isArray()) {
            throw runtime_error(u8"в JSON-файле не указан парк машин 'fleet'");
        }

        const Json::Value& fleet = root["fleet"];


        // Основная переменная со всеми машинами
        vector<Car> vechiclePark(fleet.size());

        size_t counter = 0; // Счётчик для занесения записей о машинах в вектор

        for (const Json::Value& bus : fleet) {
            
            string name = bus.get("model", DEF).asString();
            string size = bus.get("class", DEF).asString();

            int amount = bus.get("max_available", DEFN).asInt();
            int capacity = bus.get("capacity", DEFN).asInt();
            int cost = bus.get("base_cost_per_hour", DEFN).asInt();

            if (name == DEF || size == DEF || amount == DEFN || capacity == DEFN || cost == DEFN) {
                string error = u8"ошибка в описании параметров машины, порядковый номер в списке - " + (to_string(counter + 1));
                throw runtime_error(error);
            }

            Car car = Car(name, size, amount, capacity, cost);

            vechiclePark[counter++] = car;

        }

        if (!root.isMember("modifiers")) {
            throw runtime_error(u8"в json-файле раздел особых условий 'modifiers' повреждён или отсутствует");
        }

        const Json::Value& mods = root["modifiers"];

        const string region = mods.get("region", DEF).asString();
        const string season = mods.get("season", DEF).asString();
        const double cycle = mods.get("route_cycle_hours", DEFD).asDouble();

        if (region == DEF || season == DEF || cycle == DEFD) {
            throw runtime_error(u8"ошибка данных в разделе особых условий 'modifiers'");
        }

        // Основная переменная с модификаторами на стоимость поездки
        Modifiers modifiers = Modifiers(region, season, cycle);

        if (!root.isMember("schedule") || !root["schedule"].isObject()) {
            throw runtime_error(u8"в json-файле раздел с расписанием 'schedule' повреждён или отсутствует");
        }

        const Json::Value& sched = root["schedule"];

        const string dayOfWeek = sched.get("day_of_week", DEF).asString();
        const double coef = sched.get("daily_coefficient", DEFD).asDouble();

        if (dayOfWeek == DEF || coef == DEFD) {
            throw runtime_error(u8"ошибка в разделе расписания");
        }

        if (!sched.isMember("hourly_flow") || !sched["hourly_flow"].isObject()) {
            throw runtime_error(u8"раздел 'hourly_flow' в 'schedule' повреждён или отсутствует");
        }

        const Json::Value& flow = sched["hourly_flow"];

        vector<pair<string, int>> hourly_flow(flow.size());

        counter = 0; // Переиспользуем переменную

        for (const string& key : flow.getMemberNames()) {

            hourly_flow[counter].first = key;
            hourly_flow[counter++].second = flow.get(key, DEFN).asInt();
            if (hourly_flow[counter - 1].second == DEFN) {
                throw runtime_error(u8"количество людей в 'hourly_flow' не является целым числом");
            }

        }

        // Основная переменная с параметрами расписания
        Schedule schedule = Schedule(dayOfWeek, coef, hourly_flow);

        // Вывел вперёд, потому что компилятор жалуется, что у try нет обработчика
        #ifdef DEBUG
//   <- типа эту полосочку разрезаю своим #ifdef

        cout << u8"Машины:" << endl;

        for (int i = 0; i < vechiclePark.size(); ++i) {

            cout << u8"Название модели: " << vechiclePark[i].modelName << u8"; Класс машины: " << vechiclePark[i].carSize << u8"; Количество машин: " << vechiclePark[i].amountOfCars \
                << u8"; Вместимость салона: " << vechiclePark[i].innerCapacity << u8"; Цена за час езды: " << vechiclePark[i].driveCost << endl << endl;

        }

        cout << endl << u8"Модификаторы:" << endl;

        cout << u8"Регион: " << modifiers.region << u8"\nСезон: " << modifiers.season << u8"\nДлительность рейса: " << modifiers.route_cycle_hours << endl << endl;

        cout << u8"Расписание пассажиропотока:" << endl;

        cout << u8"День недели: " << schedule.dayOfWeek << u8"\nКоэффициент: " << schedule.dailyCoefficient << endl << endl;

        for (int i = 0; i < schedule.hourly_flow.size(); ++i) {

            cout << u8"Часы: " << schedule.hourly_flow[i].first << u8" Количество людей: " << schedule.hourly_flow[i].second << endl << endl;

        }

        cout << endl;

        #endif


        // Блок алгоса


        // Блок формирования выходного JSON-файла

    }
    catch (exception& e) {

        cerr << u8"Ошибка: " << e.what() << endl;

    }
    


}