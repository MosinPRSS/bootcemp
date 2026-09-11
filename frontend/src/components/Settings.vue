<script setup>
import { computed, watch } from "vue"

const props = defineProps({
    title: { type: String, default: "" },
    modifiers: { type: Object, default: () => ({}) },
    schedule: { type: Object, default: () => ({}) },
    initialDate: { type: String, default: "" }
})
const emit = defineEmits(["calculate", "export", "update:date"])

const regions = [
    "Центральный",
    "Северо-Западный",
    "Южный",
    "Сибирский",
    "Дальневосточный",
    "Крайний Север"
]

const REGION_MULTIPLIERS = {
    "Дальневосточный": 1.3,
    "Крайний Север": 1.3
}
const DEFAULT_REGION_MULTIPLIER = 1

function regionMultiplier(region) {
    return REGION_MULTIPLIERS[region] ?? DEFAULT_REGION_MULTIPLIER
}

watch(() => props.modifiers.region, (r) => {
    if (r) props.modifiers.region_multiplier = regionMultiplier(r)
})

const seasons = ["Зима", "Весна", "Лето", "Осень"]

const daysOfWeek = [
    "Понедельник",
    "Вторник",
    "Среда",
    "Четверг",
    "Пятница",
    "Суббота",
    "Воскресенье"
]

const calcMode = computed({
    get: () => props.modifiers.calc_mode || "date",
    set: v => (props.modifiers.calc_mode = v)
})

// День недели живёт в schedule.day_of_week — именно его читает api.js
// (payload.schedule.day_of_week) и рендерит AppHeader на ResultsPage.
// ВАЖНО: не "modifiers.day" — то поле никуда не уходит.
const dayOfWeek = computed({
    get: () => props.schedule?.day_of_week || "",
    set: v => { if (props.schedule) props.schedule.day_of_week = v }
})

const dailyCoefficient = computed({
    get: () => props.schedule?.daily_coefficient ?? 1.0,
    set: v => {
        if (!props.schedule) return
        const n = Number(v)
        props.schedule.daily_coefficient = Number.isFinite(n) && n > 0 ? n : 1.0
    }
})

function weekdayFromIso(iso) {
    if (!iso) return ""
    const wd = new Date(iso).toLocaleDateString("ru-RU", { weekday: "long" })
    return wd.charAt(0).toUpperCase() + wd.slice(1)
}

const effectiveDate = computed(() =>
    calcMode.value === "range"
        ? props.schedule?.date_from
        : props.schedule?.date
)

watch(effectiveDate, (date, prev) => {
    if (!date) return
    const expectedPrev = weekdayFromIso(prev)
    const current = props.schedule?.day_of_week || ""
    // Если текущее значение пустое ИЛИ совпадает с тем, что мы бы
    // поставили из прошлой даты — значит его не трогали руками.
    if (!current || current === expectedPrev) {
        props.schedule.day_of_week = weekdayFromIso(date)
    }
}, { immediate: true })

const cycleHours = computed(() => {
    const s  = +props.modifiers.stops             || 0
    const sd = +props.modifiers.stop_duration_min || 0
    const t  = +props.modifiers.travel_time_min   || 0

    const minutes = s < 2 ? 0 : (s - 1) * t + (s - 2) * sd
    const hours = +(minutes / 60).toFixed(2)

    props.modifiers.route_cycle_hours = hours
    return hours
})

const onDate = () =>
    emit(
        "update:date",
        calcMode.value === "range" ? props.schedule.date_from : props.schedule.date
    )
</script>

<template>
    <div class="settings-card">
        <h2 class="header-title">{{ title }}</h2>

        <div class="settings-grid">
            <p class="settings-section-title">Регион и сезон</p>

            <div class="field">
                <label>Регион</label>
                <select class="form-select" v-model="modifiers.region">
                    <option value="" disabled selected>Регион не выбран</option>
                    <option v-for="r in regions" :key="r" :value="r">{{ r }}</option>
                </select>
            </div>

            <div class="field">
                <label>Суточный коэффициент</label>
                <input
                    type="number"
                    step="0.01"
                    min="0.01"
                    class="form-control"
                    v-model.number="dailyCoefficient"
                />
            </div>

            <div class="field">
                <label>Сезон</label>
                <select class="form-select" v-model="modifiers.season">
                    <option value="" disabled selected>Сезон не выбран</option>
                    <option v-for="s in seasons" :key="s" :value="s">{{ s }}</option>
                </select>
            </div>

            <div class="field">
                <label>День недели</label>
                <select class="form-select" v-model="dayOfWeek">
                    <option value="" disabled>День недели не выбран</option>
                    <option v-for="d in daysOfWeek" :key="d" :value="d">{{ d }}</option>
                </select>
            </div>

            <p class="settings-section-title">Маршрут</p>

            <div class="field">
                <label>Остановок, шт.</label>
                <input
                    type="number"
                    min="2"
                    class="form-control"
                    v-model.number="modifiers.stops"
                />
            </div>

            <div class="field">
                <label>Стоянка, мин</label>
                <input
                    type="number"
                    min="0"
                    class="form-control"
                    v-model.number="modifiers.stop_duration_min"
                />
            </div>

            <div class="field">
                <label>Движение между остановками, мин</label>
                <input
                    type="number"
                    min="0"
                    class="form-control"
                    v-model.number="modifiers.travel_time_min"
                />
            </div>
        </div>

        <div class="settings-summary">
            <span class="chip">Цикл маршрута: <b>{{ cycleHours }}</b> ч</span>
        </div>

        <div class="field field--check">
            <label class="form-check-label" for="long-ride">
                <input
                    class="form-check-input me-2"
                    id="long-ride"
                    type="checkbox"
                    v-model="modifiers.is_long_ride"
                />
                Длительный рейс
            </label>
        </div>

        <div class="settings-actions">
            <button class="btn btn-success" @click="emit('calculate')">Рассчитать</button>
            <button class="btn btn-outline-success" @click="emit('export')">Выгрузить в Excel</button>
        </div>
    </div>
</template>
