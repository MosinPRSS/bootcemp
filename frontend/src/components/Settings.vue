<template>
<h2 class="header-title">{{ title }}</h2>
<!-- Первое меню - выбор типа расчета -->
 <label for="form-select-sm">Выберите тип расчета:
    <select class="form-select-sm" v-model="calcType">
        <option value="" disabled>Тип расчета не выбран</option>
        <option value="hours">Расчет по часам</option>
        <option value="weekdays">Расчет по дням недели</option>
    </select>
</label>

<!-- Второе меню - выбор единиц -->
<!-- Выбор часа -->
<div v-if="calcType === 'hours'">
    <label for="hourSelection">Выберите час:
        <select id="hourSelection" class="form-select-sm" v-if="calcType === 'hours'" v-model="selectedHour" @change="hourChange">
            <option value="" disabled>Выберите час...</option>
            <option v-for="hourOption in hourOptions" :key="hourOption.value" :value="hourOption.value">{{ hourOption.label }}</option>
        </select>
    </label>
</div>
    
<!-- Выбор дня недели -->
 <div v-if="calcType === 'weekdays'">
    <label for="hourSelection">Выберите день недели:
        <select class="form-select-sm" v-if="calcType === 'weekdays'" v-model="selectedWeekday" @change="weekdayChange">
            <option value="" disabled>Выберите день недели...</option>
            <option v-for="weekdayOption in weekdayOptions" :key="weekdayOption.value" :value="weekdayOption.value">{{ weekdayOption.label }}</option>
        </select>
    </label>
</div>

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

<router-link to="/about" class="btn btn-primary">
  Рассчитать
</router-link>


</template>

<script setup>
import { ref, computed } from 'vue'

// Хранение состояния
const calcType = ref('');
const selectedHour = ref(null);
const selectedweekDay = ref(null);

// Входные данные
const props = defineProps({
    // Заголовок
    title: {
        type: String,
        default: ""
    },
    hourData: {
        type: Array,
        default: () => [],
    },
    weekdayData: {
        type: Array,
        default: () => []
    }
}) 

// Изменение типа расчета
function calcTypeChange(){
    selectedHour.value = null;
    selectedWeekday.value = null;
}

// обработка изменения выбранного часа
function hourChange() {
    if (selectedHour.value !== null) {
         const hourInfo = hourData.find(item => item.hour === selectedHour.value);
    }
}
// Обработка изменения выбранного дня
function weekdayChange() {
    if (selectedWeekday.value !== null) {
         const hourInfo = weekdayData.find(item => item.hour === selectedWeekday.value);
}
}

// Постоянные коэффициенты загруженности по дням недели
const WEEKDAY_COEFFICIENTS = {
  'ПН': 1.08,
  'ВТ': 1.02,
  'СР': 1.05,
  'ЧТ': 1.08,
  'ПТ': 1.1,
  'СБ': 0.89,
  'ВС': 0.79
};

const hourOptions = computed(() => {
    const options = [];
    // Генерация часов от 5 до 23 включительно
    for (let i = 5; i <= 23; i++) {
        options.push({
        value: i,        // Хранение первого числа (5, 6, 7, ...)
        label: `${i}-${i + 1}`  // Отображение в формате "5-6", "6-7", и т.д.
        });
    }
    // Добавление последнего варианта: 24-01
    options.push({
        value: 24,       // Хранение числа 24
        label: '24-01'   // Отображение как "24-01"
    });
    return options;
});

const weekdayOptions = [
  { value: 'ПН', label: 'Понедельник' },
  { value: 'ВТ', label: 'Вторник' },
  { value: 'СР', label: 'Среда' },
  { value: 'ЧТ', label: 'Четверг' },
  { value: 'ПТ', label: 'Пятница' },
  { value: 'СБ', label: 'Суббота' },
  { value: 'ВС', label: 'Воскресенье' }
];

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