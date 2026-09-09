<script setup>
import { ref } from "vue"
import AppHeader from "../components/AppHeader.vue"
import ResultSummary from "../components/ResultSummary.vue"
import HourlyScheduleTable from "../tables/HourlyScheduleTable.vue"
import FleetUsageTable from "../tables/FleetUsageTable.vue"
import RouteInfo from "../components/RouteInfo.vue"

const result = ref({
    dayOfWeek: "Пятница",
    knn: 1.10,
    date: "2026-09-08",
    totalCost: 123456,
    totalBuses: 45,
    servedPassengers: 38000,
    deficit: 4000,

    hourlyData: [
        {
            hour: "07-08",
            baseFlow: 3500,
            adjustedFlow: 3850,
            assigned: [
                { model: "ЛиАЗ-6213", count: 2 },
                { model: "ГАЗель NEXT", count: 4 }
            ],
            totalCapacity: 3850,
            served: 3500,
            deficit: 350
        },
        {
            hour: "08-09",
            baseFlow: 2500,
            adjustedFlow: 2750,
            assigned: [
                { model: "ПАЗ-32053", count: 3 },
                { model: "Yutong ZK6122H", count: 1 }
            ],
            totalCapacity: 2800,
            served: 2750,
            deficit: 0
        }
    ],

    fleetUsage: [
        { model: "ЛиАЗ-6213", classType: "Особо большой", available: 15, used: 10, totalCost: 36000 },
        { model: "ПАЗ-32053", classType: "Малый", available: 30, used: 20, totalCost: 42000 },
        { model: "ГАЗель NEXT", classType: "Особо малый", available: 37, used: 12, totalCost: 19800 },
        { model: "Yutong ZK6122H", classType: "Большой", available: 7, used: 3, totalCost: 12000 }
    ],

    routeInfo: {
        stops: 10,
        stopDuration: 3,
        cycleDuration: 120
    }
})

const exportReport = () => {
    console.log('Экспорт отчёта в Excel')
    alert('Функция экспорта будет реализована позже')
}
</script>

<template>
    <div class="container results-page">
        <AppHeader
            title="Результаты отчёта"
            :dayOfWeek="result.dayOfWeek"
            :knn="result.knn"
            :date="result.date"
        />
        <ResultSummary
            :totalCost="result.totalCost"
            :totalBuses="result.totalBuses"
            :servedPassengers="result.servedPassengers"
            :deficit="result.deficit"
        />
        <HourlyScheduleTable
            :data="result.hourlyData"
            :dayOfWeek="result.dayOfWeek"
        />
        <FleetUsageTable :fleet-usage="result.fleetUsage"/>
        <RouteInfo
            :stops="result.routeInfo.stops"
            :stopDuration="result.routeInfo.stopDuration"
            :cycleDuration="result.routeInfo.cycleDuration"
        />
        <div class="text-center mt-4 mb-5">
            <button class="btn btn-lg btn-success" @click="exportReport">
                Экспорт отчёта в Excel
            </button>
        </div>
    </div>
</template>

<style scoped>

</style>