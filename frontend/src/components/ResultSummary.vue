<script setup>
import MetricCard from '../cards/MetricCard.vue'

const props = defineProps({
    totalCost: { // Итоговая стоимость
        type: Number,
        required: true
    },
    totalBuses: { // Всего автобусов
        type: Number,
        required: true
    },
    servedPassengers: { // Обслужено пассажиров
        type: Number,
        required: true
    },
    deficit: { // Дефицит
        type: Number,
        required: true
    }
})
</script>

<template>
    <div class="result-summary">
        <div class="row g-4">
            <div class="col-12 col-sm-6 col-xl-3">
                <MetricCard title="Общая стоимость" :value="totalCost" unit="₽" variant="primary">
                    <template #icon>
                        <slot name="icon-totalCost">💰</slot>
                    </template>
                </MetricCard>
            </div>
            <div class="col-12 col-sm-6 col-xl-3">
                <MetricCard title="Всего назначено ТС" :value="totalBuses" unit="шт." variant="info">
                    <template #icon>
                        <slot name="icon-totalBuses">🚌</slot>
                    </template>
                </MetricCard>
            </div>
            <div class="col-12 col-sm-6 col-xl-3">
                <MetricCard title="Обслужено пассажиров" :value="servedPassengers" unit="пас." variant="info">
                    <template #icon>
                        <slot name="icon-servedPassengers">👥</slot>
                    </template>
                </MetricCard>
            </div>
            <div class="col-12 col-sm-6 col-xl-3">
                <MetricCard title="Дефицит" :value="deficit" unit="пас." :variant="deficit > 0 ? 'danger' : 'success'" :is-deficit="deficit > 0">
                    <template #icon>
                        <slot name="icon-deficit">
                            <span v-if="deficit > 0">⚠️</span>
                            <span v-else>✅</span>
                        </slot>
                    </template>
                    <template #footer>
                        <slot name="deficit-footer">
                            <small v-if="deficit > 0" class="text-danger">
                                Требуется докупить автобусы
                            </small>
                            <small v-else class="text-success">
                                Все пассажиры обслужены
                            </small>
                        </slot>
                    </template>
                </MetricCard>
            </div>
        </div>
    </div>
</template>

<style scoped>
.result-summary {
  margin-bottom: 2rem;
}   
</style>