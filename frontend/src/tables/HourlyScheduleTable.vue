<script setup>
import { computed } from "vue"

// Данные по часам - это output.json schedule, передаётся как есть

const props = defineProps({
    data: {
        type: Array,
        required: true,
        validator: (value) => value.every(item =>
            "interval" in item && // Интервал времени
            "demand" in item && // Спрос (пассажиры)
            "offered_capacity" in item && // Суммарная вместимость ТС
            "shortage" in item && // Дефицит
            "assigned" in item // { model, capacity, offered_capacity, trips, departures[] }
        )
    },
    dayOfWeek: { type: String, default: "" }
})

// Итоги по всей таблице
const totals = computed(() => {
    const result = { 
        demand: 0, 
        offered: 0, 
        served: 0, 
        shortage: 0 
    }

    props.data.forEach(item => {
        result.demand += item.demand || 0
        result.offered += item.offered_capacity || 0
        result.served += (item.demand || 0) - (item.shortage || 0)
        result.shortage += item.shortage || 0
    })

    return result
})

// Форматирование потока в тыс.
const formatFlow = (value) => {
    if (value === null || value === undefined) 
        return "-"
    return (value / 1000).toFixed(1) + " тыс."
}

// Форматирование целых чисел
const formatInt = (value) => {
    if (value === null || value === undefined) 
        return "-"
    return value.toLocaleString("ru-RU")
}

// Текстовое представление назначенных ТС
const formatAssigned = (assigned) => {
    if (!assigned || assigned.length === 0) 
        return "-"
    return assigned.map(a => `${a.model} (${a.trips} рейс., ${formatInt(a.offered_capacity)} мест)`).join(", ")
}
</script>

<template>
    <div class="hourly-schedule">
        <h3 class="schedule-title">Расписание по частям</h3>
        <p v-if="dayOfWeek" class="schedule-context">{{ dayOfWeek }}</p>
        <div class="table-responsive">
            <table class="table table-striped table-hover">
                <thead>
                    <tr>
                        <th>Время</th>
                        <th>Спрос</th>
                        <th>Назначенные ТС</th>
                        <th>Вместимость</th>
                        <th>Обслужено</th>
                        <th class="deficit-col">Дефицит</th>
                    </tr>
                </thead>
                <tbody>
                    <tr v-for="(item, idx) in data" :key="idx">
                        <td><strong>{{ item.interval }}</strong></td>
                        <td>{{ formatFlow(item.demand) }}</td>
                        <td>{{ formatAssigned(item.assigned) }}</td>
                        <td>{{ formatInt(item.offered_capacity) }}</td>
                        <td>{{ formatInt(item.demand - item.shortage) }}</td>
                        <td :class="['deficit-col', { 'text-danger': item.shortage > 0 }]">
                            {{ formatInt(item.shortage) }}
                        </td>
                    </tr>
                </tbody>
                <tfoot>
                    <tr class="table-active">
                        <th>Итого</th>
                        <th>{{ formatFlow(totals.demand) }}</th>
                        <th></th>
                        <th>{{ formatInt(totals.offered) }}</th>
                        <th>{{ formatInt(totals.served) }}</th>
                        <th :class="['deficit-col', { 'text-danger': totals.shortage > 0 }]">
                            {{ formatInt(totals.shortage) }}
                        </th>
                    </tr>
                </tfoot>
            </table>
        </div>
    </div>
</template>

<style scoped>
.schedule-title {
    border-left: 4px solid var(--accent);
    padding-left: 15px;
    margin-bottom: 20px;
    color: #fff;
}

.schedule-context {
    color: var(--text);
    margin-bottom: 1rem;
}
</style>