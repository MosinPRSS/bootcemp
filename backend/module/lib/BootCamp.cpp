#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <regex>
#include <utility>
#include <locale.h>
#include <json/json.h>

using namespace std;

// Сейчас используется для проверки корректности полученных данных
#define DEBUG

// Если при попытке взять взначение по ключю в json-файле не удаётся, будет это значение по умолчанию
#define DEF "default" 

// Wrond String Data - ошибка при получении строкового параметра
#define WSD "unknown"

// Wrong Number Data - ошибка при получении числового параметра
#define WND -1

// Wrong Double Number Data - ошибка при получении дробного параметра
#define WDND -1.0

// =====================
// todo: 
// - Алгоритм оптимального распределения машин
// - Обработка ошибок
// - Переопределить взаимодействие с json-файлами
// =====================


// Функция для обнаружения двух максимальных чисел - одно до разделителя "/", второе - после
// ==================================================
// Пример:
//      Строка - "abc22def15ghi123jkl/abc321def123ghi"
//      Результат - pair.first = 123, pair.second = 321
// ==================================================
// Нужна, только если формат прайса как в эксельке: "105–115 чел. / 30–33 места"
pair<int, int> parseMaxNumbers(const string& input) {
    
    size_t slashPos = input.find('/');
    if (slashPos == string::npos) {
        // Если разделителя нет - что-то не так
        return { -1, -1 };
    }

    string leftPart = input.substr(0, slashPos);
    string rightPart = input.substr(slashPos + 1);

    // Лямбда для извлечения максимального целого числа из подстроки
    auto getMax = [](const string& text) -> int {
        std::regex numberPattern(R"(\d+)");  // одно или более цифр
        auto begin = sregex_iterator(text.begin(), text.end(), numberPattern);
        auto end = sregex_iterator();

        int maxVal = 0;
        for (auto it = begin; it != end; ++it) {
            int val = stoi(it->str());
            if (val > maxVal) maxVal = val;
        }
        return maxVal;
    };

    int leftMax = getMax(leftPart);
    int rightMax = getMax(rightPart);

    return { leftMax, rightMax };
}

// Класс Машина
// Содержит информацию по каждой модели машины
class Car {

public:

    string modelName; // имя модели
    string carSize; // класс машины (например: "малый", "особо большой")

    int amountOfCars; // количество подобных моделей в автопарке
    int innerCapacity; // вместимость салона общая

    // стоимость поездки
    int driveCost; 

    // Вместо него всегда будет вызываться конструктор со значениями по умолчанию
//    Car() = delete;

    // Конструктор со всеми значениями(он и должен всегда вызываться)
    Car(string modelName = WSD, 
        string carSize = WSD, 
        int amountOfCars = WND, 
        int innerCapacity = WND, 
        int driveCost = WND) 
    {

        this->modelName = modelName;
        this->carSize = carSize;
        this->amountOfCars = amountOfCars;
        this->innerCapacity = innerCapacity;
        this->driveCost = driveCost;

    }

};

// Класс Модификаторы
// Хранит общие значения для всех перевозок, которые повлияют на итоговую стоимость
class Modifiers {

public:

    string region; // Регион один для всех перевозок
    string season; // Время года
    double route_cycle_hours; // Длительность каждой из перевозок

    Modifiers() = delete;

    Modifiers(string region = WSD, 
        string season = WSD, 
        double cycle = WND) 
    {

        this->region = region;
        this->season = season;
        this->route_cycle_hours = cycle;

    }

};

// Класс Расписание
// Хранит параметры, привязанные к определённому дню недели
class Schedule {

public:

    string dayOfWeek; // День недели
    double dailyCoefficient; // Коэф за плотность пассжиропотока
    vector<pair<string, int>> hourly_flow; // Кол-во людей по часам

    Schedule() = delete;

    Schedule(string dayOfWeek = WSD,
        double dailyCoefficient = WDND,
        const vector<pair<string, int>>& hourly_flow = {}) 
    {
        
        this->dayOfWeek = dayOfWeek;
        this->dailyCoefficient = dailyCoefficient;
        this->hourly_flow = hourly_flow;

    }

};



// Три основные переменные с данными:
// - Массив ТС vechiclePark
// - Модификаторы стоимости modifiers
// - Расписание пассажиропотока schedule
int main() {
    
    setlocale(LC_ALL, "ru");

    // Для работы с json-файлом
    Json::Value root;
    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;

    // Пока берём из текущей папки
    ifstream file("data.json");

    // Проверка на корректное открытие json-файла
    bool successOpen = Json::parseFromStream(builder, file, &root, &errs);

    if (!successOpen) {
        cerr << "Ошибка открытия json-файла: " << errs << endl;
        return 1; // todo: добавить обработку ошибок
    }

    const Json::Value& fleet = root["fleet"];

    // Основная переменная со всеми машинами
    vector<Car> vechiclePark(fleet.size());

    size_t counter = 0; // Счётчик для занесения записей о машинах в вектор

    for (const Json::Value& bus : fleet) {

        string name = bus["model"].asString();
        string size = bus["class"].asString();

        int amount = bus["max_available"].asInt();
        int capacity = bus["capacity"].asInt();
        int cost = bus["base_cost_per_hour"].asInt();
        
        Car car = Car(name, size, amount, capacity, cost);

        vechiclePark[counter++] = car;

        // todo: обработка ошибок

    }

    const Json::Value& mods = root["modifiers"];

    const string region = mods["region"].asString();
    const string season = mods["season"].asString();
    const double cycle = mods["route_cycle_hours"].asDouble();

    // Основная переменная с модификаторами на стоимость поездки
    Modifiers modifiers = Modifiers(region, season, cycle);

    const Json::Value& sched = root["schedule"];

    const string dayOfWeek = sched["day_of_week"].asString();
    const double coef = sched["daily_coefficient"].asDouble();

    const Json::Value& flow = root["schedule"]["hourly_flow"];

    vector<pair<string, int>> hourly_flow(flow.size());

    counter = 0; // Переиспользуем переменную

    for (const string& key : flow.getMemberNames()) {

        hourly_flow[counter].first = key;
        hourly_flow[counter++].second = flow[key].asInt();

        // todo: обработка ошибок

    }

    // Основная переменная с параметрами расписания
    Schedule schedule = Schedule(dayOfWeek, coef, hourly_flow);

#ifdef DEBUG

    cout << "Машины:" << endl;

    for (int i = 0; i < vechiclePark.size(); ++i) {

        cout << "Название модели: " << vechiclePark[i].modelName << "; Класс машины: " << vechiclePark[i].carSize << "; Количество машин: " << vechiclePark[i].amountOfCars \
            << "; Вместимость салона: " << vechiclePark[i].innerCapacity << "; Цена за час езды: " << vechiclePark[i].driveCost << endl << endl;

    }

    cout << endl << "Модификаторы:" << endl;

    cout << "Регион: " << modifiers.region << "\nСезон: " << modifiers.season << "\nДлительность рейса: " << modifiers.route_cycle_hours << endl << endl;

    cout << "Расписание пассажиропотока:" << endl;

    cout << "День недели: " << schedule.dayOfWeek << "\nКоэффициент: " << schedule.dailyCoefficient << endl << endl;

    for (int i = 0; i < schedule.hourly_flow.size(); ++i) {

        cout << "Часы: " << schedule.hourly_flow[i].first << " Количество людей: " << schedule.hourly_flow[i].second << endl << endl;

    }

    cout << endl;

#endif

}