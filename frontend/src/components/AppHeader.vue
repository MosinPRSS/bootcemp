<script setup>
import { computed } from "vue"

const props = defineProps({
    // Заголовок
    title: {
        type: String,
        default: ""
    },
    // День недели
    dayOfWeek: {
        type: String,
        default: ""
    },
    // Коэффициент отношения дня к среднему
    knn: {
        type: Number,
        default: null
    },
    // Дата
    date: {
        type: [String, Date],
        default: null
    }
})

// Форматирование даты
const formattedDate = computed(() => {
    if (!props.date)
    {
        return new Date().toLocaleDateString("ru-RU", {
            day: "numeric",
            month: "long",
            year: "numeric"
        })
    }

    if (typeof props.date === "string")
        return props.date

    return props.date.toLocaleDateString("ru-RU", {
        day: "numeric",
        month: "long",
        year: "numeric"
    })
})
</script>

<template>
    <header class="result-header">
        <div class="header-left">
            <h1 class="header-title">{{ title }}</h1>
            <p v-if="dayOfWeek" class="header-day">
                {{ dayOfWeek }}
                <span v-if="knn" class="header-knn">Коэффициент отношения дня к среднему: {{ knn }}</span>
            </p>
        </div>
        <div class="header-right">
            <p class="header-date">{{ formattedDate }}</p>
        </div>
    </header>
</template>

<style scoped>
    
</style>