const BASE = "/api"


export async function calculateReport(input) 
{
    const r = await fetch(`${BASE}?type=json`, postJSON(toBackendPayload(input)))
    if (!r.ok) 
        throw new Error(`Бэкенд ${r.status}: ${await errText(r)}`)
    return r.json()
}

export async function downloadExcel(input, fallbackName = "transport_report.xlsx") 
{
    const r = await fetch(`${BASE}?type=excel`, postJSON(toBackendPayload(input)))
    if (!r.ok) 
        throw new Error(`Бэкенд ${r.status}: ${await errText(r)}`)

    const { file_url } = await r.json()
    if (!file_url) 
        throw new Error("бэкенд не вернул file_url")

    const name = file_url.split("/").pop()
    const fileResp = await fetch(`${BASE}/reports/${encodeURIComponent(name)}`)
    if (!fileResp.ok) 
        throw new Error(`Не удалось скачать файл: ${fileResp.status}`)

    saveBlob(await fileResp.blob(), fallbackName)
}


function toBackendPayload(input) 
{
    validateInput(input)

    const m = input.modifiers
    const s = input.schedule

    return {
        fleet: input.fleet.map((v) => ({
            model: v.model,
            class: v.class,
            max_available: v.max_available,
            capacity: v.seats_total,
            seats: v.seats_seated,
            base_cost_per_hour: v.base_cost_per_hour,
        })),
        modifiers: {
            region: m.region || "1",
            season: seasonName(m, s.date),
            route_cycle_hours: cycle(m) / 60,
        },
        schedule: {
            day_of_week: s.day_of_week,
            daily_coefficient: s.daily_coefficient,
            hourly_flow: flattenFlow(s.hourly_flow),
        },
    }
}


function validateInput(input) 
{
    if (!input) 
        throw new Error("Нет данных для расчёта")

    const fleet = input.fleet || []
    if (fleet.length === 0) 
        throw new Error("Автопарк пуст")
    fleet.forEach((v, i) => {
        const n = i + 1
        // Бэк требует непустые model/class и строго положительные числа.
        // Пустая модель или 0 в любом из полей на бэке -> ошибка -> ветка без return.
        if (!String(v.model || "").trim()) 
            throw new Error(`Автопарк, строка ${n}: не заполнена модель`)
        if (!String(v.class || "").trim()) 
            throw new Error(`Автопарк, строка ${n}: не выбран класс`)
        if (!(v.max_available > 0)) 
            throw new Error(`Автопарк, строка ${n}: «Кол-во в автопарке» должно быть > 0`)
        // seats_total уедет как capacity, а C++ отдельно требует capacity > 0.
        if (!(v.seats_total > 0)) 
            throw new Error(`Автопарк, строка ${n}: «Всего мест» должно быть > 0`)
        if (!(v.base_cost_per_hour > 0))
            throw new Error(`Автопарк, строка ${n}: «Стоимость машино-часа» должна быть > 0`)
    })

    const m = input.modifiers || {}

    if (cycle(m) <= 0) 
        throw new Error("Цикл маршрута = 0 (нужно ≥ 2 остановок)")

    const s = input.schedule || {}
    if (!String(s.day_of_week || "").trim()) 
        throw new Error("Не задан день недели (выбери дату)")
    if (!(s.daily_coefficient > 0)) 
        throw new Error("Коэффициент дня должен быть > 0")

    const flow = s.hourly_flow || {}
    const slots = Object.keys(flow)

    if (slots.length === 0)
        throw new Error("Пассажиропоток по часам пуст — в ручном режиме он не задаётся, нужен data.json")
    for (const slot of slots) 
    {
        if (!isValidSlot(slot))
            throw new Error(`Некорректный интервал «${slot}» — допустимы часы вида 1-2 … 23-24`)
        const val = typeof flow[slot] === "number" ? flow[slot] : (flow[slot]?.total ?? 0)
        if (val < 0) 
            throw new Error(`Интервал «${slot}»: поток не может быть отрицательным`)
    }
}


function isValidSlot(slot) 
{
    const [a, b] = String(slot).split("-")
    const start = Number(a), end = Number(b)
    return Number.isInteger(start) && Number.isInteger(end) && end === start + 1 && start >= 1 && start <= 23
}

function flattenFlow(flow = {}) 
{
    const out = {}
    for (const [slot, v] of Object.entries(flow || {})) 
    {
        const n = typeof v === "number" ? v : (v?.total ?? 0)
        out[slot] = Math.max(0, Math.round(n))
    }
    return out
}

function isWinter(iso, m) 
{
    if (!iso || !m.winter_from || !m.winter_to) 
        return false
    const md = iso.slice(5), f = m.winter_from, t = m.winter_to
    return f <= t ? (md >= f && md <= t) : (md >= f || md <= t)
}

function seasonName(m, iso) 
{
    if (m.is_far_north) 
        return "Крайний Север"
    return isWinter(iso, m) ? "Зима" : "Лето"
}

function cycle(m) 
{
    const s = +m.stops || 0, sd = +m.stop_duration_min || 0, t = +m.travel_time_min || 0
    return s < 2 ? 0 : (s - 1) * t + (s - 2) * sd
}

// Обёртка опций fetch для POST c JSON-телом (чтобы не дублировать в двух местах).
function postJSON(body) 
{
    console.log(body)
    return {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(body),
    }
}


function saveBlob(blob, filename) 
{
    const url = URL.createObjectURL(blob)
    const a = document.createElement("a")
    a.href = url
    a.download = filename
    document.body.appendChild(a)
    a.click()
    a.remove()
    URL.revokeObjectURL(url)
}

async function errText(r) 
{
    try 
    {
        const j = await r.clone().json()
        return j.error || JSON.stringify(j)
    } 
    catch 
    {
        try 
        { 
            return (await r.text()).slice(0, 200) 
        } 
        catch 
        { 
            return "" 
        }
    }
}