<script setup>
import { ref } from "vue"
import ResultsPage from "./pages/ResultsPage.vue"
import DataSetup from "./pages/DataSetup.vue"

const currentPage = ref("setup")
const inputData = ref(null)
const outputData = ref(null)
const isLoadingInput = ref(false)
const isLoadingOutput = ref(false)
const error = ref(null)

const loadInputData = async () => {
  isLoadingInput.value = false
  error.value = null

  try
  {
    const response = await fetch("/input.json")
    if (!response.ok)
      throw new Error("Ошибка загрузки input.json")
    inputData.value = await response.json()
  }
  catch (e)
  {
    error.value = e.message
  }
  finally
  {
    isLoadingInput.value = false
  }
}

const loadOutputData = async () => {
  isLoadingOutput.value = false
  error.value = null

  try
  {
    const response = await fetch("/output.json")
    if (!response.ok)
      throw new Error("Ошибка загрузки output.json")
    outputData.value = await response.json()
  }
  catch (e)
  {
    error.value = e.message
  }
  finally
  {
    isLoadingOutput.value = false
  }
}

const goToResults = async () => {
  if (!outputData.value)
    await loadOutputData()
  currentPage.value = "results"
}

const goToSetup = () => currentPage.value = "setup"
</script>

<template>
  <div class="app-container">
    <div v-if="error" class="alert alert-danger m-4">{{ error }}</div>
    <nav class="app-nav">
      <button
        class="nav-btn"
        :class="{ active: currentPage === 'setup' }"
        @click="goToSetup"
      >
        Входные данные
      </button>
      <button 
        class="nav-btn" 
        :class="{ active: currentPage === 'results' }" 
        @click="goToResults"
        :disabled="isLoadingOutput"
      >
        {{ isLoadingOutput ? "Загрузка..." : "Результаты отчёта" }}
      </button>
    </nav>
    <DataSetup 
      v-if="currentPage === 'setup'" 
      :input-data="inputData"
      :is-loading="isLoadingInput"
      @load-input="loadInputData"
      @calculate="goToResults"
    />
    <ResultsPage 
      v-else-if="currentPage === 'results'" 
      :output-data="outputData"
      :is-loading="isLoadingOutput"
      @load-output="loadOutputData"
      @back="goToSetup"
    />
    <div v-else class="text-center mt-5">
      <p>Данные результатов ещё не загружены.</p>
      <button class="btn btn-primary" @click="loadOutputData">Загрузить output.json</button>
    </div>
  </div>
</template>

<style scoped>
.app-container {
  min-height: 100vh;
  padding: 20px;
}

.app-nav {
  display: flex;
  justify-content: center;
  gap: 10px;
  margin-bottom: 30px;
  padding: 10px;
  background: rgba(255, 255, 255, 0.05);
  border-radius: 8px;
  border: 1px solid rgba(255, 255, 255, 0.1);
}

.nav-btn {
  background: transparent;
  color: var(--text);
  border: 1px solid transparent;
  padding: 10px 25px;
  border-radius: 6px;
  cursor: pointer;
  transition: all 0.3s;
  font-weight: 500;
}

.nav-btn:hover:not(:disabled) {
  color: #fff;
  border-color: var(--accent-border);
}

.nav-btn.active {
  background: var(--accent-bg);
  color: #fff;
  border-color: var(--accent);
}

.nav-btn:disabled {
  opacity: 0.6;
  cursor: not-allowed;
}
</style>