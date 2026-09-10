<script setup>
import { computed } from "vue"

// Данные по использованию автопарка.
// Формируются в ResultsPage объединением input.fleet и output.fleet_usage: model, classType, available, trips, busyMinutes, baseCostPerHour, totalCost
const props = defineProps({
    fleetUsage: {
        type: Array,
        required: true,
        default: () => [],
        validator: (value) => value.every(item =>
            "model" in item && // Модель
            "available" in item && // В наличии
            "trips" in item && // Пути
            "totalCost" in item // Общая стоимость
        )
    }
})

// Итоги по всей таблице
const totals = computed(() => {
    const result = { 
        available: 0, // В наличии
        trips: 0, // Пути
        busyMinutes: 0, // Занятые минуты
        totalCost: 0 // Общая стоимость
    }
    
    props.fleetUsage.forEach(item => {
        result.available += item.available || 0
        result.trips += item.trips || 0
        result.busyMinutes += item.busyMinutes || 0
        result.totalCost += item.totalCost || 0
    })
    return result
})

// Форматирование денег (без копеек)
const formatCurrency = (value) => {
    if (value === null || value === undefined) 
        return "-"
    return value.toLocaleString("ru-RU", {
        minimumFractionDigits: 0,
        maximumFractionDigits: 0
    }) + " ₽"
}

// Форматирование целых чисел
const formatInt = (value) => {
    if (value === null || value === undefined) 
        return "-"
    return value.toLocaleString("ru-RU")
}

// Процент загрузки: сколько рейсов на одну доступную машину,
// нормируем к 100 % - 100 рейсов на машину
const getUsagePercent = (trips, available) => {
    if (!available) 
        return 0
    const pct = Math.round((trips / (available * 100)) * 100)
    return Math.min(pct, 100)
}

// Цвет прогресс-бара в зависимости от загрузки
const getProgressFunc = (percent) => {
    if (percent < 70) 
        return "bg-success"
    if (percent < 90) 
        return "bg-warning"
    return "bg-danger"
}
</script>

<template>
    <div class="fleet-usage">
        <h3 class="fleet-title">Использование автопарка</h3>
        <div class="table-responsive">
            <table class="table table-striped table-hover">
                <thead>
                    <tr>
                        <th>Модель</th>
                        <th v-if="fleetUsage.some(item => 'classType' in item)">Класс</th>
                        <th>Доступно</th>
                        <th>Рейсов</th>
                        <th>Загрузка</th>
                        <th class="text-end">Время работы</th>
                        <th class="text-end">Расчётная стоимость</th>
                    </tr>
                </thead>
                <tbody>
                    <tr v-for="(item, idx) in fleetUsage" :key="idx">
                        <td><strong>{{ item.model }}</strong></td>
                        <td v-if="fleetUsage.some(item => 'classType' in item)">
                            {{ item.classType || '—' }}
                        </td>
                        <td>{{ formatInt(item.available) }}</td>
                        <td>
                            <span :class="{
                                'text-warning': item.trips >= item.available * 10,
                                'text-danger':  item.trips >= item.available * 20
                            }">
                                {{ formatInt(item.trips) }}
                            </span>
                        </td>
                        <td>
                            <div class="progress" style="height: 20px; width: 120px;">
                                <div
                                    class="progress-bar"
                                    role="progressbar"
                                    :style="{ width: getUsagePercent(item.trips, item.available) + '%' }"
                                    :class="getProgressFunc(getUsagePercent(item.trips, item.available))"
                                    :aria-valuenow="getUsagePercent(item.trips, item.available)"
                                    aria-valuemin="0"
                                    aria-valuemax="100"
                                >
                                    {{ getUsagePercent(item.trips, item.available) }}%
                                </div>
                            </div>
                        </td>
                        <td class="text-end">{{ formatInt(item.busyMinutes) }} мин.</td>
                        <td class="text-end">{{ formatCurrency(item.totalCost) }}</td>
                    </tr>
                </tbody>
                <tfoot>
                    <tr class="table-active">
                        <th>Итого</th>
                        <th v-if="fleetUsage.some(item => 'classType' in item)"></th>
                        <th>{{ formatInt(totals.available) }}</th>
                        <th>{{ formatInt(totals.trips) }}</th>
                        <th></th>
                        <th class="text-end">{{ formatInt(totals.busyMinutes) }} мин.</th>
                        <th class="text-end">{{ formatCurrency(totals.totalCost) }}</th>
                    </tr>
                </tfoot>
            </table>
        </div>
    </div>
</template>

<style scoped>
.fleet-title {
    border-left: 4px solid var(--accent);
    padding-left: 15px;
    margin-bottom: 20px;
    color: #fff;
}
</style>