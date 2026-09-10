<script setup>
import { computed } from "vue"

const props = defineProps({
    // Заголовок метрики
    title: {
        type: String,
        required: true
    },
    // Значение (число или строка)
    value: {
        type: [Number, String],
        required: true
    },
    // Единица измерения (₽, шт., тыс. пас.)
    unit: {
        type: String,
        default: ""
    },
    // Вариант окраски: "primary", "success", "danger", "warning", "info"
    variant: {
        type: String,
        default: "primary" // первичный - базовое значение
    },
    // Иконка
    icon: {
        type: String, 
        default: ""
    },
    // Является ли метрика особым дефицитом (для выделения)
    isDeficit: {
        type: Boolean,
        default: false
    }
})

// Форматирование значения с единицей измерения
const formattedValue = computed(() => {
    if (props.value === null || props.value === undefined)
        return "-"

    if (typeof props.value === "number")
    {
        const n = props.value.toLocaleString("ru-RU")
        return props.unit ? `${n} ${props.unit}` : n
    }

    return props.value
})

// Функция - если дефицит и его значение больше 0
const deficitFunc = computed(() => {
    return (props.isDeficit && props.value > 0) ? "metric-card-deficit" : ""
})
</script>

<template>
    <div :class="['card', 'text-center', 'metric-card-wrapper', `metric-card-${variant}`, deficitFunc]">
        <div class="card-body">
            <div class="metric-icon">
                <slot name="icon">
                    <span v-if="icon" v-html="icon"></span>
                </slot>
            </div>
            <div class="metric-value">{{ formattedValue }}</div>
            <div class="metric-title">{{ title }}</div>
        </div>
    </div>
</template>

<style scoped>
    
</style>