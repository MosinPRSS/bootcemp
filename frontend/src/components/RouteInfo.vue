<script setup>
defineProps({
    stops: { // Количество остановок на маршруте
        type: Number,
        default: 0
    },
    stopDuration: { // Время стоянки на каждой остановке в минутах
        type: Number,
        default: 0
    },
    travelTime: { // Время поездки
        type: Number, 
        default: 0 
    },
    cycleDuration: { // Время полного круга маршрута без учёта остановок в минутах
        type: Number,
        default: 0
    },
    routes: { // Массив маршрутов
        type: Array,
        default: () => []
    }
})
</script>

<template>
    <div class="route-info">
        <h3 class="section-title">Информация о маршруте</h3>
        <div class="route-params">
            <div class="param-item">
                <span class="param-label">Остановок:</span>
                <span class="param-value">{{ stops }}</span>
            </div>
            <div class="param-item">
                <span class="param-label">Время на остановку:</span>
                <span class="param-value">{{ stopDuration }} мин.</span>
            </div>
            <div class="param-item">
                <span class="param-label">Движение между остановками:</span>
                <span class="param-value">{{ travelTime }} мин.</span>
            </div>
            <div class="param-item">
                <span class="param-label">Длительность цикла:</span>
                <span class="param-value">{{ cycleDuration }} мин.</span>
            </div>
        </div>
        <div v-if="routes.length" class="route-list">
            <h4>Список маршрутов</h4>
            <ul class="list-unstyled">
                <li v-for="(route, idx) in routes" :key="idx">
                    <span class="route-name">{{ route.name || `Маршрут ${idx+1}` }}</span>
                    <span v-if="route.duration" class="route-duration">({{ route.duration }} мин.)</span>
                </li>
            </ul>
        </div>
    </div>
</template>

<style scoped>

</style>