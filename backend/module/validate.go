package module

import (
	"encoding/json"
	"fmt"
	"strconv"
	"strings"
)

func (r *Root) ValidateJSON(d []byte) (*Root, error) {
	// если дается файлом с фронта, то нужно будет черз _os_
	// его превратить в строку

	if !json.Valid(d) {
		return nil, fmt.Errorf("Cannot validate the data!")
	}

	err := json.Unmarshal(d, r)

	if err != nil {
		return nil, fmt.Errorf("Cannot unmarshal the data")
	}
	// куча проверок..................
	if len(r.Fleet) == 0 {
		return nil, fmt.Errorf("fleet не может быть пустым")
	}

	for i, item := range r.Fleet {
		if item.Model == "" {
			return nil, fmt.Errorf("fleet[%d]: model обязательное поле", i)
		}
		if item.Class == "" {
			return nil, fmt.Errorf("fleet[%d]: class обязательное поле", i)
		}
		if item.MaxAvailable <= 0 {
			return nil, fmt.Errorf("fleet[%d]: max_available должен быть > 0, получено %d", i, item.MaxAvailable)
		}
		if item.Capacity <= 0 {
			return nil, fmt.Errorf("fleet[%d]: capacity должен быть > 0, получено %d", i, item.Capacity)
		}
		if item.BaseCostPerHour <= 0 {
			return nil, fmt.Errorf("fleet[%d]: base_cost_per_hour должен быть > 0, получено %.2f", i, item.BaseCostPerHour)
		}
	}

	// Проверка modifiers
	if r.Modifiers.Region == "" {
		r.Modifiers.Region = "1"
	}
	if r.Modifiers.Season == "" {
		r.Modifiers.Season = "1"
	}
	if r.Modifiers.RouteCycleHours <= 0 {
		fmt.Errorf("modifiers.route_cycle_hours должен быть > 0, получено %.1f", r.Modifiers.RouteCycleHours)
		r.Modifiers.RouteCycleHours = 1
	}

	// Проверка schedule
	if r.Schedule.DayOfWeek == "" {
		return nil, fmt.Errorf("schedule.day_of_week обязательное поле")
	}
	if r.Schedule.DailyCoefficient <= 0 {
		return nil, fmt.Errorf("schedule.daily_coefficient должен быть > 0, получено %.2f", r.Schedule.DailyCoefficient)
	}
	if len(r.Schedule.HourlyFlow) == 0 {
		return nil, fmt.Errorf("schedule.hourly_flow не может быть пустым")
	}

	// Проверка временных интервалов
	for timeSlot, flow := range r.Schedule.HourlyFlow {
		if !isValidTimeSlot(timeSlot) {
			return nil, fmt.Errorf("schedule.hourly_flow: невалидный временной интервал '%s'", timeSlot)
		}
		if flow < 0 {
			return nil, fmt.Errorf("schedule.hourly_flow[%s]: значение не может быть отрицательным, получено %d", timeSlot, flow)
		}
	}

	fmt.Println("Проверено")
	return r, nil
}

func isValidTimeSlot(slot string) bool {
	// Проверка формата "HH-HH"
	parts := strings.Split(slot, "-")
	if len(parts) != 2 {
		return false
	}
	for _, p := range parts {
		if len(p) != 2 {
			return false
		}
		hour, err := strconv.Atoi(p)
		if err != nil || hour < 0 || hour > 23 {
			return false
		}
	}
	return true
}
