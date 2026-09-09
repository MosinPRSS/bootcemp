<template>
<!-- Выбор даты, по дате определяется сезон -->
<label for="date-input">Выберите дату:
    <input 
      id="date-input" 
      type="date" 
      v-model="selectedDate" 
    />
</label>

 <!-- Крайний Север - флажок -->
<div class="form-check">
    <label for="far-north" class="form-check-label" >Регион - крайний север
        <input class="form-check-input" id="far-north" type="checkbox" v-model="isFarNorth"/>
    </label>
</div>

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
#date-input{
    width: min-content;
}

.form-check{
    width: 35vh;
}
</style>