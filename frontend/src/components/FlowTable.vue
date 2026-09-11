<script setup>
import { ref, watch } from "vue"

const props = defineProps({
    flow: {
        type: Object,
        required: true,
        default: () => ({})
    }
})

const rows = ref([])

function loadFromProps() {
    rows.value = Object.entries(props.flow || {})
        .map(([slot, v]) => ({
            slot,
            value: typeof v === "number" ? v : (v?.total ?? 0)
        }))
        .sort((a, b) => parseInt(a.slot) - parseInt(b.slot))
}

function syncToProps() {
    const out = {}
    for (const r of rows.value) {
        if (!r.slot) continue
        out[r.slot] = Math.max(0, Math.round(Number(r.value) || 0))
    }
    for (const k of Object.keys(props.flow))
        delete props.flow[k]
    Object.assign(props.flow, out)
}

watch(() => props.flow, loadFromProps, { immediate: true })

const isValidSlot = (s) => {
    const [a, b] = String(s).split("-")
    const start = Number(a), end = Number(b)
    return Number.isInteger(start) && Number.isInteger(end)
        && end === start + 1 && start >= 1 && start <= 23
}

const addRow = () => {
    const used = new Set(rows.value.map(r => r.slot))
    let h = 1
    while (used.has(`${h}-${h + 1}`) && h < 23) h++
    rows.value.push({ slot: `${h}-${h + 1}`, value: 0 })
    syncToProps()
}

const removeRow = (idx) => {
    rows.value.splice(idx, 1)
    syncToProps()
}

const slotClass = (slot) => isValidSlot(slot) ? "" : "is-invalid"
</script>

<template>
    <div class="flow-table">
        <h3 class="section-title">Пассажиропоток по часам</h3>
        <table class="table">
            <thead>
                <tr>
                    <th>Временной промежуток</th>
                    <th>Пассажиропоток, чел.</th>
                    <th style="width: 50px;"></th>
                </tr>
            </thead>
            <tbody>
                <tr v-for="(row, idx) in rows" :key="idx">
                    <td>
                        <input
                            v-model="row.slot"
                            type="text"
                            class="form-control"
                            :class="slotClass(row.slot)"
                            placeholder="6-7"
                            @change="syncToProps"
                        />
                    </td>
                    <td>
                        <input
                            v-model.number="row.value"
                            type="number"
                            min="0"
                            class="form-control"
                            @input="syncToProps"
                        />
                    </td>
                    <td class="text-center align-middle">
                        <button
                            class="btn btn-sm btn-outline-danger"
                            title="Удалить строку"
                            @click="removeRow(idx)"
                        >×</button>
                    </td>
                </tr>
            </tbody>
        </table>
        <div class="d-flex justify-content-between align-items-center mt-2">
            <button class="btn btn-sm btn-outline-success" @click="addRow">
                + Добавить интервал
            </button>
            <span class="table-hint">
                Формат: «6-7», «7-8», …, «23-24». Начальный час — от 1 до 23.
            </span>
        </div>
    </div>
</template>

<style scoped>
.flow-table { margin-bottom: 30px; }
.section-title {
    border-left: 4px solid var(--accent);
    padding-left: 15px;
    margin-bottom: 20px;
    color: #fff;
}
.flow-table .table { color: var(--text); }
.flow-table .table td { padding: 0.5rem; vertical-align: middle; }
.flow-table input {
    background-color: var(--code-bg);
    border: 1px solid var(--border);
    color: var(--text-h);
    min-height: 46px;
}
.flow-table input:focus {
    background-color: rgba(255, 255, 255, 0.1);
    border-color: var(--accent);
    color: #fff;
    box-shadow: 0 0 0 0.25rem rgba(170, 59, 255, 0.25);
}
.table-hint { color: var(--text); font-size: 13px; }
</style>