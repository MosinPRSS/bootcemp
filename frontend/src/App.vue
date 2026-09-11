<script setup>
import { computed, ref } from "vue"
import { calculateReport, downloadExcel } from "./api"
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

// Флаги результата
const exportOnly = ref(false)

// Ошибка
const error = ref(null)

// Выбранная дата (для отчёта)
const selectedDate = ref("")

// День недели: первая буква в верхнем регистре
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

// Создание пустого шаблона (ручной ввод)
// Структура полностью совпадает с input.json, только значения пустые
const createEmptyInput = () => ({
    fleet: [
        {
            model: "",
            class: "Малый",
            max_available: 0,
            seats_total: 0,
            seats_seated: 0,
            base_cost_per_hour: 0,
            max_machine_hours: 0
        }
    ],
    modifiers: {
        region: "",
        is_far_north: false,
        winter_from: "11-01",
        winter_to: "03-31",
        stops: 0,
        stop_duration_min: 0,
        travel_time_min: 0,
        region_multiplier: 1.0,
        winter_surcharge: 0.15,
        far_north_surcharge: 0.40,
        service_start: "05:00",
        service_end: "01:00",
        driver_shift_hours: 8
    },
    schedule: {
        date: new Date().toISOString().slice(0, 10),
        date_from: new Date().toISOString().slice(0, 10),
        date_to: new Date().toISOString().slice(0, 10),
        day_of_week: new Date().toLocaleDateString("ru-RU", { weekday: "long" }),
        daily_coefficient: 1.0,
        hourly_flow: {}
    }
})

// Инициализация ручного ввода
const initEmptyInput = () => {
    error.value = null
    outputData.value = null
    inputData.value = createEmptyInput()
    selectedDate.value = inputData.value.schedule.date
}

// Загрузка/расчёт output.json — вызывается из goToResults
const loadOutputData = async () => {
    if (!inputData.value) {
        error.value = "Нет данных для расчёта"
        outputData.value = null
        return
    }

    isLoadingOutput.value = true
    error.value = null

    try {
        outputData.value = await calculateReport(inputData.value)
    } catch (e) {
        error.value = e.message
        outputData.value = null
    } finally {
        isLoadingOutput.value = false
    }
}

// Переход к результатам.
// data — снимок { fleet, modifiers, schedule } из DataSetup.buildInput().
const goToResults = async (data) => {
    if (data) {
        inputData.value = data
        if (!selectedDate.value && data.schedule?.date)
            selectedDate.value = data.schedule.date
    }

    // Подставляем актуальный день недели из выбранной даты
    if (inputData.value?.schedule && dayOfWeekFromDate.value)
        inputData.value.schedule.day_of_week = dayOfWeekFromDate.value

    exportOnly.value = false
    await loadOutputData()

    // Переключаемся только если реально получили данные
    if (!error.value && outputData.value)
        currentPage.value = "results"
}

// Выгрузка в Excel: результат не показываем
const goToExport = async (data) => {
    if (data) {
        inputData.value = data
        if (!selectedDate.value && data.schedule?.date)
            selectedDate.value = data.schedule.date
    }

    if (inputData.value?.schedule && dayOfWeekFromDate.value)
        inputData.value.schedule.day_of_week = dayOfWeekFromDate.value

    error.value = null
    isLoadingOutput.value = true

    try {
        const iso = selectedDate.value || inputData.value?.schedule?.date || "no-date"
        await downloadExcel(inputData.value, `transport_report_${iso}.xlsx`)
    } catch (e) {
        error.value = e.message
    } finally {
        isLoadingOutput.value = false
    }
}

// Возврат на настройки
const goToSetup = () => (currentPage.value = "setup")
</script>

<template>
    <div class="app-container">
        <div v-if="error" class="alert alert-danger m-4">{{ error }}</div>
        <DataSetup
            v-if="currentPage === 'setup'"
            :input-data="inputData"
            :is-loading="isLoadingInput"
            @create-empty="initEmptyInput"
            @calculate="goToResults"
            @export="goToExport"
            @update:date="(d) => (selectedDate = d)"
        />
        <ResultsPage
            v-else
            :output-data="outputData"
            :input-data="inputData"
            :is-loading="isLoadingOutput"
            :report-date="reportDate"
            :error="error"
            :export-only="exportOnly"
            @back="goToSetup"
        />
    </div>
</template>