<template>
<!-- Выбор даты, по дате определяется сезон -->
<label for="date-input">Выберите дату:</label>
    <input 
      id="date-input" 
      type="date" 
      v-model="selectedDate" 
    />

 <!-- Крайний Север - флажок -->
<label for="far-north">Регион - крайний север</label>
    <input id="far-north" type="checkbox" v-model="isFarNorth"/>
</template>

<script setup>
import { ref, computed } from 'vue'

const selectedDate = ref('');

// Функция определяет, принадлежит ли указанная дата к зимнему периоду
const currentSeason = computed(() => {
    if (selectedDate.value) return ''
    const date = new Date(selectedDate.value);
    const month = date.getMonth();

    if (month >=10 || month <= 2) {
        return isWinterSeason = true;
    }
    else return isWinterSeason = false;
})

const isFarNorth = ref(false);
</script>

<style scoped>

</style>