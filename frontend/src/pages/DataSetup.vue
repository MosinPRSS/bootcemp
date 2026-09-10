<script setup>
import AppHeader from "../components/AppHeader.vue"
import Table from "../tables/Table.vue"
import Settings from "../components/Settings.vue"

const props = defineProps({
    inputData: { type: Object, default: null },
    isLoading: { type: Boolean, default: false }
})

// loadInput - загрузить input.json
// createEmpty - создать пустой шаблон для ручного ввода
// calculate - перейти к результатам
// update:date - пользователь поменял дату
const emit = defineEmits(["loadInput", "createEmpty", "calculate", "update:date"])

// Начальная дата для шапки (если input.json загружен)
const initialDate = props.inputData?.schedule?.date || ""
</script>

<template>
    <AppHeader title="Входные данные" :date="initialDate" />
    <div class="text-center mb-4 d-flex justify-content-center gap-2">
        <button
            class="btn btn-primary"
            @click="emit('loadInput')"
            :disabled="isLoading"
        >
            {{ isLoading ? "Загрузка..." : "Загрузить input.json" }}
        </button>
        <button
            class="btn btn-outline-primary"
            @click="emit('createEmpty')"
            :disabled="isLoading"
        >
            Заполнить вручную
        </button>
    </div>
    <template v-if="inputData">
        <Table :vehicles="inputData.fleet" />
        <Settings
            title="Настройки"
            :modifiers="inputData.modifiers"
            :schedule="inputData.schedule"
            :initial-date="inputData.schedule?.date || ''"
            @update:date="(d) => emit('update:date', d)"
            @calculate="emit('calculate')"
        />
    </template>
    <div v-else class="alert alert-info text-center">
        Нажмите «Загрузить input.json» или «Заполнить вручную», чтобы начать.
    </div>
</template>