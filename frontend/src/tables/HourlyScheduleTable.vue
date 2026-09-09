<script setup>
import { computed } from "vue"

const props = defineProps({
    // Массив с данными по часам
    data: {
        type: Array,
        required: true,
        validator: (value) => {
            return value.every(item => {
                "hour" in item && // Временной интервал
                "baseFlow" in item && // Базовый пасажиропоток
                "adjustedFlow" in item && // Пассажиропоток с учётом коэффициента отношения дня к среднему
                "assigned" in item && // Массив назначенных ТС в этот час
                "totalCapacity" in item && // Общая вместимость всех назначенных ТС
                "served" in item && // Количество фактически обслуженных пассажиров
                "deficit" in item // Де
            })
        }
    },
    // Названия дня недели
    dayOfWeek: { type: String, default: "" }
})

// Вычисление итогов
const totals = computed(() => {
    // Иницализация результатов для подсчёта
    const result = {
        baseFlow: 0,
        adjustedFlow: 0,
        totalCapacity: 0,
        served: 0,
        deficit: 0
    }

    // Вычисление результата
    props.data.forEach(item => {
        result.baseFlow += item.baseFlow || 0
        result.adjustedFlow += item.adjustedFlow || 0
        result.totalCapacity += item.totalCapacity || 0
        result.served += item.served || 0
        result.deficit += item.deficit || 0
    })
    return result
})

// Форматирование чисел (тыс.)
const formatFlow= (value) => {
    if (value === null || value === undefined) 
        return "-"
    return (value / 1000).toFixed(1) + ' тыс.'
}

// Форматирование целых чисел (для вместимости и пассажиров)
const formatInt = (value) => {
    if (value === null || value === undefined) 
        return "-"
    return value.toLocaleString("ru-RU")
}

// Текстовое представление назанченных ТС

const formatAssigned = (assigned) => {
    if (!assigned || assigned.length === 0)
        return "-"
    return assigned.map(a => `${a.model} (${a.count} шт.)`).join(', ')
}
</script>

<template>
    <div class="hourly-schedule">
        <h3 class="schedule-title">Расписание по частям</h3>
        <p v-if="dayOfWeek" class="schedule-context">
            {{ dayOfWeek }}
        </p>
        <div class="table-responsive">
            <table class="table table-striped table-hover">
                <thead>
                    <tr>
                        <th>Время</th>
                        <th>Исходный поток</th>
                        <th>Итоговый поток</th>
                        <th>Назначенные ТС</th>
                        <th>Вместимость</th>
                        <th>Обслужено</th>
                        <th class="deficit-col">Дефицит</th>
                    </tr>
                </thead>
                <tbody>
                    <tr v-for="(item, idx) in data" :key="idx">
                        <td><strong>{{ item.hour }}</strong></td>
                        <td>{{ formatFlow(item.baseFlow) }}</td>
                        <td>{{ formatFlow(item.adjustedFlow) }}</td>
                        <td>{{ formatAssigned(item.assigned) }}</td>
                        <td>{{ formatInt(item.totalCapacity) }}</td>
                        <td>{{ formatInt(item.served) }}</td>
                        <td :class="['deficit-col', { 'text-danger': item.deficit > 0 }]">
                            {{ formatInt(item.deficit) }}
                        </td>
                    </tr>
                </tbody>
                <tfoot>
                    <tr class="table-active">
                        <th>Итого</th>
                        <th>{{ formatFlow(totals.baseFlow) }}</th>
                        <th>{{ formatFlow(totals.adjustedFlow) }}</th>
                        <th></th>
                        <th>{{ formatInt(totals.totalCapacity) }}</th>
                        <th>{{ formatInt(totals.served) }}</th>
                        <th :class="['deficit-col', { 'text-danger': totals.deficit > 0 }]">
                            {{ formatInt(totals.deficit) }}
                        </th>
                    </tr>
                </tfoot>
            </table>
        </div>
    </div>
</template>

<style scoped>

</style>