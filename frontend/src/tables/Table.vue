<template>
<div class='table-container'>
    <!-- Таблица -->
    <table>
        <!-- Заголовок таблицы -->
        <thead>
            <tr>
                <th>ID</th>
                <th>Модель</th>
                <th>Класс</th>
                <th>Кол-во в автопарке</th>
                <th>Мин. общая пассажировместимость</th>
                <th>Макс. общая пассажировместимость</th>
                <th>Мин. посадочные места</th>
                <th>Макс. посадочные места</th>
                <th>Мин. стоимость машино-часа</th>
                <th>Макс. стоимость машино-часа</th>
            </tr>    
        </thead>
        <!-- Тело таблицы -->
        <tbody>
        <tr v-for="vehicle in vehicles" :key="vehicle.id">
          <!-- id статичный (пока не счетчик) -->
          <td>{{ vehicle.id }}</td>
          
          <td>
            <input v-model="vehicle.modelName" type="text" class="table-input" />
          </td>
          <td>
            <!-- Для класса машины выпадающий список -->
            <select v-model="vehicle.vehicleClass" class="table-select">
              <option value="Особо малый">Особо малый</option>
              <option value="Малый">Малый</option>
              <option value="Средний">Средний</option>
              <option value="Большой">Большой</option>
            </select>
          </td>
          <td>
            <input v-model="vehicle.numberInFleet" type="number" class="table-input" />
          </td>
          <td>
            <input v-model="vehicle.passengerCapacityOverallMin" type="number" class="table-input" />
          </td>
          <td>
            <input v-model="vehicle.passengerCapacityOverallMax" type="number" class="table-input" />
          </td>
          <td>
            <input v-model="vehicle.passengerCapacitySeatsMin" type="number" class="table-input" />
          </td>
          <td>
            <input v-model="vehicle.passengerCapacitySeatsMax" type="number" class="table-input" />
          </td>
          <td>
            <input v-model="vehicle.carHourCostMin" type="number" class="table-input" />
          </td>
          <td>
            <input v-model="vehicle.carHourCostMax" type="number" class="table-input" />
          </td>
        </tr>
      </tbody>
            
    </table>
</div>
</template>

<script setup>
import { ref } from 'vue'
// Данные таблицы (временный вариант)
const vehicles = ref([
    {id: 1, modelName: 'ПАЗ-32053 / 3204 «Вектор»', vehicleClass: 'Малый', numberInFleet: '30', passengerCapacityOverallMin: 36, passengerCapacityOverallMax: 50, passengerCapacitySeatsMin: 22, passengerCapacitySeatsMax: 30, carHourCostMin: 1800, carHourCostMax: 2400}
]) 
</script>

<style scoped>

</style>