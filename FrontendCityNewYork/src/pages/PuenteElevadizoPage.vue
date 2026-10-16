<script setup lang="ts">
import { ref } from 'vue'
import { puenteService } from '@/api/puenteService'

const loading = ref(false)
const error = ref<string | null>(null)

const subirPuente = async () => {
  if (loading.value) return
  loading.value = true
  try {
    error.value = null
    await puenteService.abrir()
  } catch (e: any) {
    error.value = 'Error al subir el puente'
  } finally {
    loading.value = false
  }
}

const bajarPuente = async () => {
  if (loading.value) return
  loading.value = true
  try {
    error.value = null
    await puenteService.cerrar()
  } catch (e: any) {
    error.value = 'Error al bajar el puente'
  } finally {
    loading.value = false
  }
}
</script>

<template>
  <div class="puente-page">
    <div class="puente-header">
      <h1>Control de Puente Elevadizo</h1>
      <p>Sistema de entrada a la ciudad de New York</p>
    </div>

    <div class="puente-control">
      <div class="botones">
        <button class="btn btn-subir" @click="subirPuente" :disabled="loading">
          SUBIR PUENTE
        </button>
        <button class="btn btn-bajar" @click="bajarPuente" :disabled="loading">
          BAJAR PUENTE
        </button>
      </div>

      <div v-if="error" class="error-message">{{ error }}</div>
    </div>
  </div>
</template>

<style scoped>
.puente-page {
  max-width: 600px;
  margin: 0 auto;
}

.puente-header {
  text-align: center;
  margin-bottom: 2rem;
}

.puente-header h1 {
  font-size: 1.8rem;
  color: var(--text-primary);
  margin-bottom: 0.5rem;
}

.puente-header p {
  color: var(--text-secondary);
  margin: 0;
}

.puente-control {
  background: var(--card-bg);
  border: 1px solid var(--border-color);
  border-radius: var(--radius-lg);
  padding: 2rem;
}

.botones {
  display: flex;
  gap: 1rem;
}

.btn {
  flex: 1;
  padding: 1rem 2rem;
  border: none;
  border-radius: var(--radius-md);
  font-size: 1rem;
  font-weight: 700;
  cursor: pointer;
  transition: all 0.2s;
}

.btn:disabled {
  opacity: 0.5;
  cursor: not-allowed;
}

.btn-subir {
  background: linear-gradient(135deg, #16A34A 0%, #15803D 100%);
  color: white;
}

.btn-subir:hover:not(:disabled) {
  background: linear-gradient(135deg, #15803D 0%, #166534 100%);
}

.btn-bajar {
  background: linear-gradient(135deg, #DC2626 0%, #B91C1C 100%);
  color: white;
}

.btn-bajar:hover:not(:disabled) {
  background: linear-gradient(135deg, #B91C1C 0%, #991B1B 100%);
}

.error-message {
  margin-top: 1rem;
  padding: 0.75rem 1rem;
  background: rgba(239, 68, 68, 0.1);
  border: 1px solid rgba(239, 68, 68, 0.3);
  border-radius: var(--radius-md);
  color: #DC2626;
  font-size: 0.9rem;
  text-align: center;
}
</style>