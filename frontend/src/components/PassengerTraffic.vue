<template>
<!-- Первое меню - выбор типа расчета -->
<select v-model="calcType">
    <option value="" disabled>Выберите тип расчета</option>
    <option value="hours">Расчет по часам</option>
    <option value="weekdays">Расчет по дням недели</option>
</select>

<!-- Второе меню - выбор единиц -->
<!-- Выбор часа -->
<select v-if="calcType === 'hours'" v-model="selectedHour" @change="hourChange">
    <option value="" disabled>Выберите час</option>
    <option v-for="hourOption in hourOptions" :key="hourOption.value" :value="hourOption.value">{{ hourOption.label }}</option>
</select>

<!-- Выбор дня недели -->
<select v-if="calcType === 'weekdays'" v-model="selectedWeekday" @change="weekdayChange">
    <option value="" disabled>Выберите день недели</option>
    <option v-for="weekdayOption in weekdayOptions" :key="weekdayOption.value" :value="weekdayOption.value">{{ weekdayOption.label }}</option>
</select>
</template>


<script setup>
import { ref, computed, watch } from 'vue';

// Хранение состояния
const calcType = ref('');
const selectedHour = ref(null);
const selectedweekDay = ref(null);

// Входные данные
const props = defineProps({
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
    // Генерирация часов от 5 до 23 включительно
    for (let i = 5; i <= 23; i++) {
        options.push({
        value: i,        // Хранение первое число (5, 6, 7, ...)
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
</script>


<style scoped>
    
</style>