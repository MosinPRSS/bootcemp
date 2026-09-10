<script setup>
defineProps({
  vehicles: { type: Array, default: () => [] }
})
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
            <input
              v-model.number="vehicle.max_available"
              type="number"
              class="form-control"
            />
          </td>
          <td>
            <input
              v-model.number="vehicle.capacity"
              type="number"
              class="form-control"
            />
          </td>
          <td>
            <input
              v-model.number="vehicle.base_cost_per_hour"
              type="number"
              class="form-control"
            />
          </td>
        </tr>
      </tbody>
    </table>
  </div>
</template>

<style scoped>
.table-container {
  margin-bottom: 30px;
}

.table {
  color: var(--text);
}

.table input,
.table select {
  background-color: rgba(255, 255, 255, 0.05);
  border: 1px solid rgba(255, 255, 255, 0.1);
  color: #fff;
}

.table input:focus,
.table select:focus {
  background-color: rgba(255, 255, 255, 0.1);
  border-color: var(--accent);
  color: #fff;
  box-shadow: 0 0 0 0.25rem rgba(170, 59, 255, 0.25);
}
</style>