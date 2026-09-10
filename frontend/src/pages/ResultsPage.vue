<script setup>
import { computed } from 'vue'
import * as XLSX from 'xlsx'
import AppHeader from '../components/AppHeader.vue'
import ResultSummary from '../components/ResultSummary.vue'
import HourlyScheduleTable from '../tables/HourlyScheduleTable.vue'
import FleetUsageTable from '../tables/FleetUsageTable.vue'
import RouteInfo from '../components/RouteInfo.vue'

const props = defineProps({
    outputData: { // Данные из output.json
        type: Object, 
        default: null 
    },
    inputData: { // Данные из input.json (нужны для cost/class/даты)
        type: Object, 
        default: null 
    }, 
    isLoading: { // Флаг загрузки output.json
        type: Boolean, 
        default: false 
    }, 
    reportDate: { // Дата, выбранная пользователем
        type: String, 
        default: '' 
    },
    error: {  // Текст ошибки загрузки (если есть)
        type: String, 
        default: '' 
    }
})

// loadOutput — загрузка output.json; back — возврат на страницу настроек
const emit = defineEmits(['back'])

const formattedDate = computed(() => {
    const iso = props.reportDate || props.inputData?.schedule?.date
    if (!iso)
        return new Date().toLocaleDateString('ru-RU', { day: 'numeric', month: 'long', year: 'numeric' })
    return new Date(iso).toLocaleDateString('ru-RU', { day: 'numeric', month: 'long', year: 'numeric' })
})

// Нормализация данных для UI
const result = computed(() => {
    if (!props.outputData)
        return null
    const data = props.outputData
    const input = props.inputData

    // Соединяем fleet_usage из output.json с input.json:
    //   - класс ТС (Малый / Большой / …) есть только в input
    //   - base_cost_per_hour тоже только в input
    // Считаем расчётную стоимость = (busy_minutes / 60) * base_cost_per_hour
    // ВАЖНО: это НЕ то же самое, что summary.operating_cost_rub из output —
    // бэкенд может считать по своей формуле (с коэффициентами региона/сезона и т.п.)
    const fleetUsageCombined = data.fleet_usage.map((usage) => {
        const fleetItem = input?.fleet?.find((f) => f.model === usage.model)
        const baseCost = fleetItem?.base_cost_per_hour || 0
        return {
            model: usage.model,
            classType: fleetItem?.class || '—',
            available: usage.max_available,
            trips: usage.trips,
            busyMinutes: usage.busy_minutes,
            baseCostPerHour: baseCost,
            totalCost: Math.round((usage.busy_minutes / 60) * baseCost)
        }
    })

    // Длительность цикла: приоритет у output (в минутах), иначе - пересчёт из input (в часах)
    const cycleDuration = data.parameters?.route_cycle_minutes ?? (input?.modifiers?.route_cycle_hours ? input.modifiers.route_cycle_hours * 60 : 0)

    return {
        // День недели берём ИЗ INPUT, а не из interval (иначе было "06-07")
        dayOfWeek: input?.schedule?.day_of_week || data.parameters?.day_of_week || '',
        knn: data.parameters?.daily_coefficient ?? null,
        date: formattedDate.value,

        // Итоговая стоимость - берём готовое значение из output
        totalCost: data.summary?.operating_cost_rub || 0,
        totalBuses: data.fleet_usage.reduce((sum, i) => sum + i.trips, 0),
        servedPassengers: data.summary?.transported_passengers || 0,
        deficit: data.summary?.shortage_passengers || 0,

        hourlyData: data.schedule || [],
        fleetUsage: fleetUsageCombined,

        routeInfo: {
            // Если в input.modifiers есть stops/stop_duration_min - берём оттуда
            stops: input?.modifiers?.stops ?? data.parameters?.stops ?? 0,
            stopDuration: input?.modifiers?.stop_duration_min ?? data.parameters?.stop_duration_min ?? 0,
            cycleDuration
        }
    }
})

// Экспорт в Excel (4 листа)
const exportReport = () => {
    if (!result.value)
        return
    const wb = XLSX.utils.book_new()

    // Сводка
    const summaryData = [
        ['Дата расчёта', result.value.date],
        ['День недели', result.value.dayOfWeek],
        ['Коэффициент дня', result.value.knn],
        [],
        ['Показатель', 'Значение'],
        ['Стоимость (₽)', result.value.totalCost],
        ['Пассажиров перевезено', result.value.servedPassengers],
        ['Дефицит', result.value.deficit]
    ]
    const ws1 = XLSX.utils.aoa_to_sheet(summaryData)
    XLSX.utils.book_append_sheet(wb, ws1, 'Сводка')

    // Расписание по часам
    const scheduleData = result.value.hourlyData.map((item) => ({
        Интервал: item.interval,
        Спрос: item.demand,
        Вместимость: item.offered_capacity,
        Обслужено: item.demand - item.shortage,
        Дефицит: item.shortage
    }))
    const ws2 = XLSX.utils.json_to_sheet(scheduleData)
    XLSX.utils.book_append_sheet(wb, ws2, 'Расписание')

    // Автопарк
    // колонка "Стоимость" - это расчётная стоимость из UI,
    // она может отличаться от summary.operating_cost_rub
    const fleetData = result.value.fleetUsage.map((item) => ({
        Модель: item.model,
        Класс: item.classType,
        Доступно: item.available,
        Рейсов: item.trips,
        'Время работы (мин)': item.busyMinutes,
        'Расчётная стоимость (₽)': item.totalCost
    }))
    const ws3 = XLSX.utils.json_to_sheet(fleetData)
    XLSX.utils.book_append_sheet(wb, ws3, 'Автопарк')

    // Параметры расчёта
    const paramsData = [
        ['Параметр', 'Значение'],
        ['Регион', props.inputData?.modifiers?.region ?? '—'],
        ['Сезон', props.inputData?.modifiers?.season ?? '—'],
        ['Коэф. дня', props.outputData?.parameters?.daily_coefficient ?? '—'],
        ['Коэф. региона', props.outputData?.parameters?.region_multiplier ?? '—'],
        ['Коэф. сезона', props.outputData?.parameters?.season_multiplier ?? '—'],
        ['Цикл маршрута (мин)', result.value.routeInfo.cycleDuration]
    ]
    const ws4 = XLSX.utils.aoa_to_sheet(paramsData)
    XLSX.utils.book_append_sheet(wb, ws4, 'Параметры')

    // Имя файла с датой - чтобы отчёты не перезаписывали друг друга
    const isoDate = props.reportDate || props.inputData?.schedule?.date || 'no-date'
    XLSX.writeFile(wb, `transport_report_${isoDate}.xlsx`)
}
</script>

<template>
    <div class="container results-page">
        <AppHeader
            title="Результаты отчёта"
            :dayOfWeek="result?.dayOfWeek || ''"
            :knn="result?.knn"
            :date="result?.date || ''"
        />
        <div v-if="error" class="alert alert-danger text-center">
            {{ error }}
        </div>
        <template v-if="result">
            <ResultSummary
                :totalCost="result.totalCost"
                :totalBuses="result.totalBuses"
                :servedPassengers="result.servedPassengers"
                :deficit="result.deficit"
            />
            <HourlyScheduleTable :data="result.hourlyData" />
            <FleetUsageTable :fleet-usage="result.fleetUsage" />
            <RouteInfo
                :stops="result.routeInfo.stops"
                :stopDuration="result.routeInfo.stopDuration"
                :cycleDuration="result.routeInfo.cycleDuration"
            />
            <div class="text-center mt-4 mb-5">
                <button class="btn btn-lg btn-success me-2" @click="exportReport">
                    Экспорт отчёта в Excel
                </button>
                <button class="btn btn-lg btn-secondary" @click="emit('back')">
                    Назад к настройкам
                </button>
            </div>
        </template>
        <div v-else class="alert alert-info text-center">
            Нажмите кнопку выше, чтобы загрузить результаты.
        </div>
    </div>
</template>