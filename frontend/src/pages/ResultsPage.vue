<script setup>
import { computed } from 'vue'
import * as XLSX from 'xlsx'
import AppHeader from '../components/AppHeader.vue'
import ResultSummary from '../components/ResultSummary.vue'
import HourlyScheduleTable from '../tables/HourlyScheduleTable.vue'
import FleetUsageTable from '../tables/FleetUsageTable.vue'
import RouteInfo from '../components/RouteInfo.vue'

const props = defineProps({
    outputData: { type: Object, default: null },
    inputData: { type: Object, default: null },
    isLoading: { type: Boolean, default: false }
})

const emit = defineEmits(['loadOutput', 'back'])

const result = computed(() => {
    if (!props.outputData)
        return null
    const data = props.outputData

    const fleetUsageCombined = data.fleet_usage.map((usage) => {
        const fleetItem = props.inputData?.fleet?.find((f) => f.model === usage.model)
        const baseCost = fleetItem?.base_cost_per_hour || 0
        return {
            model: usage.model,
            classType: fleetItem?.class || '—',
            available: usage.max_available,
            trips: usage.trips,
            busyMinutes: usage.busy_minutes,
            baseCostPerHour: baseCost,
            totalCost: (usage.busy_minutes / 60) * baseCost
        }
    })

    return {
        dayOfWeek: data.schedule?.[0]?.interval ? data.schedule[0].interval : '',
        knn: data.parameters?.daily_coefficient || null,
        date: new Date().toLocaleDateString('ru-RU'),
        totalCost: data.summary?.operating_cost_rub || 0,
        totalBuses: data.fleet_usage.reduce((sum, i) => sum + i.trips, 0),
        servedPassengers: data.summary?.transported_passengers || 0,
        deficit: data.summary?.shortage_passengers || 0,
        hourlyData: data.schedule || [],
        fleetUsage: fleetUsageCombined,
        routeInfo: {
            stops: 10,
            stopDuration: 3,
            cycleDuration: data.parameters?.route_cycle_minutes || 0
        }
    }
})

const exportReport = () => {
    if (!result.value) 
        return
    const wb = XLSX.utils.book_new()

    const summaryData = [
        ['Показатель', 'Значение'],
        ['Стоимость (₽)', result.value.totalCost],
        ['Пассажиров перевезено', result.value.servedPassengers],
        ['Дефицит', result.value.deficit]
    ]
    const ws1 = XLSX.utils.aoa_to_sheet(summaryData)
    XLSX.utils.book_append_sheet(wb, ws1, 'Сводка')

    const scheduleData = result.value.hourlyData.map((item) => ({
        Интервал: item.interval,
        Спрос: item.demand,
        Вместимость: item.offered_capacity,
        Обслужено: item.demand - item.shortage,
        Дефицит: item.shortage
    }))
    const ws2 = XLSX.utils.json_to_sheet(scheduleData)
    XLSX.utils.book_append_sheet(wb, ws2, 'Расписание')

    const fleetData = result.value.fleetUsage.map((item) => ({
        Модель: item.model,
        Класс: item.classType,
        Доступно: item.available,
        Рейсов: item.trips,
        'Время работы (мин)': item.busyMinutes,
        'Стоимость (₽)': item.totalCost
    }))
    const ws3 = XLSX.utils.json_to_sheet(fleetData)
    XLSX.utils.book_append_sheet(wb, ws3, 'Автопарк')

    XLSX.writeFile(wb, 'transport_report.xlsx')
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
        <div class="text-center mb-4">
            <button
                class="btn btn-secondary"
                @click="emit('loadOutput')"
                :disabled="isLoading"
            >
                {{ isLoading ? 'Загрузка...' : 'Загрузить output.json' }}
            </button>
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