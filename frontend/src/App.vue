<script setup>
import { computed, ref } from "vue"
import ResultsPage from "./pages/ResultsPage.vue"
import DataSetup from "./pages/DataSetup.vue"

// Текущая страница: "setup" | "results"
const currentPage = ref("setup")

// Загруженные / введённые вручную данные
const inputData = ref(null)
const outputData = ref(null)

// Флаги загрузки
const isLoadingInput = ref(false)
const isLoadingOutput = ref(false)

// Ошибка
const error = ref(null)

// Выбранная дата (для отчёта)
const selectedDate = ref("")

// День недели: первая буква перевести в большую
const dayOfWeekFromDate = computed(() => {
    if (!selectedDate.value) 
      return ""
    const wd = new Date(selectedDate.value).toLocaleDateString("ru-RU", { weekday: "long" })
    return wd.charAt(0).toUpperCase() + wd.slice(1) 
})

// Итоговая дата отчёта
const reportDate = computed(() => {
    if (selectedDate.value)
        return selectedDate.value
    if (inputData.value?.schedule?.date)
        return inputData.value.schedule.date
    return new Date().toISOString().slice(0, 10)
})

// Загрузка input.json
const loadInputData = async () => {
    isLoadingInput.value = true
    error.value = null
    try 
    {
        const response = await fetch("/input.json")
        if (!response.ok)
            throw new Error(`HTTP ${response.status}: input.json`)
        inputData.value = await response.json()
        if (inputData.value?.schedule?.date)
            selectedDate.value = inputData.value.schedule.date
    } 
    catch (e) 
    {
        error.value = e.message
    } 
    finally 
    {
        isLoadingInput.value = false
    }
}

// Создание пустого шаблона (ручной ввод)
// Структура полностью совпадает с input.json, только значения пустые
const createEmptyInput = () => ({
    fleet: [
        // Одна пустая строка — чтобы пользователю было с чего начать
        {
            model: "",
            class: "Малый",
            max_available: 0,
            capacity: 0,
            base_cost_per_hour: 0
        }
    ],
    modifiers: {
        region: "",
        season: "Лето",
        route_cycle_hours: 2.0,
        stops: 0,
        stop_duration_min: 0
    },
    schedule: {
        date: new Date().toISOString().slice(0, 10),
        day_of_week: new Date().toLocaleDateString("ru-RU", { weekday: "long" }),
        daily_coefficient: 1.0,
        hourly_flow: {}
    }
})

// Инициализация ручного ввода
const initEmptyInput = () => {
    error.value = null
    inputData.value = createEmptyInput()
    selectedDate.value = inputData.value.schedule.date
}

// Загрузка output.json (автоматически, без кнопки)
const loadOutputData = async () => {
    isLoadingOutput.value = true
    error.value = null
    try 
    {
        const response = await fetch("/output.json")
        if (!response.ok)
            throw new Error(`HTTP ${response.status}: output.json`)
        outputData.value = await response.json()
    } 
    catch (e) 
    {
        error.value = e.message
    } 
    finally 
    {
        isLoadingOutput.value = false
    }
}

// Переход к результатам 
const goToResults = async () => {
    // Подставляем актуальный день недели из выбранной даты
    if (inputData.value?.schedule)
        inputData.value.schedule.day_of_week = dayOfWeekFromDate.value
    if (!outputData.value)
        await loadOutputData()
    currentPage.value = "results"
}

// Возврат на настройки
const goToSetup = () => (currentPage.value = "setup")
</script>

<template>
    <div class="app-container">
        <div v-if="error" class="alert alert-danger m-4">{{ error }}</div>
        <nav class="app-nav">
            <button
                class="nav-btn"
                :class="{ active: currentPage === 'setup' }"
                @click="goToSetup"
            >
                Входные данные
            </button>
            <button
                class="nav-btn"
                :class="{ active: currentPage === 'results' }"
                @click="goToResults"
                :disabled="isLoadingOutput"
            >
                {{ isLoadingOutput ? "Загрузка..." : "Результаты отчёта" }}
            </button>
        </nav>
        <DataSetup
            v-if="currentPage === 'setup'"
            :input-data="inputData"
            :is-loading="isLoadingInput"
            @load-input="loadInputData"
            @create-empty="initEmptyInput"
            @calculate="goToResults"
            @update:date="(d) => (selectedDate = d)"
        />
        <ResultsPage
            v-else
            :output-data="outputData"
            :input-data="inputData"
            :is-loading="isLoadingOutput"
            :report-date="reportDate"
            :error="error"
            @back="goToSetup"
        />
    </div>
</template>