<script setup>
import AppHeader from "../components/AppHeader.vue"
import Table from "../tables/Table.vue"
import Settings from "../components/Settings.vue"

const props = defineProps({
    inputData: { type: Object, default: null },
    isLoading: { type: Boolean, default: false }
})

const emit = defineEmits(["loadInput", "calculate"])
</script>

<template>
    <AppHeader title="Входные данные" />
    <div class="text-center mb-4">
        <button
            class="btn btn-primary"
            @click="emit('loadInput')"
            :disabled="isLoading"
        >
        {{ isLoading ? "Загрузка..." : "Загрузить input.json" }}
        </button>
    </div>
    <template v-if="inputData">
        <Table :vehicles="inputData.fleet" />
        <Settings
            title="Настройки"
            :modifiers="inputData.modifiers"
            :schedule="inputData.schedule"
            @calculate="emit('calculate')"
        />
    </template>
    <div v-else class="alert alert-info text-center">
        Нажмите кнопку выше, чтобы загрузить входные данные.
    </div>
</template>