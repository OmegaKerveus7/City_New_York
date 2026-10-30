<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import { semaforoService } from '@/api/semaforoService'
import type { CarrilSemaforo, ColorSemaforo } from '@/api/semaforoService'

const ns = ref<string | null>(null)
const eo = ref<string | null>(null)
const modo = ref<string | null>(null)

const loading = ref(false)
const error = ref<string | null>(null)
const mensaje = ref<string | null>(null)
const ultimaActualizacion = ref<string | null>(null)

const colores: ColorSemaforo[] = ['ROJO', 'AMARILLO', 'VERDE']
const carriles: { id: CarrilSemaforo; titulo: string; subtitulo: string }[] = [
  { id: 'NS', titulo: 'Norte - Sur', subtitulo: 'Avenida Central' },
  { id: 'EO', titulo: 'Este - Oeste', subtitulo: 'Calle transversal' }
]

let temporizador: number | undefined

const colorActual = (carril: CarrilSemaforo) => (carril === 'NS' ? ns.value : eo.value)

const cargarEstado = async () => {
  try {
    const estado = await semaforoService.obtenerEstado()
    ns.value = estado.ns
    eo.value = estado.eo
    modo.value = estado.modo
    const componentes = estado.componentes || []
    if (componentes.length > 0) {
      ultimaActualizacion.value = componentes[0].FechaActualizacion
    }
  } catch (e) {
    // si el backend no responde dejamos el ultimo estado conocido
  }
}

const ejecutar = async (accion: () => Promise<any>, texto: string) => {
  if (loading.value) return
  loading.value = true
  error.value = null
  mensaje.value = null
  try {
    const resultado = await accion()
    mensaje.value = resultado?.yaEstabaPendiente
      ? 'La orden ya estaba en cola; no se duplico.'
      : texto
    await cargarEstado()
  } catch (e) {
    error.value = 'No se pudo enviar la orden al controlador de semáforos'
  } finally {
    loading.value = false
  }
}

const cambiar = (carril: CarrilSemaforo, color: ColorSemaforo) =>
  ejecutar(() => semaforoService.cambiar(carril, color), `Orden enviada: ${carril} en ${color.toLowerCase()}`)

const activarAuto = () => ejecutar(() => semaforoService.modoAutomatico(), 'Ciclo automático activado')
const pararAuto = () => ejecutar(() => semaforoService.parar(), 'Ciclo automático detenido')

onMounted(() => {
  cargarEstado()
  temporizador = window.setInterval(cargarEstado, 5000)
})

onUnmounted(() => {
  if (temporizador) window.clearInterval(temporizador)
})
</script>

<template>
  <div class="semaforo-page">
    <div class="semaforo-header animate-fade-in-up">
      <div>
        <h1>Control de Semáforos</h1>
        <p>Intersecciones de la ciudad · modo manual y modo automático</p>
      </div>
      <div class="modo-badge" :class="{ 'modo-badge--auto': modo === 'AUTO' }">
        <span class="modo-dot"></span>
        {{ modo === 'AUTO' ? 'MODO AUTOMÁTICO' : 'MODO MANUAL' }}
      </div>
    </div>

    <div class="semaforo-grid">
      <div
        v-for="(carril, index) in carriles"
        :key="carril.id"
        class="semaforo-card animate-fade-in-up"
        :class="`delay-${index + 1}`"
      >
        <div class="card-title">
          <h3>{{ carril.titulo }}</h3>
          <span class="card-subtitle">{{ carril.subtitulo }}</span>
        </div>

        <div class="semaforo-visor">
          <div
            class="farola"
            v-for="color in ['APAGADO', ...colores]"
            :key="color"
            :class="[`farola--${color.toLowerCase()}`, { 'farola--activo': colorActual(carril.id) === color }]"
          >
            <span class="farola__luz"></span>
            <span class="farola__label">{{ color === 'APAGADO' ? 'OFF' : color }}</span>
          </div>
        </div>

        <div class="botones">
          <button
            v-for="color in colores"
            :key="color"
            class="btn"
            :class="`btn--${color.toLowerCase()}`"
            :disabled="loading"
            @click="cambiar(carril.id, color)"
          >
            {{ color }}
          </button>
        </div>
      </div>
    </div>

    <div class="semaforo-panel animate-fade-in-up delay-3">
      <div class="panel-actions">
        <button class="btn btn--auto" :disabled="loading" @click="activarAuto">
          ▶ MODO AUTOMÁTICO
        </button>
        <button class="btn btn--stop" :disabled="loading" @click="pararAuto">
          ■ PARAR CICLO
        </button>
      </div>

      <p class="panel-note">
        Ciclo automático: verde 8 s → amarillo 2 s → rojo 8 s, alternando el carril
        Norte-Sur y el Este-Oeste.
      </p>

      <div v-if="error" class="error-message">{{ error }}</div>
      <div v-if="mensaje" class="success-message">{{ mensaje }}</div>

      <div v-if="ultimaActualizacion" class="panel-footer">
        Última lectura del dispositivo:
        {{ new Date(ultimaActualizacion).toLocaleString('es-ES') }}
      </div>
    </div>
  </div>
</template>

<style scoped>
.semaforo-page {
  max-width: 1100px;
  margin: 0 auto;
}

.semaforo-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  gap: 1rem;
  margin-bottom: 2rem;
  flex-wrap: wrap;
}

.semaforo-header h1 {
  font-size: 1.8rem;
  color: var(--text-primary);
  margin-bottom: 0.35rem;
}

.semaforo-header p {
  color: var(--text-secondary);
  margin: 0;
}

.modo-badge {
  display: inline-flex;
  align-items: center;
  gap: 0.5rem;
  padding: 0.5rem 1rem;
  border-radius: 9999px;
  background: var(--card-bg);
  border: 1px solid var(--border-color);
  font-size: 0.8rem;
  font-weight: 700;
  letter-spacing: 0.04em;
  color: var(--text-secondary);
}

.modo-badge--auto {
  background: rgba(56, 161, 105, 0.12);
  border-color: rgba(56, 161, 105, 0.4);
  color: #2F855A;
}

.modo-dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  background: var(--text-secondary);
}

.modo-badge--auto .modo-dot {
  background: #38A169;
  animation: parpadeo 1.2s infinite;
}

.semaforo-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(320px, 1fr));
  gap: 1.5rem;
  margin-bottom: 1.5rem;
}

.semaforo-card {
  background: var(--card-bg);
  border: 1px solid var(--border-color);
  border-radius: var(--radius-lg);
  padding: 1.5rem;
  box-shadow: var(--shadow-sm);
}

.card-title {
  margin-bottom: 1.25rem;
}

.card-title h3 {
  font-size: 1.05rem;
  font-weight: 700;
  color: var(--text-primary);
}

.card-subtitle {
  font-size: 0.78rem;
  color: var(--text-secondary);
}

.semaforo-visor {
  display: flex;
  justify-content: center;
  gap: 1rem;
  background: #1A202C;
  border-radius: var(--radius-md);
  padding: 1.25rem;
  margin-bottom: 1.25rem;
}

.farola {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 0.4rem;
}

.farola__luz {
  width: 34px;
  height: 34px;
  border-radius: 50%;
  background: #2D3748;
  border: 2px solid #4A5568;
  transition: all 0.25s ease;
}

.farola__label {
  font-size: 0.62rem;
  letter-spacing: 0.08em;
  color: #718096;
  font-weight: 700;
}

.farola--rojo.farola--activo .farola__luz {
  background: #E53E3E;
  border-color: #FC8181;
  box-shadow: 0 0 14px rgba(229, 62, 62, 0.75);
}

.farola--amarillo.farola--activo .farola__luz {
  background: #F6AD55;
  border-color: #FBD38D;
  box-shadow: 0 0 14px rgba(246, 173, 85, 0.75);
}

.farola--verde.farola--activo .farola__luz {
  background: #48BB78;
  border-color: #9AE6B4;
  box-shadow: 0 0 14px rgba(72, 187, 120, 0.75);
}

.farola--activo .farola__label {
  color: #F7FAFC;
}

.botones {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 0.6rem;
}

.btn {
  padding: 0.7rem 0.5rem;
  border: none;
  border-radius: var(--radius-md);
  font-size: 0.78rem;
  font-weight: 700;
  cursor: pointer;
  color: white;
  transition: opacity 0.2s, transform 0.1s;
}

.btn:active:not(:disabled) {
  transform: translateY(1px);
}

.btn:disabled {
  opacity: 0.5;
  cursor: not-allowed;
}

.btn--rojo { background: linear-gradient(135deg, #E53E3E 0%, #C53030 100%); }
.btn--amarillo { background: linear-gradient(135deg, #F6AD55 0%, #DD6B20 100%); }
.btn--verde { background: linear-gradient(135deg, #48BB78 0%, #2F855A 100%); }
.btn--auto { background: linear-gradient(135deg, #3182CE 0%, #2B6CB0 100%); }
.btn--stop { background: linear-gradient(135deg, #718096 0%, #4A5568 100%); }

.semaforo-panel {
  background: var(--card-bg);
  border: 1px solid var(--border-color);
  border-radius: var(--radius-lg);
  padding: 1.5rem;
  box-shadow: var(--shadow-sm);
}

.panel-actions {
  display: flex;
  gap: 1rem;
  flex-wrap: wrap;
}

.panel-actions .btn {
  padding: 0.9rem 1.5rem;
  font-size: 0.85rem;
}

.panel-note {
  margin: 1rem 0 0;
  font-size: 0.8rem;
  color: var(--text-secondary);
}

.panel-footer {
  margin-top: 1rem;
  padding-top: 0.75rem;
  border-top: 1px solid var(--border-color);
  font-size: 0.72rem;
  color: var(--text-secondary);
}

.error-message {
  margin-top: 1rem;
  padding: 0.75rem 1rem;
  background: rgba(229, 62, 62, 0.1);
  border: 1px solid rgba(229, 62, 62, 0.3);
  border-radius: var(--radius-md);
  color: #C53030;
  font-size: 0.85rem;
}

.success-message {
  margin-top: 1rem;
  padding: 0.75rem 1rem;
  background: rgba(56, 161, 105, 0.1);
  border: 1px solid rgba(56, 161, 105, 0.3);
  border-radius: var(--radius-md);
  color: #2F855A;
  font-size: 0.85rem;
}

@keyframes parpadeo {
  0%, 100% { opacity: 1; }
  50% { opacity: 0.35; }
}
</style>
