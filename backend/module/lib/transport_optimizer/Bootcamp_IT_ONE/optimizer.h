#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace transport {
    inline constexpr int kHoursPerDay = 24;
    struct FleetType {
        std::string model;
        std::string vehicle_class;
        int64_t max_available = 0;
        int64_t capacity = 0;
        int64_t base_cost_kopecks_per_hour = 0;
    };

    struct InputData {
        std::vector<FleetType> fleet;
        std::array<int64_t, kHoursPerDay> demand{};
        std::string day_of_week;
        std::string region;
        std::string season;
        int64_t route_minutes = 0;
        double daily_coefficient = 1.0;
        double region_multiplier = 1.0;
        double season_multiplier = 1.0;
        double time_limit_seconds = 5.0;
        int workers = 8;
    };

    nlohmann::json Optimize(const InputData& data);
}