<script setup>
// Таблица автопарка.
// Работает и с загруженным input.json, и с пустым шаблоном из createEmptyInput().
import { watch } from "vue"

const props = defineProps({
    vehicles: { type: Array, default: () => [] }
})

// Максимум машино-часов на одно ТС (смена водителя) — больше 8 быть не может
const MAX_SHIFT_HOURS = 8

// Ограничить машино-часы диапазоном [0, 8]
const clampHours = (vehicle) => {
    let h = Number(vehicle.max_machine_hours)
    if (!Number.isFinite(h)) h = 0
    vehicle.max_machine_hours = Math.min(MAX_SHIFT_HOURS, Math.max(0, h))
}

// Прогнать ограничение по всему автопарку (в т.ч. после загрузки fleet.json)
const clampAll = () => props.vehicles.forEach(clampHours)
watch(() => props.vehicles, clampAll, { immediate: true })

// Добавить пустую строку в конец массива
const addRow = () => {
    props.vehicles.push({
        model: "",
        class: "Малый",
        max_available: 0,
        seats_total: 0,
        seats_seated: 0,
        base_cost_per_hour: 0,
        max_machine_hours: MAX_SHIFT_HOURS
    })
}

// Удалить строку по индексу
const removeRow = (idx) => {
    props.vehicles.splice(idx, 1)
}
</script>

<template>
    <div class="table-container">
        <table class="table">
            <thead>
                <tr>
                    <th>Модель</th>
                    <th>Класс</th>
                    <th>Кол-во в автопарке</th>
                    <th>Всего мест</th>
                    <th>Сидячих</th>
                    <th>Стоимость машино-часа</th>                    
                    <th style="width: 50px;"></th>
                </tr>
            </thead>
            <tbody>
                <tr v-for="(vehicle, idx) in vehicles" :key="idx">
                    <td>
                        <input v-model="vehicle.model" type="text" class="form-control" />
                    </td>
                    <td>
                        <select v-model="vehicle.class" class="form-select">
                            <option value="Особо малый">Особо малый</option>
                            <option value="Малый">Малый</option>
                            <option value="Средний">Средний</option>
                            <option value="Большой">Большой</option>
                            <option value="Особо большой">Особо большой</option>
                        </select>
                    </td>
                    <td>
                        <input v-model.number="vehicle.max_available" type="number" min="0" class="form-control" />
                    </td>
                    <td>
                        <input v-model.number="vehicle.seats_total" type="number" min="0" class="form-control" />
                    </td>
                    <td>
                        <input v-model.number="vehicle.seats_seated" type="number" min="0" class="form-control" />
                    </td>
                    <td>
                        <input v-model.number="vehicle.base_cost_per_hour" type="number" min="0" class="form-control" />
                    </td>
                    <td class="text-center align-middle">
                        <button
                            class="btn btn-sm btn-outline-danger"
                            title="Удалить строку"
                            @click="removeRow(idx)"
                        >
                            ×
                        </button>
                    </td>
                </tr>
            </tbody>
        </table>
        <div class="d-flex justify-content-between align-items-center mt-2">
            <button class="btn btn-sm btn-outline-success" @click="addRow">
                + Добавить ТС
            </button>            
        </div>
    </div>
</template>

<style scoped>
.table-container {
    margin-bottom: 30px;
}

.table {
    color: var(--text);
}

.table td {
    padding: 0.5rem;
    vertical-align: middle;
}

.table th:nth-child(2),
.table td:nth-child(2) {
    min-width: 160px;
    white-space: nowrap;
}

.table input, .table select {
    background-color: var(--code-bg);
    border: 1px solid var(--border);
    color: var(--text-h);
    min-width: 0;
    font-size: 1rem;
    min-height: 46px;
    padding: 0.55rem 0.7rem;
    background-position: right 0.6rem center;
}

.table select {
    padding-right: 1.75rem !important;
}

.table td:first-child input {
    min-width: 190px;
}

.table input[type="number"] {
    min-width: 96px;
    max-width: 150px;
}

.table select option {
    background-color: #2a2f36;
    color: #ffffff;
}

.table input:focus,
.table select:focus {
    background-color: rgba(255, 255, 255, 0.1);
    border-color: var(--accent);
    color: #fff;
    box-shadow: 0 0 0 0.25rem rgba(170, 59, 255, 0.25);
}

.table-hint {
    color: var(--text);
    font-size: 13px;
}
</style>