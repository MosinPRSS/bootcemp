<script setup>
import { computed } from "vue"

const props = defineProps({
    // Массив с данными по использованию автопарка
    fleetUsage: {
        type: Array,
        required: true,
        validator: (value) => {
            return value.every(item =>
                "model" in item && // Модель 
                "available" in item && // в наличии
                "used" in item &&
                "totalCost" in item
            )
        }
    }
})

// Вычисляем итоги (всего доступно, всего использовано, общая стоимость)
const totals = computed(() => {
    // Иницализация результатов для подсчёта
    const result = {
        available: 0,
        used: 0,
        totalCost: 0
    }

    // Вычисление результата
    props.fleetUsage.forEach(item => {
        result.available += item.available || 0
        result.used += item.used || 0
        result.totalCost += item.totalCost || 0
    })
    return result
})

// Форматирование денег (без копеек)
const formatCurrency = (value) => {
    if (value === null || value === undefined) 
        return '-'
    return value.toLocaleString('ru-RU', { minimumFractionDigits: 0, maximumFractionDigits: 0 }) + ' ₽'
}

// Процент загрузки
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
                        <th>Использовано</th>
                        <th>Загрузка</th>
                        <th class="text-end">Суммарная стоимость</th>
                    </tr>
                </thead>
                <tbody>
                    <tr v-for="(item, idx) in fleetUsage" :key="idx">
                        <td><strong>{{ item.model }}</strong></td>
                        <td v-if="fleetUsage.some(item => 'classType' in item)">{{ item.classType || '—' }}</td>
                        <td>{{ item.available }}</td>
                        <td>
                            <span :class="{
                                'text-warning': item.used === item.available,
                                'text-danger': item.used > item.available
                            }">
                                {{ item.used }}
                            </span>
                            <small v-if="item.used === item.available" class="text-muted"> (весь парк)</small>
                        </td>
                        <td>
                            <div class="progress" style="height: 20px; width: 120px;">
                                <div
                                    class="progress-bar"
                                    role="progressbar"
                                    :style="{ width: getUsagePercent(item.used, item.available) + '%' }"
                                    :class="getProgressClass(getUsagePercent(item.used, item.available))"
                                    :aria-valuenow="getUsagePercent(item.used, item.available)"
                                    aria-valuemin="0"
                                    aria-valuemax="100"
                                >
                                    {{ getUsagePercent(item.used, item.available) }}%
                                </div>
                            </div>
                        </td>
                        <td class="text-end">{{ formatCurrency(item.totalCost) }}</td>
                    </tr>
                </tbody>
                <tfoot>
                    <tr class="table-active">
                        <th>Итого</th>
                        <th v-if="fleetUsage.some(item => 'classType' in item)"></th>
                        <th>{{ totals.available }}</th>
                        <th>{{ totals.used }}</th>
                        <th></th>
                        <th class="text-end">{{ formatCurrency(totals.totalCost) }}</th>
                    </tr>
                </tfoot>
            </table>
        </div>
    </div>
</template>

<style scoped>

</style>