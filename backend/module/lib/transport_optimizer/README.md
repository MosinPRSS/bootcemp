# Вычислительный модуль

Подключите optimizer.h и свяжите приложение с CMake-целью transport_optimizer.
Возвращаемый nlohmann::json сохраняет результат.

## Пример вызова

~~~cpp
#include "optimizer.h"

void Calculate() {
    transport::InputData data;
    data.fleet.push_back({"ЛиАЗ-6213", "Особо большой", 15, 175, 360000});
    data.route_minutes = 120;
    data.daily_coefficient = 1.10;
    data.demand[6] = 1980; // 1800 * 1.10, уже округлено вызывающей стороной.
    data.region = "Крайний Север";
    data.season = "Зима";
    data.region_multiplier = 1.40;
    data.season_multiplier = 1.125;

    const nlohmann::json result = transport::Optimize(data);
    // Передайте result следующему модулю или используйте result.dump(2).
}
~~~

## Контракт данных

- fleet: модели, количество доступных автобусов, вместимость и цена в копейках за час.
- route_minutes: положительная целая длительность рейса в минутах.
- demand: 24 неотрицательных значения; индекс 0 соответствует 00-01, индекс 23 — 23-24.
  Пропущенные часы оставляют нулевыми. Дневной коэффициент применяется до вызова,
  с округлением до целого, как в прежнем чтении JSON.
- daily_coefficient: отражается в результате; повторно на demand не умножается.
- region_multiplier и season_multiplier: явно заданные коэффициенты стоимости.
  Значения по умолчанию равны 1.0. Строки region, season и day_of_week сохранены
  для передачи контекста, сами по себе расчёт не меняют.
- time_limit_seconds: лимит каждого запуска решателя, а не всего вызова Optimize.
- workers: положительное число рабочих потоков.

Optimize не изменяет InputData. Основные некорректные параметры вызывают
std::invalid_argument. Вызывающая сторона отвечает за проверку размеров данных
и диапазонов, исключающих переполнение целочисленной арифметики модели.
Статусы UNKNOWN, INFEASIBLE и MODEL_INVALID возвращаются в JSON.
Признак optimality_proven показывает, доказана ли оптимальность обоих этапов.

## Сборка и интеграция

~~~cmake
add_subdirectory(path/to/module transport-module)
target_link_libraries(your_application PRIVATE transport_optimizer)
~~~

CMake скачивает OR-Tools v9.15 и требует установленный пакет nlohmann_json.
Рабочая проверенная конфигурация этого проекта - MSVC x64, C++20, Release.
Пути к установленным зависимостям передаются через настройки CMake окружения.

~~~text
cmake -S . -B build
cmake --build build --config Release --target transport_optimizer
~~~
