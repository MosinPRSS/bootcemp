<script setup>
import { ref, computed } from "vue"
import AppHeader from "../components/AppHeader.vue"
import Table from "../tables/Table.vue"
import Settings from "../components/Settings.vue"

const props = defineProps({
    inputData: { type: Object, default: null },
    isLoading: { type: Boolean, default: false }
})

// loadFleet - загрузить fleet.json
// loadData - загрузить data.json
// createEmpty - создать пустой шаблон для ручного ввода
// calculate - перейти к результатам
// export - экспорт
// update:date - пользователь поменял дату
const emit = defineEmits(["loadFleet", "loadData", "createEmpty", "calculate", "export", "update:date"])

const tab = ref("fleet")

// Начальная дата для шапки (если input.json загружен)
const initialDate = computed(() => props.inputData?.schedule?.date || "")
</script>

<template>
    <AppHeader title="Расчёт маршрута" :date="initialDate" />
    <ul class="nav nav-tabs justify-content-center mb-3">
        <li class="nav-item"><a class="nav-link" :class="{active: tab==='fleet'}" @click="tab='fleet'">Автопарк</a></li>
        <li class="nav-item"><a class="nav-link" :class="{active: tab==='flow'}"  @click="tab='flow'">Пассажиропоток</a></li>
    </ul>
    <div class="text-center mb-4 d-flex justify-content-center gap-2">
        <button v-if="tab==='fleet'" class="btn btn-primary" @click="emit('loadFleet')" :disabled="isLoading">Загрузить автопарк (fleet.json)</button>
        <button v-if="tab==='flow'"  class="btn btn-primary" @click="emit('loadData')"  :disabled="isLoading">Загрузить пассажиропоток (data.json)</button>
        <button class="btn btn-outline-primary" @click="emit('createEmpty')" :disabled="isLoading">Заполнить вручную</button>
    </div>
    <template v-if="inputData">
        <Table v-if="tab==='fleet'" :vehicles="inputData.fleet" />
        <Settings
            v-if="tab==='flow'"
            title="Настройки"
            :modifiers="inputData.modifiers"
            :schedule="inputData.schedule"
            :initial-date="inputData.schedule?.date || ''"
            @update:date="(d) => emit('update:date', d)"
            @calculate="emit('calculate')"
            @export="emit('export')"
        />
    </template>
    <div v-else class="alert alert-info text-center">
        Загрузите автопарк и пассажиропоток или заполните вручную.
    </div>
</template>