// includes math binding for module
package module

import "C"

// JSON input
// ---
type FleetItem struct {
	Model           string  `json:"model"`
	Class           string  `json:"class"`
	MaxAvailable    int     `json:"max_available"`
	Capacity        int     `json:"capacity"`
	BaseCostPerHour float64 `json:"base_cost_per_hour"`
}

type Modifiers struct {
	Region          string  `json:"region"`
	Season          string  `json:"season"`
	RouteCycleHours float64 `json:"route_cycle_hours"`
}

type Schedule struct {
	DayOfWeek        string         `json:"day_of_week"`
	DailyCoefficient float64        `json:"daily_coefficient"`
	HourlyFlow       map[string]int `json:"hourly_flow"`
}

// верхушка айсберга
type Root struct {
	Fleet     []FleetItem `json:"fleet"`
	Modifiers Modifiers   `json:"modifiers"`
	Schedule  Schedule    `json:"schedule"`
}

// ---

// JSON output
// ---
// Departure представляет один рейс
type Departure struct {
	Time  string `json:"time"`
	Trips int    `json:"trips"`
}

// AssignedVehicle представляет назначенный транспорт в расписании
type AssignedVehicle struct {
	Capacity        int         `json:"capacity"`
	Departures      []Departure `json:"departures"`
	Model           string      `json:"model"`
	OfferedCapacity int         `json:"offered_capacity"`
	Trips           int         `json:"trips"`
}

// ScheduleInterval представляет интервал расписания
type ScheduleInterval struct {
	Assigned        []AssignedVehicle `json:"assigned"`
	Demand          int               `json:"demand"`
	Interval        string            `json:"interval"`
	OfferedCapacity int               `json:"offered_capacity"`
	Shortage        int               `json:"shortage"`
}

// FleetUsage представляет использование автопарка
type FleetUsage struct {
	BusyMinutes  int    `json:"busy_minutes"`
	MaxAvailable int    `json:"max_available"`
	Model        string `json:"model"`
	Trips        int    `json:"trips"`
}

// Parameters представляет параметры расчета
type Parameters struct {
	DailyCoefficient  float64 `json:"daily_coefficient"`
	RegionMultiplier  float64 `json:"region_multiplier"`
	RouteCycleMinutes int     `json:"route_cycle_minutes"`
	SeasonMultiplier  float64 `json:"season_multiplier"`
}

// Summary представляет сводку результата
type Summary struct {
	AllDemandSatisfied    bool    `json:"all_demand_satisfied"`
	OperatingCostRub      float64 `json:"operating_cost_rub"`
	ShortagePassengers    int     `json:"shortage_passengers"`
	TotalDemand           int     `json:"total_demand"`
	TransportedPassengers int     `json:"transported_passengers"`
}

// OutputResponse корневая структура выходных данных
type OutputResponse struct {
	CostStatus       string             `json:"cost_status"`
	FleetUsage       []FleetUsage       `json:"fleet_usage"`
	OptimalityProven bool               `json:"optimality_proven"`
	Parameters       Parameters         `json:"parameters"`
	PrimaryStatus    string             `json:"primary_status"`
	Schedule         []ScheduleInterval `json:"schedule"`
	Status           string             `json:"status"`
	Summary          Summary            `json:"summary"`
}

// ---
func (*Root) CalculateSolution() (OutputResponse, error)
