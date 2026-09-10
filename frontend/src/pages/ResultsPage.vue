<script setup>
import { computed, onMounted } from 'vue'
import { downloadExcel } from '../api'
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
    exportOnly: { // Только экспорт?
        type: Boolean, 
        default: false 
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
            seatsTotal: fleetItem?.seats_total ?? null,
            seatsSeated: fleetItem?.seats_seated ?? null,
            totalCost: Math.round((usage.busy_minutes / 60) * baseCost)
        }
    })

    // Длительность цикла
    const cycleDuration = data.parameters?.route_cycle_minutes ?? input?.modifiers?.route_cycle_minutes ?? 0

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
            travelTime: input?.modifiers?.travel_time_min ?? data.parameters?.travel_time_min ?? 0,
            cycleDuration
        }
    }
})

// Экспорт в Excel
const exportExcel = async () => {
    const iso = props.reportDate || props.inputData?.schedule?.date || "no-date"
    try 
    {
        await downloadExcel(props.inputData, `transport_report_${iso}.xlsx`)
    } 
    catch (e) 
    {
        alert(`Ошибка выгрузки: ${e.message}`)
    }
}
</script>

<template>
    <div class="container results-page">
        <AppHeader
            title="Результаты расчёта маршрута"
            :dayOfWeek="result?.dayOfWeek || ''"
            :knn="result?.knn"
            :date="result?.date || ''"
        />
        <div v-if="error" class="alert alert-danger text-center">
            {{ error }}
        </div>
        <template v-if="result  && !exportOnly">
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
                :travelTime="result.routeInfo.travelTime"
                :cycleDuration="result.routeInfo.cycleDuration"
            />
            <div class="text-center mt-4 mb-5">
                <button class="btn btn-lg btn-success me-2" @click="exportExcel">
                    Экспорт отчёта в Excel
                </button>
                <button class="btn btn-lg btn-secondary" @click="emit('back')">
                    Назад к настройкам
                </button>
            </div>
        </template>
        <div v-else-if="exportOnly" class="alert alert-success text-center mt-4">
            Отчёт выгружен в Excel.
            <button class="btn btn-secondary ms-2" @click="emit('back')">Назад к настройкам</button>
        </div>
    </div>
</template>