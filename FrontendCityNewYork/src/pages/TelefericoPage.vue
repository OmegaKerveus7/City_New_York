<script setup lang="ts">
import { ref } from 'vue'
import { telefericoService } from '@/api/telefericoService'

const loading = ref(false)
const error = ref<string | null>(null)
const mensaje = ref<string | null>(null)

const avanzarTeleferico = async () => {
  if (loading.value) return
  loading.value = true
  try {
    error.value = null
    const resultado = await telefericoService.avanzar()
    mensaje.value = resultado.yaEstabaPendiente
      ? 'El avance ya estaba en cola; no se agregó otra orden.'
      : 'Avance de 3000 pasos agregado a la cola.'
  } catch (e: any) {
    mensaje.value = null
    error.value = 'Error al avanzar el teleferico'
  } finally {
    loading.value = false
  }
}

const regresarTeleferico = async () => {
  if (loading.value) return
  loading.value = true
  try {
    error.value = null
    const resultado = await telefericoService.regresar()
    mensaje.value = resultado.yaEstabaPendiente
      ? 'El regreso ya estaba en cola; no se agregó otra orden.'
      : 'Regreso de 3000 pasos agregado a la cola.'
  } catch (e: any) {
    mensaje.value = null
    error.value = 'Error al regresar el teleferico'
  } finally {
    loading.value = false
  }
}

const pararTeleferico = async () => {
  if (loading.value) return
  loading.value = true
  try {
    error.value = null
    const resultado = await telefericoService.parar()
    mensaje.value = resultado.yaEstabaPendiente
      ? 'La orden de detener ya estaba en cola.'
      : 'Orden de detener agregada a la cola.'
  } catch (e: any) {
    mensaje.value = null
    error.value = 'Error al parar el teleferico'
  } finally {
    loading.value = false
  }
}
</script>

<template>
  <div class="teleferico-page">
    <div class="teleferico-header">
      <h1>Control de Teleferico</h1>
      <p>Sistema de transporte aereo del puerto · movimientos de 3000 pasos</p>
    </div>

    <div class="teleferico-control">
      <div class="botones">
        <button class="btn btn-avanzar" @click="avanzarTeleferico" :disabled="loading">
          AVANZAR · 3000 PASOS
        </button>
        <button class="btn btn-parar" @click="pararTeleferico" :disabled="loading">
          PARAR
        </button>
        <button class="btn btn-regresar" @click="regresarTeleferico" :disabled="loading">
          REGRESAR · 3000 PASOS
        </button>
      </div>

      <div v-if="error" class="error-message">{{ error }}</div>
      <div v-if="mensaje" class="success-message">{{ mensaje }}</div>
    </div>
  </div>
</template>

<style scoped>
.teleferico-page {
  max-width: 600px;
  margin: 0 auto;
}

.teleferico-header {
  text-align: center;
  margin-bottom: 2rem;
}

.teleferico-header h1 {
  font-size: 1.8rem;
  color: var(--text-primary);
  margin-bottom: 0.5rem;
}

.teleferico-header p {
  color: var(--text-secondary);
  margin: 0;
}

.teleferico-control {
  background: var(--card-bg);
  border: 1px solid var(--border-color);
  border-radius: var(--radius-lg);
  padding: 2rem;
}

.botones {
  display: flex;
  flex-direction: column;
  gap: 1rem;
}

.btn {
  padding: 1.25rem 2rem;
  border: none;
  border-radius: var(--radius-md);
  font-size: 1.1rem;
  font-weight: 700;
  cursor: pointer;
  transition: all 0.2s;
}

.btn:disabled {
  opacity: 0.5;
  cursor: not-allowed;
}

.btn-avanzar {
  background: linear-gradient(135deg, #16A34A 0%, #15803D 100%);
  color: white;
}

.btn-avanzar:hover:not(:disabled) {
  background: linear-gradient(135deg, #15803D 0%, #166534 100%);
}

.btn-parar {
  background: linear-gradient(135deg, #F59E0B 0%, #D97706 100%);
  color: white;
}

.btn-parar:hover:not(:disabled) {
  background: linear-gradient(135deg, #D97706 0%, #B45309 100%);
}

.btn-regresar {
  background: linear-gradient(135deg, #DC2626 0%, #B91C1C 100%);
  color: white;
}

.btn-regresar:hover:not(:disabled) {
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

.success-message {
  margin-top: 1rem;
  padding: 0.75rem 1rem;
  background: rgba(34, 197, 94, 0.1);
  border: 1px solid rgba(34, 197, 94, 0.3);
  border-radius: var(--radius-md);
  color: #15803D;
  font-size: 0.9rem;
  text-align: center;
}
</style>
