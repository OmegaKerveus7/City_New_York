<script setup lang="ts">
import { ref, computed } from 'vue'
import { puenteService, type PuenteEstado } from '@/api/puenteService'

const servoEstado = ref<PuenteEstado | null>(null)
const loading = ref(false)
const error = ref<string | null>(null)

const isAbierto = computed(() => servoEstado.value?.estado === 'ABIERTO')

const obtenerEstado = async () => {
  try {
    error.value = null
    servoEstado.value = await puenteService.obtenerEstado()
  } catch (e: any) {
    error.value = 'No se pudo conectar'
  }
}

const subirPuente = async () => {
  if (loading.value) return
  loading.value = true
  try {
    error.value = null
    await puenteService.abrir()
    setTimeout(obtenerEstado, 1500)
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
    setTimeout(obtenerEstado, 1500)
  } catch (e: any) {
    error.value = 'Error al bajar el puente'
  } finally {
    loading.value = false
  }
}

obtenerEstado()
</script>

<template>
  <div class="puente-page">
    <div class="puente-header">
      <h1>Control de Puente Elevadizo</h1>
      <p>Sistema de entrada a la ciudad de New York</p>
    </div>

    <div class="puente-control">
      <div class="estado-actual">
        <span class="estado-label">Estado:</span>
        <span class="estado-badge" :class="isAbierto ? 'estado-abierto' : 'estado-cerrado'">
          {{ servoEstado?.estado || 'DESCONECTADO' }}
        </span>
      </div>

      <div class="control-info">
        <div class="info-row">
          <span class="label">Angulo:</span>
          <span class="value">{{ servoEstado?.angulo ?? '--' }}°</span>
        </div>
        <div class="info-row">
          <span class="label">Mega:</span>
          <span class="status-dot" :class="servoEstado?.megaOnline ? 'online' : 'offline'"></span>
        </div>
        <div class="info-row">
          <span class="label">Esclavo:</span>
          <span class="status-dot" :class="servoEstado?.esclavoOnline ? 'online' : 'offline'"></span>
        </div>
      </div>

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

.estado-actual {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 1rem;
  margin-bottom: 1.5rem;
  padding-bottom: 1.5rem;
  border-bottom: 1px solid var(--border-color);
}

.estado-label {
  font-size: 1.1rem;
  color: var(--text-secondary);
}

.estado-badge {
  padding: 0.5rem 1.5rem;
  border-radius: var(--radius-md);
  font-weight: 700;
  font-size: 1.2rem;
}

.estado-abierto {
  background: rgba(34, 197, 94, 0.2);
  color: #16A34A;
}

.estado-cerrado {
  background: rgba(239, 68, 68, 0.2);
  color: #DC2626;
}

.control-info {
  margin-bottom: 1.5rem;
}

.info-row {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 0.75rem 0;
  border-bottom: 1px solid var(--border-color);
}

.info-row:last-child {
  border-bottom: none;
}

.info-row .label {
  color: var(--text-secondary);
}

.info-row .value {
  font-weight: 600;
  color: var(--text-primary);
}

.status-dot {
  width: 12px;
  height: 12px;
  border-radius: 50%;
}

.status-dot.online {
  background: #16A34A;
  box-shadow: 0 0 8px rgba(22, 163, 74, 0.5);
}

.status-dot.offline {
  background: #DC2626;
  box-shadow: 0 0 8px rgba(220, 38, 38, 0.5);
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