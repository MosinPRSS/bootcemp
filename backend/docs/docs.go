package docs

import (
	"fmt"
	"os"
	"path/filepath"

	"github.com/xuri/excelize/v2"

	"bootcemp/server/module"
)

// GenerateXLSX создаёт простой XLSX-отчёт и возвращает путь к файлу.
func GenerateXLSX(data module.OutputResponse) (string, error) {
	f := excelize.NewFile()
	defer func() {
		_ = f.Close()
	}()

	// ============ Лист 1: Сводка ============
	summarySheet := "Сводка"
	idx, err := f.NewSheet(summarySheet)
	if err != nil {
		return "", fmt.Errorf("не удалось создать лист %q: %w", summarySheet, err)
	}
	f.SetActiveSheet(idx)
	_ = f.DeleteSheet("Sheet1")

	_ = f.SetColWidth(summarySheet, "A", "A", 35)
	_ = f.SetColWidth(summarySheet, "B", "B", 30)

	summaryRows := [][]interface{}{
		{"Показатель", "Значение"},
		{"Статус", data.Status},
		{"Статус стоимости", data.CostStatus},
		{"Первичный статус", data.PrimaryStatus},
		{"Оптимальность доказана", boolRu(data.OptimalityProven)},
		{"Все потребности удовлетворены", boolRu(data.Summary.AllDemandSatisfied)},
		{},
		{"Общий спрос (пасс.)", data.Summary.TotalDemand},
		{"Перевезено (пасс.)", data.Summary.TransportedPassengers},
		{"Дефицит (пасс.)", data.Summary.ShortagePassengers},
		{"Операционные расходы (руб.)", data.Summary.OperatingCostRub},
		{},
		{"Суточный коэффициент", data.Parameters.DailyCoefficient},
		{"Региональный множитель", data.Parameters.RegionMultiplier},
		{"Время цикла маршрута (мин)", data.Parameters.RouteCycleMinutes},
		{"Сезонный множитель", data.Parameters.SeasonMultiplier},
	}

	for i, row := range summaryRows {
		r := i + 1
		if len(row) == 0 {
			continue
		}
		_ = f.SetCellValue(summarySheet, fmt.Sprintf("A%d", r), row[0])
		if len(row) > 1 {
			_ = f.SetCellValue(summarySheet, fmt.Sprintf("B%d", r), row[1])
		}
	}

	// ============ Лист 2: Расписание ============
	scheduleSheet := "Расписание"
	if _, err := f.NewSheet(scheduleSheet); err != nil {
		return "", fmt.Errorf("не удалось создать лист %q: %w", scheduleSheet, err)
	}

	headers := []string{
		"Интервал", "Модель", "Вместимость", "Время отправления",
		"Рейсов", "Предложенная вместимость", "Спрос",
		"Предложено всего", "Дефицит",
	}
	_ = f.SetColWidth(scheduleSheet, "A", "A", 14)
	_ = f.SetColWidth(scheduleSheet, "B", "B", 22)
	_ = f.SetColWidth(scheduleSheet, "C", "C", 14)
	_ = f.SetColWidth(scheduleSheet, "D", "D", 18)
	_ = f.SetColWidth(scheduleSheet, "E", "E", 10)
	_ = f.SetColWidth(scheduleSheet, "F", "F", 24)
	_ = f.SetColWidth(scheduleSheet, "G", "G", 12)
	_ = f.SetColWidth(scheduleSheet, "H", "H", 18)
	_ = f.SetColWidth(scheduleSheet, "I", "I", 12)

	for i, h := range headers {
		cell, _ := excelize.CoordinatesToCellName(i+1, 1)
		_ = f.SetCellValue(scheduleSheet, cell, h)
	}

	row := 2
	for _, interval := range data.Schedule {
		for _, av := range interval.Assigned {
			for _, dep := range av.Departures {
				_ = f.SetCellValue(scheduleSheet, fmt.Sprintf("A%d", row), interval.Interval)
				_ = f.SetCellValue(scheduleSheet, fmt.Sprintf("B%d", row), av.Model)
				_ = f.SetCellValue(scheduleSheet, fmt.Sprintf("C%d", row), av.Capacity)
				_ = f.SetCellValue(scheduleSheet, fmt.Sprintf("D%d", row), dep.Time)
				_ = f.SetCellValue(scheduleSheet, fmt.Sprintf("E%d", row), dep.Trips)
				_ = f.SetCellValue(scheduleSheet, fmt.Sprintf("F%d", row), av.OfferedCapacity)
				_ = f.SetCellValue(scheduleSheet, fmt.Sprintf("G%d", row), interval.Demand)
				_ = f.SetCellValue(scheduleSheet, fmt.Sprintf("H%d", row), interval.OfferedCapacity)
				_ = f.SetCellValue(scheduleSheet, fmt.Sprintf("I%d", row), interval.Shortage)
				row++
			}
		}
	}

	// ============ Лист 3: Автопарк ============
	fleetSheet := "Автопарк"
	if _, err := f.NewSheet(fleetSheet); err != nil {
		return "", fmt.Errorf("не удалось создать лист %q: %w", fleetSheet, err)
	}

	fleetHeaders := []string{
		"Модель", "Занято (мин)", "Доступно макс.", "Рейсов",
		"Занято (ч)", "Загрузка, %",
	}
	_ = f.SetColWidth(fleetSheet, "A", "A", 22)
	_ = f.SetColWidth(fleetSheet, "B", "B", 16)
	_ = f.SetColWidth(fleetSheet, "C", "C", 18)
	_ = f.SetColWidth(fleetSheet, "D", "D", 12)
	_ = f.SetColWidth(fleetSheet, "E", "E", 14)
	_ = f.SetColWidth(fleetSheet, "F", "F", 14)

	for i, h := range fleetHeaders {
		cell, _ := excelize.CoordinatesToCellName(i+1, 1)
		_ = f.SetCellValue(fleetSheet, cell, h)
	}

	totalBusy := 0
	totalMax := 0
	totalTrips := 0
	for i, fu := range data.FleetUsage {
		r := i + 2
		busyHours := float64(fu.BusyMinutes) / 60.0
		loadPct := 0.0
		if fu.MaxAvailable > 0 {
			loadPct = float64(fu.Trips) / float64(fu.MaxAvailable) * 100.0
		}

		_ = f.SetCellValue(fleetSheet, fmt.Sprintf("A%d", r), fu.Model)
		_ = f.SetCellValue(fleetSheet, fmt.Sprintf("B%d", r), fu.BusyMinutes)
		_ = f.SetCellValue(fleetSheet, fmt.Sprintf("C%d", r), fu.MaxAvailable)
		_ = f.SetCellValue(fleetSheet, fmt.Sprintf("D%d", r), fu.Trips)
		_ = f.SetCellValue(fleetSheet, fmt.Sprintf("E%d", r), busyHours)
		_ = f.SetCellValue(fleetSheet, fmt.Sprintf("F%d", r), loadPct)

		totalBusy += fu.BusyMinutes
		totalMax += fu.MaxAvailable
		totalTrips += fu.Trips
	}

	totalRow := len(data.FleetUsage) + 2
	_ = f.SetCellValue(fleetSheet, fmt.Sprintf("A%d", totalRow), "Итого")
	_ = f.SetCellValue(fleetSheet, fmt.Sprintf("B%d", totalRow), totalBusy)
	_ = f.SetCellValue(fleetSheet, fmt.Sprintf("C%d", totalRow), totalMax)
	_ = f.SetCellValue(fleetSheet, fmt.Sprintf("D%d", totalRow), totalTrips)
	_ = f.SetCellValue(fleetSheet, fmt.Sprintf("E%d", totalRow), float64(totalBusy)/60.0)

	// ============ Сохранение ============
	if err := os.MkdirAll("reports", 0o755); err != nil {
		return "", fmt.Errorf("не удалось создать директорию reports: %w", err)
	}
	filePath := filepath.Join("reports", "report.xlsx")
	if err := f.SaveAs(filePath); err != nil {
		return "", fmt.Errorf("не удалось сохранить файл %q: %w", filePath, err)
	}

	return filePath, nil
}

func boolRu(b bool) string {
	if b {
		return "Да"
	}
	return "Нет"
}
