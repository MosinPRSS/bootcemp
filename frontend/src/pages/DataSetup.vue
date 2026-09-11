<script setup>
import { ref, computed, watch } from "vue"
import AppHeader from "../components/AppHeader.vue"
import Table from "../tables/Table.vue"
import Settings from "../components/Settings.vue"
import FlowTable from "../components/FlowTable.vue"

const props = defineProps({
    inputData: { type: Object, default: null },
    isLoading: { type: Boolean, default: false }
})

const emit = defineEmits([
    "loadFleet",
    "loadData",
    "createEmpty",
    "calculate",
    "export",
    "update:date",
    "update:flow"
])

const tab = ref("fleet")

// Локальные черновики — используются, пока inputData ещё не создан родителем
const localFleet = ref([])
const localModifiers = ref({})
const localSchedule = ref({ hourly_flow: {} })

// Скрытые input'ы для выбора файлов
const fleetInput = ref(null)
const modifiersInput = ref(null)
const flowInput = ref(null)

// Эффективные данные: берём из inputData, иначе — из черновиков
const effectiveFleet = computed(() => props.inputData?.fleet || localFleet.value)
const effectiveModifiers = computed(() => props.inputData?.modifiers || localModifiers.value)
const effectiveSchedule = computed(() => props.inputData?.schedule || localSchedule.value)
const currentFlow = computed(() => effectiveSchedule.value.hourly_flow || {})

const initialDate = computed(() => props.inputData?.schedule?.date || "")

// --- Коэффициент региона ---
const REGION_MULTIPLIERS = {
    "Дальневосточный": 1.3,
    "Крайний Север": 1.3
}
const DEFAULT_REGION_MULTIPLIER = 1

function regionMultiplier(region) {
    return REGION_MULTIPLIERS[region] ?? DEFAULT_REGION_MULTIPLIER
}

// --- Вспомогательные нормализаторы ---

// "06-07" -> "6-7"
function normalizeSlot(slot) {
    const [a, b] = String(slot).split("-")
    const start = Number(a), end = Number(b)
    if (!Number.isInteger(start) || !Number.isInteger(end)) return String(slot)
    return `${start}-${end}`
}

// --- Загрузка автопарка ---

function extractFleet(json) {
    if (Array.isArray(json?.fleet)) return json.fleet
    if (Array.isArray(json)) return json
    return null
}

function normalizeVehicle(v) {
    return {
        model: String(v.model ?? ""),
        class: String(v.class ?? "Малый"),
        max_available: Number(v.max_available) || 0,
        seats_total: Number(v.seats_total ?? v.capacity) || 0,
        seats_seated: Number(v.seats_seated ?? v.seats) || 0,
        base_cost_per_hour: Number(v.base_cost_per_hour) || 0,
        max_machine_hours: Number(v.max_machine_hours) || 8
    }
}

function applyFleet(fleet) {
    if (props.inputData) {
        props.inputData.fleet = fleet
    } else {
        localFleet.value = fleet
    }
}

// --- Загрузка модификаторов ---

function extractModifiers(json) {
    if (json?.modifiers && typeof json.modifiers === "object") return json.modifiers
    return null
}

function normalizeModifiers(m) {
    const out = {}

    if (m.region !== undefined) out.region = String(m.region)
    if (m.season !== undefined) out.season = String(m.season)
    if (m.is_long !== undefined) out.is_long_ride = Boolean(m.is_long)
    if (m.is_long_ride !== undefined) out.is_long_ride = Boolean(m.is_long_ride)
    if (m.is_far_north !== undefined) out.is_far_north = Boolean(m.is_far_north)

    // Если region_multiplier не пришёл — считаем по региону.
    // Если пришёл — доверяем значению из JSON.
    if (m.region_multiplier !== undefined) {
        out.region_multiplier = Number(m.region_multiplier)
    } else if (out.region !== undefined) {
        out.region_multiplier = regionMultiplier(out.region)
    }

    if (m.stops !== undefined) out.stops = Number(m.stops)
    if (m.stop_duration_min !== undefined) out.stop_duration_min = Number(m.stop_duration_min)
    if (m.travel_time_min !== undefined) out.travel_time_min = Number(m.travel_time_min)

    // route_cycle_hours из JSON: синтезируем stops/duration/travel,
    // потому что Settings.vue пересчитывает cycle_hours из них.
    const hasStops = out.stops !== undefined
    const hasStopDur = out.stop_duration_min !== undefined
    const hasTravel = out.travel_time_min !== undefined

    if (m.route_cycle_hours !== undefined && !hasStops && !hasStopDur && !hasTravel) {
        const H = Number(m.route_cycle_hours)
        if (H > 0) {
            const stops = 5
            const sd = 1
            const t = Math.max(1, Math.round((H * 60 - (stops - 2) * sd) / (stops - 1)))
            out.stops = stops
            out.stop_duration_min = sd
            out.travel_time_min = t
        }
    }

    return out
}

function applyModifiers(mods) {
    if (props.inputData) {
        props.inputData.modifiers = mods
    } else {
        localModifiers.value = mods
    }
}

// --- Загрузка расписания (day_of_week, daily_coefficient) ---

function extractSchedule(json) {
    if (json?.schedule && typeof json.schedule === "object") return json.schedule
    return null
}

function normalizeSchedule(s) {
    const out = {}
    if (s.day_of_week !== undefined) out.day_of_week = String(s.day_of_week)
    if (s.daily_coefficient !== undefined) out.daily_coefficient = Number(s.daily_coefficient)
    if (s.date !== undefined) out.date = String(s.date)
    return out
}

// Мержим с уже существующим schedule, чтобы не терять hourly_flow
function applySchedule(patch) {
    if (props.inputData) {
        if (!props.inputData.schedule) props.inputData.schedule = {}
        Object.assign(props.inputData.schedule, patch)
    } else {
        localSchedule.value = { ...localSchedule.value, ...patch }
    }
}

// --- Загрузка потока ---

function looksLikeFlow(obj) {
    if (!obj || typeof obj !== "object" || Array.isArray(obj)) return false
    const entries = Object.entries(obj)
    if (entries.length === 0) return false
    return entries.every(([k, v]) =>
        /^\d{1,2}-\d{1,2}$/.test(k) &&
        (typeof v === "number" || (v && typeof v === "object" && "total" in v))
    )
}

function extractFlow(json) {
    if (json?.schedule?.hourly_flow && typeof json.schedule.hourly_flow === "object")
        return json.schedule.hourly_flow
    if (json?.hourly_flow && typeof json.hourly_flow === "object")
        return json.hourly_flow
    if (looksLikeFlow(json))
        return json
    return null
}

function applyFlow(flow) {
    if (props.inputData) {
        if (!props.inputData.schedule) props.inputData.schedule = {}
        props.inputData.schedule.hourly_flow = flow
    } else {
        localSchedule.value = { ...localSchedule.value, hourly_flow: flow }
    }
    emit("update:flow", flow)
}

// --- Общий помощник: открыть файл и распарсить ---

async function readJSONFile(event, onData) {
    const file = event.target.files?.[0]
    if (!file) return
    try {
        const text = await file.text()
        const json = JSON.parse(text)
        onData(json)
    } catch (e) {
        alert(`Ошибка загрузки: ${e.message}`)
    } finally {
        event.target.value = ""
    }
}

// --- Обработчики кнопок ---

const triggerLoadFleet = () => fleetInput.value?.click()
const triggerLoadModifiers = () => modifiersInput.value?.click()
const triggerLoadFlow = () => flowInput.value?.click()

const onFleetFileChange = (event) => readJSONFile(event, (json) => {
    const raw = extractFleet(json)
    if (!raw) throw new Error("в файле не найден массив fleet")
    applyFleet(raw.map(normalizeVehicle))
})

// Модификаторы: с одного и того же файла подхватываем и modifiers, и schedule
const onModifiersFileChange = (event) => readJSONFile(event, (json) => {
    const rawMods = extractModifiers(json)
    if (!rawMods) throw new Error("в файле не найден объект modifiers")
    applyModifiers(normalizeModifiers(rawMods))

    const rawSched = extractSchedule(json)
    if (rawSched) {
        const patch = normalizeSchedule(rawSched)
        if (Object.keys(patch).length) applySchedule(patch)
    }
})

const onFlowFileChange = (event) => readJSONFile(event, (json) => {
    const raw = extractFlow(json)
    if (!raw) throw new Error("в файле не найден hourly_flow")

    const normalized = {}
    for (const [slot, v] of Object.entries(raw)) {
        const s = normalizeSlot(slot)
        const n = typeof v === "number" ? v : (v?.total ?? 0)
        normalized[s] = Math.max(0, Math.round(Number(n) || 0))
    }
    applyFlow(normalized)
})

// --- Сборка актуального input для расчёта/экспорта ---

// Собираем "эффективные" данные в один снимок и отдаём наверх.
// Глубокое клонирование обязательно: иначе родитель и DataSetup будут
// держать ссылки на одни и те же объекты.
function buildInput() {
    const fleet = JSON.parse(JSON.stringify(effectiveFleet.value || []))
    const modifiers = JSON.parse(JSON.stringify(effectiveModifiers.value || {}))
    const schedule = JSON.parse(JSON.stringify(effectiveSchedule.value || { hourly_flow: {} }))

    // Автозаполнение дня недели из даты — иначе api.js отдаст
    // «Не задан день недели (выбери дату)».
    if (!schedule.day_of_week && schedule.date) {
        const wd = new Date(schedule.date).toLocaleDateString("ru-RU", { weekday: "long" })
        schedule.day_of_week = wd.charAt(0).toUpperCase() + wd.slice(1)
    }

    return { fleet, modifiers, schedule }
}

const onCalculate = () => emit("calculate", buildInput())
const onExport    = () => emit("export",    buildInput())

// --- Перенос локальных черновиков в inputData, когда он появится ---
watch(() => props.inputData, (newVal) => {
    if (!newVal) return

    if (localFleet.value.length && (!newVal.fleet || newVal.fleet.length === 0)) {
        newVal.fleet = localFleet.value
    }
    if (Object.keys(localModifiers.value).length &&
        (!newVal.modifiers || Object.keys(newVal.modifiers).length === 0)) {
        newVal.modifiers = localModifiers.value
    }

    const draftSched = localSchedule.value
    if (draftSched && Object.keys(draftSched).length) {
        if (!newVal.schedule) newVal.schedule = {}
        for (const k of Object.keys(draftSched)) {
            if (newVal.schedule[k] === undefined ||
                (typeof newVal.schedule[k] === "object" &&
                 Object.keys(newVal.schedule[k] || {}).length === 0)) {
                newVal.schedule[k] = draftSched[k]
            }
        }
    }

    localFleet.value = []
    localModifiers.value = {}
    localSchedule.value = { hourly_flow: {} }
}, { immediate: false })
</script>

<template>
    <AppHeader title="Расчёт маршрута" :date="initialDate" />

    <ul class="nav nav-tabs justify-content-center mb-3">
        <li class="nav-item"><a class="nav-link" :class="{active: tab==='fleet'}" @click="tab='fleet'">Автопарк</a></li>
        <li class="nav-item"><a class="nav-link" :class="{active: tab==='settings'}"  @click="tab='settings'">Модификаторы</a></li>
        <li class="nav-item"><a class="nav-link" :class="{active: tab==='flow'}"  @click="tab='flow'">Пассажиропоток</a></li>
    </ul>

    <div class="text-center mb-4 d-flex justify-content-center gap-2">
        <button
            v-if="tab==='fleet'"
            class="btn btn-primary"
            @click="triggerLoadFleet"
            :disabled="isLoading"
        >
            Загрузить автопарк (JSON)
        </button>

        <button
            v-if="tab==='settings'"
            class="btn btn-primary"
            @click="triggerLoadModifiers"
            :disabled="isLoading"
        >
            Загрузить модификаторы (JSON)
        </button>

        <button
            v-if="tab==='flow'"
            class="btn btn-primary"
            @click="triggerLoadFlow"
            :disabled="isLoading"
        >
            Загрузить пассажиропоток (JSON)
        </button>

        <button
            class="btn btn-outline-primary"
            @click="emit('createEmpty')"
            :disabled="isLoading"
        >
            Заполнить вручную
        </button>

        <input ref="fleetInput" type="file" accept=".json,application/json" class="d-none" @change="onFleetFileChange" />
        <input ref="modifiersInput" type="file" accept=".json,application/json" class="d-none" @change="onModifiersFileChange" />
        <input ref="flowInput" type="file" accept=".json,application/json" class="d-none" @change="onFlowFileChange" />
    </div>

    <Table v-if="tab==='fleet'" :vehicles="effectiveFleet" />

    <Settings
        v-if="tab==='settings'"
        title="Настройки"
        :modifiers="effectiveModifiers"
        :schedule="effectiveSchedule"
        :initial-date="effectiveSchedule?.date || ''"
        @update:date="(d) => emit('update:date', d)"
        @calculate="onCalculate"
        @export="onExport"
    />

    <FlowTable v-if="tab === 'flow'" :flow="currentFlow" />
</template>