#pragma once

#include <fcntl.h>
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

// Если при попытке взять строкое значение по ключю в json-файле не удаётся, будет это значение по умолчанию
#define DEF "default" 

// Если при попытке взять числовое значение по ключю в json-файле не удаётся, будет это значение по умолчанию
#define DEFN -1

// Если при попытке взять дробное значение по ключю в json-файле не удаётся, будет это значение по умолчанию
#define DEFD -1.0

// Wrond String Data - ошибка при получении строкового параметра
#define WSD "unknown"

// Wrong Number Data - ошибка при получении числового параметра
#define WND -1

// Wrong Double Number Data - ошибка при получении дробного параметра
#define WDND -1.0


// Функция для обнаружения двух максимальных чисел - одно до разделителя "/", второе - после
// ==================================================
// Пример:
//      Строка - "abc22def15ghi123jkl/abc321def123ghi"
//      Результат - pair.first = 123, pair.second = 321
// ==================================================
// Нужна, только если формат прайса как в эксельке: "105–115 чел. / 30–33 места"
// 
// Осталась от старой версии json-файла, пока не убираю
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