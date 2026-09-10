// Слой общения с Go-бэкендом (127.0.0.1:8080).
//
// Все запросы идут не напрямую на :8080, а на относительный /api — его
// перехватывает dev-прокси Vite (см. vite.config.js) и переправляет на бэк.
// Причина не косметическая: POST с Content-Type application/json браузер
// считает "непростым" и перед ним шлёт preflight OPTIONS. В бэке нет ни
// CORS-заголовков, ни обработки OPTIONS (хендлеры принимают только POST и
// на OPTIONS ответят 405 без Access-Control-Allow-*), поэтому preflight
// провалился бы и настоящий запрос вообще не ушёл. Через прокси вызов
// становится same-origin — preflight не нужен, CORS отпадает.
//
// Формат ответа бэк выбирает по query-параметру: ?type=json | ?type=excel.
// Сам бэкенд мы НЕ трогаем: toBackendPayload приводит данные фронта к его
// контракту (module.Root в Go), а validateInput не даёт уйти запросу, на
// котором бэк падает (в его ветках ошибок забыт return — детали ниже).
const BASE = "/api"

// Посчитать и получить результат как JSON (форма совпадает с output.json бэка).
// Тут всё прямолинейно: адаптируем вход, POST-им, при не-2xx достаём текст
// ошибки от бэка ({ "error": ... }) и кидаем его наверх.
export async function calculateReport(input) 
{
    const r = await fetch(`${BASE}?type=json`, postJSON(toBackendPayload(input)))
    if (!r.ok) 
        throw new Error(`Бэкенд ${r.status}: ${await errText(r)}`)
    return r.json()
}

// Скачать отчёт в Excel — тут два неочевидных момента.
//
// 1) Бэк не отдаёт файл в ответ на POST. Он генерит .xlsx в ./reports и
//    возвращает JSON { file_url }. Значит сам файл нужно забрать вторым
//    запросом (GET).
//
// 2) Возвращённый file_url использовать как есть НЕЛЬЗЯ — он битый по двум
//    причинам:
//      а) статика в main.go примонтирована на /reports/, а URL бэк строит
//         как .../files/... . GET на /files/... уйдёт не в файловый сервер,
//         а в catch-all "/" -> OutputTypesRouter, где без ?type зовётся
//         calculateJSON, а он на GET отвечает 405. Т.е. по родному URL файл
//         не отдаётся в принципе.
//      б) имя файла бэк делает из time.Now().String() — это
//         "2025-09-10 17:59:12.13 +0000 UTC m=+0.0..." с пробелами,
//         двоеточиями и плюсом, что в URL-путь класть нельзя.
//
//    Поэтому путь из file_url выбрасываем целиком: берём только имя
//    (split("/").pop() — внутри имени слэшей нет), делаем его безопасным
//    через encodeURIComponent и собираем ПРАВИЛЬНЫЙ /reports/<имя>. Go на
//    той стороне раскодирует %20/%3A обратно в реальное имя на диске
//    (в Linux пробелы и двоеточия в именах легальны) и файл найдёт.
//    Второй GET тоже идёт через /api-прокси намеренно: напрямую с :8080
//    браузер не дал бы прочитать cross-origin blob без CORS-заголовков,
//    которых у FileServer нет.
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

// Переводит внутреннюю модель фронта в контракт бэка (module.Root в Go).
// Тут собраны все расхождения схем; каждое прокомментировано у своей строки.
// validateInput зовём первым — если данные кривые, до формирования тела
// дело не дойдёт (почему это важно — см. саму функцию).
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
            // У бэка одна вместимость — capacity; ею оптимизатор закрывает спрос
            // (served += capacity * trips). У фронта две цифры (всего/сидячих),
            // берём полную провозную (seats_total). seats_seated бэку сопоставить
            // не с чем — не отправляем. Если нужно считать по сидячим — заменить
            // тут на v.seats_seated (это уже продуктовое решение, не баг).
            capacity: v.seats_total,
            // Фронт хранит рубли/час; C++ сам переводит в копейки (*100, llround).
            base_cost_per_hour: v.base_cost_per_hour,
        })),
        modifiers: {
            // region/season бэк принимает СТРОКАМИ, но в текущем билде моста никуда
            // их не конвертирует: region_multiplier/season_multiplier остаются 1.0.
            // Т.е. что бы мы ни прислали — на стоимость это не влияет (коэф. региона
            // и сезонная надбавка с фронт-чипа в расчёт бэка НЕ попадают). Но прислать
            // ОБЯЗАНЫ: C++ делает input.at("region")/.at("season") и без ключа кидает
            // исключение -> optimizer вернёт null -> паника на бэке. || "1" гарантирует
            // непустую строку (и повторяет дефолт самого сервера).
            region: m.region || "1",
            season: seasonName(m, s.date),
            // Бэк ждёт часы (float) и внутри сам делает минуты (llround(hours*60)).
            // Фронт считает цикл в МИНУТАХ. Если прислать минуты как есть, бэк примет
            // 60 минут за 60 часов и число рейсов улетит в бессмыслицу -> делим на 60.
            route_cycle_hours: cycle(m) / 60,
        },
        schedule: {
            day_of_week: s.day_of_week,
            daily_coefficient: s.daily_coefficient,
            // Бэк ждёт одно число (спрос) на слот; фронт хранит { total, seated }.
            hourly_flow: flattenFlow(s.hourly_flow),
        },
    }
}

// Проверка входа ПЕРЕД отправкой. Существует из-за конкретного бага бэка:
// в json.go/xlsx.go после writeJSONERresponse(...ошибка...) забыт return,
// поэтому на невалидных данных хендлер идёт дальше. В xlsx-ветке result после
// ошибки = nil, а следом generateXLSX(*result) разыменовывает nil -> паника,
// net/http гасит её и рвёт соединение: клиент получает пустой ответ без причины.
// В json-ветке до разыменования не доходит, но случается второй
// writeJSONERresponse(...StatusOK, nil) -> двойной WriteHeader и склеенное тело.
// Внятного 400 в обоих случаях нет. Поэтому зеркалим серверные проверки здесь
// и кидаем конкретное сообщение — пользователь видит причину, а не оборванный
// запрос. Правила ниже — копия ValidateJSON (Go) с оглядкой на ParseHour (C++).
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
    // Цикл = 0 бывает при <2 остановок. C++-мост требует route_cycle_hours > 0.
    // Отсекаем здесь, а не полагаемся на Go: он молча подменяет <=0 на 1 и
    // замаскировал бы реальную проблему (посчитал бы по выдуманному циклу).
    if (cycle(m) <= 0) 
        throw new Error("Цикл маршрута = 0 (нужно ≥ 2 остановок)")

    const s = input.schedule || {}
    if (!String(s.day_of_week || "").trim()) 
        throw new Error("Не задан день недели (выбери дату)")
    if (!(s.daily_coefficient > 0)) 
        throw new Error("Коэффициент дня должен быть > 0")

    const flow = s.hourly_flow || {}
    const slots = Object.keys(flow)
    // Пустой поток бэк отвергает ("hourly_flow не может быть пустым"). В ручном
    // режиме поля для потока в UI нет, он остаётся {} -> нужен загруженный data.json.
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

// Слот часа вида "H-H+1". Тонкость: слот проверяют ДВА слоя бэка по-разному,
// и берём их пересечение.
//   Go  (isValidTimeSlot): end == start+1, start > 0, end <= 24, + спецкейс "24-1".
//   C++ (ParseHour): читает только стартовый час и требует 0 <= hour < 24.
// Итог: безопасны start 1..23, end = start+1.
//   - "24-1" проходит Go (спецкейс), но C++ на часе 24 бросает исключение ->
//     optimizer вернёт null -> та самая паника. Поэтому "24-1" ОТСЕКАЕМ.
//   - "0-1" C++ принял бы, но Go режет (start > 0) — значит тоже вне пересечения.
function isValidSlot(slot) 
{
    const [a, b] = String(slot).split("-")
    const start = Number(a), end = Number(b)
    return Number.isInteger(start) && Number.isInteger(end) && end === start + 1 && start >= 1 && start <= 23
}

// Сплющивает поток к тому, что ждёт бэк: одно неотрицательное целое (спрос)
// на слот. Терпимо к обеим формам значения — объект { total, seated } (из
// data.json) и голое число — на случай, если поток придёт уже плоским.
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

// Дата в зимнем интервале? md — "MM-DD" (берём с 5-го символа ISO "YYYY-MM-DD").
// Интервал может перехлёстывать Новый год (from > to, напр. 11-01…03-31) — тогда
// "внутри" означает md >= from ИЛИ md <= to; иначе обычное И между границами.
function isWinter(iso, m) 
{
    if (!iso || !m.winter_from || !m.winter_to) 
        return false
    const md = iso.slice(5), f = m.winter_from, t = m.winter_to
    return f <= t ? (md >= f && md <= t) : (md >= f || md <= t)
}

// Название сезона по тем же правилам, что чип в Settings.vue: Крайний Север
// приоритетнее зимы (при нём даты вообще игнорируются); иначе зима по дате;
// иначе лето. Уходит в modifiers.season строкой (на расчёт бэка не влияет).
function seasonName(m, iso) 
{
    if (m.is_far_north) 
        return "Крайний Север"
    return isWinter(iso, m) ? "Зима" : "Лето"
}

// Длительность цикла В МИНУТАХ = (остановок−1)·движение + (остановок−2)·стоянка.
// Первая посадка и стоянка в конечной не считаются; при <2 остановок цикл = 0.
// ВАЖНО: это минуты — в payload делится на 60, т.к. бэк ждёт часы (см. выше).
function cycle(m) 
{
    const s = +m.stops || 0, sd = +m.stop_duration_min || 0, t = +m.travel_time_min || 0
    return s < 2 ? 0 : (s - 1) * t + (s - 2) * sd
}

// Обёртка опций fetch для POST c JSON-телом (чтобы не дублировать в двух местах).
function postJSON(body) 
{
    return {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(body),
    }
}

// Скачивание blob через временную ссылку: создаём <a download>, программно
// кликаем, затем убираем элемент и ОБЯЗАТЕЛЬНО revokeObjectURL — иначе objectURL
// держит blob в памяти до конца жизни вкладки (утечка).
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

// Достаёт текст ошибки из ответа. Тело Response — поток, читается РОВНО один раз.
// Сперва пробуем JSON (у бэка формат { "error": ... }); если это не JSON —
// падаем в .text(). Но к тому моменту тело уже "съедено" неудачным .json(),
// поэтому JSON-попытку делаем на r.clone(), а оригинал остаётся нетронутым для
// текстового фолбэка. Без clone фолбэк упал бы с "body already read".
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