<script setup>
import { computed } from "vue"

const props = defineProps({
    title: { type: String, default: "" },
    modifiers: { type: Object, default: () => ({}) },
    schedule: { type: Object, default: () => ({}) },
    initialDate: { type: String, default: "" }
})
const emit = defineEmits(["calculate", "export", "update:date"])

const regions = ["Центральный", "Северо-Западный", "Южный", "Сибирский", "Дальневосточный"]

// Режим: "date" — по одной дате; "range" — диапазон (начало и конец ВКЛЮЧАЮТСЯ)
const calcMode = computed({
    get: () => props.modifiers.calc_mode || "date",
    set: v => (props.modifiers.calc_mode = v)
})

// Зима по интервалу MM-DD (учитываем переход через новый год)
const inWinter = (iso, from, to) => {
    if (!iso || !from || !to) return false
    const md = iso.slice(5)
    return from <= to ? (md >= from && md <= to) : (md >= from || md <= to)
}
const isFarNorth = computed({ get: () => !!props.modifiers.is_far_north, set: v => (props.modifiers.is_far_north = v) })

const season = computed(() => {
    if (isFarNorth.value) return "Крайний Север"
    return inWinter(props.schedule.date, props.modifiers.winter_from, props.modifiers.winter_to) ? "Зима" : "Лето"
})

// Либо зима, либо крайний север — не вместе
const surcharge = computed(() => {
    if (isFarNorth.value) return props.modifiers.far_north_surcharge || 0
    return season.value === "Зима" ? (props.modifiers.winter_surcharge || 0) : 0
})

const effectiveMultiplier = computed(() => {
    const d = props.schedule.daily_coefficient || 1
    const r = props.modifiers.region_multiplier || 1
    const m = +(d * r * (1 + surcharge.value)).toFixed(3)
    props.modifiers.effective_multiplier = m
    return m
})

// Цикл: первая посадка не считается, стоянка в конечной не считается
const cycleMinutes = computed(() => {
    const s = +props.modifiers.stops || 0, sd = +props.modifiers.stop_duration_min || 0, t = +props.modifiers.travel_time_min || 0
    const c = s < 2 ? 0 : (s - 1) * t + (s - 2) * sd
    props.modifiers.route_cycle_minutes = c
    return c
})

const onDate = () => emit("update:date", calcMode.value === "range" ? props.schedule.date_from : props.schedule.date)
</script>

<template>
    <div class="settings-card">
        <h2 class="header-title">{{ title }}</h2>
        <div class="settings-grid">
            <p class="settings-section-title">Регион и коэффициенты</p>
            <div class="field"><label>Регион</label>
                <select class="form-select" v-model="modifiers.region">
                    <option value="" disabled>Регион не выбран</option>
                    <option v-for="r in regions" :key="r" :value="r">{{ r }}</option>
                </select>
            </div>
            <div class="field"><label>Коэф. региона</label><input type="number" step="0.01" class="form-control" v-model.number="modifiers.region_multiplier" /></div>
            <div class="field"><label>Надбавка «зима»</label><input type="number" step="0.01" class="form-control" v-model.number="modifiers.winter_surcharge" :disabled="isFarNorth" /></div>
            <div class="field"><label>Надбавка «Крайний Север»</label><input type="number" step="0.01" class="form-control" v-model.number="modifiers.far_north_surcharge" :disabled="!isFarNorth" /></div>
            <p class="settings-section-title">Период и сезон</p>
            <div class="field"><label>Тип расчёта</label>
                <select class="form-select" v-model="calcMode" :disabled="isFarNorth">
                    <option value="date">По дате</option>
                    <option value="range">По диапазону дат</option>
                </select>
            </div>
            <template v-if="!isFarNorth">
                <div class="field" v-if="calcMode === 'date'"><label>Дата</label><input type="date" class="form-control" v-model="schedule.date" @change="onDate" /></div>
                <template v-else>
                    <div class="field"><label>Дата начала (вкл.)</label><input type="date" class="form-control" v-model="schedule.date_from" @change="onDate" /></div>
                    <div class="field"><label>Дата окончания (вкл.)</label><input type="date" class="form-control" v-model="schedule.date_to" @change="onDate" /></div>
                </template>
                <div class="field"><label>Зима с (ММ-ДД)</label><input class="form-control" v-model="modifiers.winter_from" /></div>
                <div class="field"><label>Зима по (ММ-ДД)</label><input class="form-control" v-model="modifiers.winter_to" /></div>
            </template>
            <div class="field field--check">
                <label class="form-check-label" for="far-north">
                    <input class="form-check-input me-2" id="far-north" type="checkbox" v-model="isFarNorth" />
                    Крайний Север (даты не используются)
                </label>
            </div>
            <p class="settings-section-title">Маршрут (в минутах)</p>
            <div class="field"><label>Остановок, шт.</label><input type="number" min="2" class="form-control" v-model.number="modifiers.stops" /></div>
            <div class="field"><label>Стоянка, мин</label><input type="number" min="0" class="form-control" v-model.number="modifiers.stop_duration_min" /></div>
            <div class="field"><label>Движение между остановками, мин</label><input type="number" min="0" class="form-control" v-model.number="modifiers.travel_time_min" /></div>
        </div>
        <div class="settings-summary">
            <span class="chip">Сезон/надбавка: <b>{{ season }}</b> (+{{ (surcharge*100).toFixed(0) }}%)</span>
            <span class="chip">Эффективный коэффициент: <b>{{ effectiveMultiplier }}</b></span>
            <span class="chip">Цикл маршрута: <b>{{ cycleMinutes }}</b> мин</span>
        </div>
        <div class="settings-actions">
            <button class="btn btn-success" @click="emit('calculate')">Рассчитать</button>
            <button class="btn btn-outline-success" @click="emit('export')">Выгрузить в Excel</button>
        </div>
    </div>
</template>