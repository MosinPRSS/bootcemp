<script setup>
// Таблица автопарка.
// Работает и с загруженным input.json, и с пустым шаблоном из createEmptyInput().
const props = defineProps({
    vehicles: { type: Array, default: () => [] }
})

// Добавить пустую строку в конец массива
const addRow = () => {
    props.vehicles.push({
        model: "",
        class: "Малый",
        max_available: 0,
        capacity: 0,
        base_cost_per_hour: 0
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
                    <th>Вместимость</th>
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
                        <input v-model.number="vehicle.max_available" type="number" class="form-control" />
                    </td>
                    <td>
                        <input v-model.number="vehicle.capacity" type="number" class="form-control" />
                    </td>
                    <td>
                        <input v-model.number="vehicle.base_cost_per_hour" type="number" class="form-control" />
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
        <div class="text-start mt-2">
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

.table th:nth-child(2),
.table td:nth-child(2) {
    min-width: 160px;
    white-space: nowrap;
}

.table input,
.table select {
    background-color: rgba(255, 255, 255, 0.05);
    border: 1px solid rgba(255, 255, 255, 0.1);
    color: #fff;
    min-width: 190px;
    padding-right: 1.5rem !important;
    background-position: right 0.5rem center; 
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
</style>