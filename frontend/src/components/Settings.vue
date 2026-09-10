<script setup>
import { ref, computed } from "vue"

const props = defineProps({
    title: { type: String, default: "" },
    modifiers: { type: Object, default: () => ({}) },
    schedule: { type: Object, default: () => ({}) }
})

const emit = defineEmits(["calculate"])

const calcType = ref("")
const selectedHour = ref(null)
const selectedWeekday = ref(null)
const selectedDate = ref("")
const isFarNorth = ref(props.modifiers?.region === "Крайний Север")

const hourOptions = computed(() => {
    const options = []
    for (let i = 5; i <= 23; i++)
        options.push({ value: i, label: `${i}-${i + 1}` })
        
    options.push({ value: 24, label: "24-01" })
    return options
})

const weekdayOptions = [
    { value: "ПН", label: "Понедельник" },
    { value: "ВТ", label: "Вторник" },
    { value: "СР", label: "Среда" },
    { value: "ЧТ", label: "Четверг" },
    { value: "ПТ", label: "Пятница" },
    { value: "СБ", label: "Суббота" },
    { value: "ВС", label: "Воскресенье" }
]

const currentSeason = computed(() => {
    if (!selectedDate.value) 
        return ""
    const date = new Date(selectedDate.value)
    const month = date.getMonth()
    return month >= 10 || month <= 2 ? "Зима" : "Лето"
})
</script>

<template>
    <h2 class="header-title">{{ title }}</h2>
    <label for="calc-type" class="form-label">Выберите тип расчета:</label>
    <select id="calc-type" class="form-select-sm mb-3" v-model="calcType">
        <option value="" disabled>Тип расчета не выбран</option>
        <option value="hours">Расчет по часам</option>
        <option value="weekdays">Расчет по дням недели</option>
    </select>
    <div v-if="calcType === 'hours'" class="mb-3">
        <label for="hourSelection" class="form-label">Выберите час:</label>
        <select id="hourSelection" class="form-select-sm" v-model="selectedHour">
            <option value="" disabled>Выберите час...</option>
            <option
                v-for="hourOption in hourOptions"
                :key="hourOption.value"
                :value="hourOption.value"
            >
                {{ hourOption.label }}
            </option>
        </select>
    </div>
    <div v-if="calcType === 'weekdays'" class="mb-3">
        <label for="weekdaySelection" class="form-label">Выберите день недели:</label>
        <select
            id="weekdaySelection"
            class="form-select-sm"
            v-model="selectedWeekday"
        >
            <option value="" disabled>Выберите день недели...</option>
            <option
                v-for="weekdayOption in weekdayOptions"
                :key="weekdayOption.value"
                :value="weekdayOption.value"
            >
                {{ weekdayOption.label }}
            </option>
        </select>
    </div>
    <label for="date-input" class="form-label">Выберите дату:</label>
    <input
        id="date-input"
        type="date"
        class="form-control-sm mb-3"
        v-model="selectedDate"
    />
    <div class="form-check mb-3">
        <input
            class="form-check-input"
            id="far-north"
            type="checkbox"
            v-model="isFarNorth"
        />
        <label for="far-north" class="form-check-label">
            Регион — крайний север
        </label>
    </div>
    <p v-if="currentSeason" class="text-muted">Сезон: {{ currentSeason }}</p>
    <button class="btn btn-success mt-3" @click="emit('calculate')">
        Рассчитать и перейти к результатам
    </button>
</template>

<style scoped>
#date-input {
  width: min-content;
}

.form-check {
  width: 35vh;
}

.form-label {
  display: block;
  margin-bottom: 0.25rem;
}
</style>