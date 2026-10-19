<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import { elevadorService, type LogItem } from '@/api/elevadorService'

const NIVEL_MIN = 0
const NIVEL_MAX = 5

const loading = ref(false)
const error = ref<string | null>(null)
const mensaje = ref<string | null>(null)

const nivelActual = ref(NIVEL_MIN)
const nivelesInput = ref(1)
const nivelDestino = ref(NIVEL_MIN)
const nivelesDisponibles = [0, 1, 2, 3, 4, 5]
const log = ref<LogItem[]>([])

let refreshTimer: number | null = null

const cargarLog = async () => {
  try {
    log.value = await elevadorService.obtenerLog(15)
  } catch {
  }
}

const refrescar = () => cargarLog()

const ejecutar = async (
  accion: () => Promise<any>,
  okMsg: string,
  dupMsg: string,
  prediccion: () => void
) => {
  if (loading.value) return
  loading.value = true
  prediccion()
  try {
    error.value = null
    mensaje.value = null
    const resultado = await accion()
    mensaje.value = resultado?.yaEstabaPendiente ? dupMsg : okMsg
    setTimeout(refrescar, 500)
  } catch (e: any) {
    mensaje.value = null
    error.value = 'No se pudo enviar la instruccion al elevador'
  } finally {
    loading.value = false
  }
}

const subir = () => {
  const delta = nivelesInput.value
  ejecutar(
    () => elevadorService.subir(delta),
    `Subir ${delta} nivel(es) encolado`,
    'La subida ya estaba en cola; no se duplico',
    () => { nivelActual.value = Math.min(NIVEL_MAX, nivelActual.value + delta) }
  )
}

const bajar = () => {
  const delta = nivelesInput.value
  ejecutar(
    () => elevadorService.bajar(delta),
    `Bajar ${delta} nivel(es) encolado`,
    'La bajada ya estaba en cola; no se duplico',
    () => { nivelActual.value = Math.max(NIVEL_MIN, nivelActual.value - delta) }
  )
}

const irANivel = () => {
  if (nivelDestino.value === nivelActual.value) {
    error.value = 'El elevador ya esta en ese nivel'
    return
  }
  const destino = nivelDestino.value
  ejecutar(
    () => elevadorService.ir(destino),
    `Ir al nivel ${destino} encolado`,
    'La instruccion ya estaba en cola; no se duplico',
    () => { nivelActual.value = destino }
  )
}

const parar = () => ejecutar(
  () => elevadorService.parar(),
  'Orden de detener agregada a la cola',
  'La orden de detener ya estaba en cola',
  () => {}
)

const calibrar = () => ejecutar(
  () => elevadorService.calibrar(),
  'Calibracion encolada (bajara a piso 0)',
  'La calibracion ya estaba en cola',
  () => { nivelActual.value = NIVEL_MIN }
)

const formatearFecha = (fecha: string) => {
  try {
    return new Date(fecha).toLocaleString('es-ES')
  } catch {
    return fecha
  }
}

const formatearInstruccion = (instr: string) => {
  return instr.replace(/_/g, ' ').toLowerCase().replace(/\b\w/g, l => l.toUpperCase())
}

onMounted(() => {
  cargarLog()
  refreshTimer = window.setInterval(refrescar, 5000)
})

onUnmounted(() => {
  if (refreshTimer !== null) {
    clearInterval(refreshTimer)
    refreshTimer = null
  }
})
</script>

<template>
  <div class="elevador-page">
    <div class="elevador-header">
      <h1>Empire State · Elevador</h1>
      <p>Sube y baja entre los 6 pisos del edificio (5000 pasos por nivel)</p>
    </div>

    <div class="elevador-grid">
      <div class="elevador-status card">
        <div class="status-header">
          <span class="status-label">Nivel actual</span>
        </div>

        <div class="nivel-display">
          <div class="nivel-numero">{{ nivelActual }}</div>
          <div class="nivel-texto">PISO</div>
        </div>
      </div>

      <div class="elevador-control card">
        <h2>Movimiento por niveles</h2>
        <div class="niveles-row">
          <label>Cantidad de niveles</label>
          <div class="stepper">
            <button class="stepper-btn" @click="nivelesInput = Math.max(1, nivelesInput - 1)" :disabled="loading">&minus;</button>
            <input type="number" v-model.number="nivelesInput" min="1" max="5" :disabled="loading" />
            <button class="stepper-btn" @click="nivelesInput = Math.min(5, nivelesInput + 1)" :disabled="loading">&plus;</button>
          </div>
        </div>

        <div class="botones-movimiento">
          <button class="btn btn-subir" @click="subir" :disabled="loading">
            SUBIR {{ nivelesInput }} NIVEL(ES)
          </button>
          <button class="btn btn-bajar" @click="bajar" :disabled="loading">
            BAJAR {{ nivelesInput }} NIVEL(ES)
          </button>
        </div>

        <div class="separador"></div>

        <h2>Ir a un piso especifico</h2>
        <div class="ir-row">
          <select v-model.number="nivelDestino" :disabled="loading">
            <option v-for="n in nivelesDisponibles" :key="n" :value="n">Piso {{ n }}</option>
          </select>
          <button class="btn btn-ir" @click="irANivel" :disabled="loading">
            IR AL PISO {{ nivelDestino }}
          </button>
        </div>

        <div class="separador"></div>

        <h2>Acciones rapidas</h2>
        <div class="botones-rapido">
          <button class="btn btn-parar" @click="parar" :disabled="loading">
            PARAR
          </button>
          <button class="btn btn-calibrar" @click="calibrar" :disabled="loading">
            CALIBRAR (ir a piso 0)
          </button>
        </div>

        <div v-if="error" class="error-message">{{ error }}</div>
        <div v-if="mensaje" class="success-message">{{ mensaje }}</div>
      </div>
    </div>

    <div class="log-section card">
      <div class="log-header">
        <h2>Historial</h2>
        <button class="btn-refresh" @click="refrescar" :disabled="loading">Actualizar</button>
      </div>
      <div v-if="log.length === 0" class="log-empty">Sin instrucciones registradas todavia</div>
      <table v-else class="log-table">
        <thead>
          <tr>
            <th>Fecha</th>
            <th>Instruccion</th>
            <th>Resultado</th>
            <th>Nivel</th>
            <th>Tipo</th>
          </tr>
        </thead>
        <tbody>
          <tr v-for="item in log" :key="item.Id">
            <td>{{ formatearFecha(item.Fecha) }}</td>
            <td>{{ formatearInstruccion(item.Instruccion) }}</td>
            <td>
              <span class="resultado" :class="item.Resultado === 'OK' ? 'ok' : 'error'">
                {{ item.Resultado }}
              </span>
            </td>
            <td>{{ item.Nivel ?? '-' }}</td>
            <td>{{ item.Tipo }}</td>
          </tr>
        </tbody>
      </table>
    </div>
  </div>
</template>

<style scoped>
.elevador-page {
  max-width: 1100px;
  margin: 0 auto;
}

.elevador-header {
  text-align: center;
  margin-bottom: 1.5rem;
}

.elevador-header h1 {
  font-size: 1.8rem;
  color: var(--nyc-navy);
  margin-bottom: 0.5rem;
}

.elevador-header p {
  color: var(--nyc-steel);
  margin: 0;
}

.card {
  background: white;
  border: 1px solid var(--nyc-gray-200);
  border-radius: var(--radius-lg);
  padding: 1.5rem;
  box-shadow: var(--shadow-sm);
}

.elevador-grid {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 1.5rem;
  margin-bottom: 1.5rem;
}

@media (max-width: 768px) {
  .elevador-grid {
    grid-template-columns: 1fr;
  }
}

.elevador-status {
  display: flex;
  flex-direction: column;
  gap: 1rem;
}

.status-header {
  text-align: center;
}

.status-label {
  font-weight: 600;
  color: var(--nyc-navy);
  text-transform: uppercase;
  font-size: 0.85rem;
  letter-spacing: 0.5px;
}

.nivel-display {
  display: flex;
  align-items: baseline;
  justify-content: center;
  gap: 0.75rem;
  padding: 1.5rem;
  background: linear-gradient(135deg, var(--nyc-navy) 0%, #1E3A5F 100%);
  border-radius: var(--radius-md);
  color: var(--nyc-taxi);
}

.nivel-numero {
  font-size: 5rem;
  font-weight: 800;
  line-height: 1;
}

.nivel-texto {
  font-size: 1.1rem;
  font-weight: 600;
  letter-spacing: 2px;
}

.elevador-control h2 {
  font-size: 1rem;
  font-weight: 700;
  color: var(--nyc-navy);
  margin: 0 0 0.75rem 0;
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

.niveles-row,
.ir-row {
  display: flex;
  align-items: center;
  gap: 0.75rem;
  margin-bottom: 1rem;
}

.niveles-row label {
  font-weight: 600;
  color: var(--nyc-steel);
  font-size: 0.9rem;
}

.stepper {
  display: flex;
  align-items: center;
  gap: 0.25rem;
  border: 1px solid var(--nyc-gray-300);
  border-radius: var(--radius-md);
  overflow: hidden;
}

.stepper-btn {
  background: var(--nyc-gray-100);
  border: none;
  width: 32px;
  height: 32px;
  font-size: 1.1rem;
  font-weight: 700;
  cursor: pointer;
  color: var(--nyc-navy);
}

.stepper-btn:hover:not(:disabled) {
  background: var(--nyc-gray-200);
}

.stepper-btn:disabled {
  opacity: 0.4;
  cursor: not-allowed;
}

.stepper input {
  width: 48px;
  height: 32px;
  border: none;
  text-align: center;
  font-size: 1rem;
  font-weight: 700;
  color: var(--nyc-navy);
}

.stepper input:focus {
  outline: none;
}

.ir-row select {
  flex: 1;
  padding: 0.6rem;
  border: 1px solid var(--nyc-gray-300);
  border-radius: var(--radius-md);
  font-size: 0.95rem;
  background: white;
  color: var(--nyc-navy);
  font-weight: 600;
  cursor: pointer;
}

.botones-movimiento,
.botones-rapido {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 0.75rem;
}

.separador {
  height: 1px;
  background: var(--nyc-gray-200);
  margin: 1.25rem 0;
}

.btn {
  padding: 0.9rem 1rem;
  border: none;
  border-radius: var(--radius-md);
  font-size: 0.95rem;
  font-weight: 700;
  cursor: pointer;
  transition: all 0.2s;
  color: white;
}

.btn:disabled {
  opacity: 0.5;
  cursor: not-allowed;
}

.btn-subir {
  background: linear-gradient(135deg, #16A34A 0%, #15803D 100%);
}

.btn-subir:hover:not(:disabled) {
  background: linear-gradient(135deg, #15803D 0%, #166534 100%);
}

.btn-bajar {
  background: linear-gradient(135deg, #DC2626 0%, #B91C1C 100%);
}

.btn-bajar:hover:not(:disabled) {
  background: linear-gradient(135deg, #B91C1C 0%, #991B1B 100%);
}

.btn-ir {
  background: linear-gradient(135deg, var(--nyc-navy) 0%, #1E3A5F 100%);
  color: var(--nyc-taxi);
}

.btn-ir:hover:not(:disabled) {
  background: linear-gradient(135deg, #1E3A5F 0%, #2D4A6F 100%);
}

.btn-parar {
  background: linear-gradient(135deg, #F59E0B 0%, #D97706 100%);
}

.btn-parar:hover:not(:disabled) {
  background: linear-gradient(135deg, #D97706 0%, #B45309 100%);
}

.btn-calibrar {
  background: linear-gradient(135deg, var(--nyc-steel) 0%, #2D3748 100%);
}

.btn-calibrar:hover:not(:disabled) {
  background: linear-gradient(135deg, #2D3748 0%, #1A202C 100%);
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

.log-section {
  padding: 1.5rem;
}

.log-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 1rem;
}

.log-header h2 {
  font-size: 1.1rem;
  font-weight: 700;
  color: var(--nyc-navy);
  margin: 0;
}

.btn-refresh {
  padding: 0.4rem 0.9rem;
  background: var(--nyc-gray-100);
  border: 1px solid var(--nyc-gray-300);
  border-radius: var(--radius-md);
  cursor: pointer;
  font-size: 0.85rem;
  font-weight: 600;
  color: var(--nyc-navy);
}

.btn-refresh:hover:not(:disabled) {
  background: var(--nyc-gray-200);
}

.log-empty {
  text-align: center;
  padding: 2rem;
  color: var(--nyc-steel);
  font-style: italic;
}

.log-table {
  width: 100%;
  border-collapse: collapse;
  font-size: 0.9rem;
}

.log-table th {
  text-align: left;
  padding: 0.6rem;
  background: var(--nyc-gray-100);
  color: var(--nyc-navy);
  font-weight: 700;
  text-transform: uppercase;
  font-size: 0.75rem;
  letter-spacing: 0.5px;
}

.log-table th:first-child {
  border-top-left-radius: var(--radius-sm);
  border-bottom-left-radius: var(--radius-sm);
}

.log-table th:last-child {
  border-top-right-radius: var(--radius-sm);
  border-bottom-right-radius: var(--radius-sm);
}

.log-table td {
  padding: 0.6rem;
  border-top: 1px solid var(--nyc-gray-200);
  color: var(--nyc-steel);
}

.resultado {
  padding: 0.2rem 0.5rem;
  border-radius: 999px;
  font-size: 0.75rem;
  font-weight: 700;
}

.resultado.ok {
  background: rgba(34, 197, 94, 0.15);
  color: #15803D;
}

.resultado.error {
  background: rgba(239, 68, 68, 0.15);
  color: #B91C1C;
}
</style>