<script setup lang="ts">
import { ref } from 'vue'
import DashboardLayout from '@/layouts/DashboardLayout.vue'

const bridges = ref([
  { id: 1, nombre: 'Puente de Brooklyn', estado: 'Operativo', ultimaInspeccion: '2024-01-15', nivel: 'Alto', estadoPuente: 'Elevado' },
  { id: 2, nombre: 'Puente de Manhattan', estado: 'Operativo', ultimaInspeccion: '2024-01-10', nivel: 'Alto', estadoPuente: 'Bajo' },
  { id: 3, nombre: 'Puente de Queensboro', estado: 'Mantenimiento', ultimaInspeccion: '2024-01-08', nivel: 'Medio', estadoPuente: 'Elevado' },
  { id: 4, nombre: 'Puente de Williamsburg', estado: 'Operativo', ultimaInspeccion: '2024-01-12', nivel: 'Alto', estadoPuente: 'Bajo' },
  { id: 5, nombre: 'Puente de Williamsburgh', estado: 'Inspección', ultimaInspeccion: '2023-12-20', nivel: 'Bajo', estadoPuente: 'Elevado' },
])

const selectedBridge = ref<typeof bridges.value[0] | null>(null)
const showModal = ref(false)

const openDetails = (bridge: typeof bridges.value[0]) => {
  selectedBridge.value = bridge
  showModal.value = true
}

const closeModal = () => {
  showModal.value = false
  selectedBridge.value = null
}

const toggleBridge = (bridge: typeof bridges.value[0]) => {
  bridge.estadoPuente = bridge.estadoPuente === 'Elevado' ? 'Bajo' : 'Elevado'
}

const getStatusClass = (estado: string) => {
  switch (estado.toLowerCase()) {
    case 'operativo': return 'status--success'
    case 'mantenimiento': return 'status--warning'
    case 'inspección': return 'status--info'
    default: return ''
  }
}

const getLevelClass = (nivel: string) => {
  switch (nivel.toLowerCase()) {
    case 'alto': return 'level--high'
    case 'medio': return 'level--medium'
    case 'bajo': return 'level--low'
    default: return ''
  }
}
</script>

<template>
  <DashboardLayout>
    <div class="puente-elevadizo">
      <div class="welcome-banner animate-fade-in-up">
        <div class="banner-icon">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <path d="M4 15s1-1 4-1 5 2 8 2 4-1 4-1V3s-1 1-4 1-5-2-8-2-4 1-4 1z" />
            <line x1="4" y1="22" x2="4" y2="15" />
          </svg>
        </div>
        <div class="banner-text">
          <h2>Puente Elevadizo</h2>
          <p>Control y monitoreo de puentes levadizos de entrada a la ciudad</p>
        </div>
      </div>

      <div class="stats-grid">
        <div class="stat-card animate-fade-in-up delay-1">
          <div class="stat-icon stat-icon--green">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <path d="M22 11.08V12a10 10 0 11-5.93-9.14" />
              <polyline points="22 4 12 14.01 9 11.01" />
            </svg>
          </div>
          <div class="stat-info">
            <span class="stat-value">{{ bridges.filter(b => b.estado === 'Operativo').length }}</span>
            <span class="stat-label">Operativos</span>
          </div>
        </div>

        <div class="stat-card animate-fade-in-up delay-2">
          <div class="stat-icon stat-icon--yellow">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <path d="M14.7 6.3a1 1 0 000 1.4l1.6 1.6a1 1 0 001.4 0l3.77-3.77a6 6 0 01-7.94 7.94l-6.91 6.91a2.12 2.12 0 01-3-3l6.91-6.91a6 6 0 017.94-7.94l-3.76 3.76z" />
            </svg>
          </div>
          <div class="stat-info">
            <span class="stat-value">{{ bridges.filter(b => b.estado === 'Mantenimiento').length }}</span>
            <span class="stat-label">Mantenimiento</span>
          </div>
        </div>

        <div class="stat-card animate-fade-in-up delay-3">
          <div class="stat-icon stat-icon--blue">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <circle cx="12" cy="12" r="10" />
              <polyline points="12 6 12 12 16 14" />
            </svg>
          </div>
          <div class="stat-info">
            <span class="stat-value">5</span>
            <span class="stat-label">Total Puentes</span>
          </div>
        </div>

        <div class="stat-card animate-fade-in-up delay-4">
          <div class="stat-icon stat-icon--red">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <path d="M10.29 3.86L1.82 18a2 2 0 001.71 3h16.94a2 2 0 001.71-3L13.71 3.86a2 2 0 00-3.42 0z" />
              <line x1="12" y1="9" x2="12" y2="13" />
              <line x1="12" y1="17" x2="12.01" y2="17" />
            </svg>
          </div>
          <div class="stat-info">
            <span class="stat-value">{{ bridges.filter(b => b.estadoPuente === 'Elevado').length }}</span>
            <span class="stat-label">Elevados</span>
          </div>
        </div>
      </div>

      <div class="bridges-section animate-fade-in-up delay-2">
        <div class="section-header">
          <h3>Puentes Elevadizos</h3>
        </div>

        <div class="bridges-table">
          <table>
            <thead>
              <tr>
                <th>Nombre</th>
                <th>Estado</th>
                <th>Nivel de Tráfico</th>
                <th>Posición</th>
                <th>Última Inspección</th>
                <th>Acciones</th>
              </tr>
            </thead>
            <tbody>
              <tr v-for="bridge in bridges" :key="bridge.id">
                <td>
                  <div class="bridge-name">
                    <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="bridge-icon">
                      <path d="M4 15s1-1 4-1 5 2 8 2 4-1 4-1V3s-1 1-4 1-5-2-8-2-4 1-4 1z" />
                      <line x1="4" y1="22" x2="4" y2="15" />
                    </svg>
                    {{ bridge.nombre }}
                  </div>
                </td>
                <td>
                  <span class="status-badge" :class="getStatusClass(bridge.estado)">
                    {{ bridge.estado }}
                  </span>
                </td>
                <td>
                  <span class="level-badge" :class="getLevelClass(bridge.nivel)">
                    {{ bridge.nivel }}
                  </span>
                </td>
                <td>
                  <span class="position-badge" :class="bridge.estadoPuente === 'Elevado' ? 'position--up' : 'position--down'">
                    <svg v-if="bridge.estadoPuente === 'Elevado'" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                      <polyline points="18 15 12 9 6 15" />
                    </svg>
                    <svg v-else viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                      <polyline points="6 9 12 15 18 9" />
                    </svg>
                    {{ bridge.estadoPuente }}
                  </span>
                </td>
                <td>{{ bridge.ultimaInspeccion }}</td>
                <td>
                  <div class="actions">
                    <button class="btn-action btn-toggle" @click="toggleBridge(bridge)" :disabled="bridge.estado !== 'Operativo'">
                      <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                        <polyline points="17 1 21 5 17 9" />
                        <path d="M3 11V9a4 4 0 014-4h14" />
                        <polyline points="7 23 3 19 7 15" />
                        <path d="M21 13v2a4 4 0 01-4 4H3" />
                      </svg>
                    </button>
                    <button class="btn-action btn-view" @click="openDetails(bridge)">
                      <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                        <path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z" />
                        <circle cx="12" cy="12" r="3" />
                      </svg>
                    </button>
                  </div>
                </td>
              </tr>
            </tbody>
          </table>
        </div>
      </div>

      <div v-if="showModal" class="modal-overlay" @click="closeModal">
        <div class="modal animate-scale-in" @click.stop>
          <div class="modal-header">
            <h3>{{ selectedBridge?.nombre }}</h3>
            <button class="modal-close" @click="closeModal">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                <line x1="18" y1="6" x2="6" y2="18" />
                <line x1="6" y1="6" x2="18" y2="18" />
              </svg>
            </button>
          </div>
          <div class="modal-body">
            <div class="detail-row">
              <span class="detail-label">Estado:</span>
              <span class="status-badge" :class="getStatusClass(selectedBridge?.estado || '')">
                {{ selectedBridge?.estado }}
              </span>
            </div>
            <div class="detail-row">
              <span class="detail-label">Nivel de Tráfico:</span>
              <span class="level-badge" :class="getLevelClass(selectedBridge?.nivel || '')">
                {{ selectedBridge?.nivel }}
              </span>
            </div>
            <div class="detail-row">
              <span class="detail-label">Posición Actual:</span>
              <span class="detail-value">{{ selectedBridge?.estadoPuente }}</span>
            </div>
            <div class="detail-row">
              <span class="detail-label">Última Inspección:</span>
              <span class="detail-value">{{ selectedBridge?.ultimaInspeccion }}</span>
            </div>
          </div>
          <div class="modal-footer">
            <button class="btn-modal btn-secondary" @click="closeModal">Cerrar</button>
            <button class="btn-modal btn-primary" @click="selectedBridge && toggleBridge(selectedBridge)" :disabled="selectedBridge?.estado !== 'Operativo'">
              Cambiar Posición
            </button>
          </div>
        </div>
      </div>
    </div>
  </DashboardLayout>
</template>

<style scoped>
.puente-elevadizo {
  max-width: 1400px;
}

.welcome-banner {
  background: linear-gradient(135deg, #B45309 0%, #92400E 100%);
  border-radius: var(--radius-lg);
  padding: 1.5rem 2rem;
  display: flex;
  align-items: center;
  gap: 1.25rem;
  margin-bottom: 2rem;
  box-shadow: var(--shadow-lg);
}

.banner-icon {
  width: 56px;
  height: 56px;
  background: rgba(255,255,255,0.2);
  border-radius: var(--radius-md);
  display: flex;
  align-items: center;
  justify-content: center;
}

.banner-icon svg {
  width: 28px;
  height: 28px;
  color: white;
}

.banner-text h2 {
  color: white;
  font-size: 1.25rem;
  margin-bottom: 0.25rem;
}

.banner-text p {
  color: rgba(255,255,255,0.85);
  font-size: 0.85rem;
}

.stats-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
  gap: 1.25rem;
  margin-bottom: 2rem;
}

.stat-card {
  background: white;
  border-radius: var(--radius-lg);
  padding: 1.25rem;
  display: flex;
  align-items: center;
  gap: 1rem;
  box-shadow: var(--shadow-sm);
  transition: transform 0.2s, box-shadow 0.2s;
}

.stat-card:hover {
  transform: translateY(-2px);
  box-shadow: var(--shadow-md);
}

.stat-icon {
  width: 48px;
  height: 48px;
  border-radius: var(--radius-md);
  display: flex;
  align-items: center;
  justify-content: center;
  flex-shrink: 0;
}

.stat-icon svg { width: 24px; height: 24px; }
.stat-icon--green { background: rgba(34, 197, 94, 0.15); color: #22C55E; }
.stat-icon--yellow { background: rgba(255, 193, 7, 0.15); color: #F59E0B; }
.stat-icon--blue { background: rgba(59, 130, 246, 0.15); color: #3B82F6; }
.stat-icon--red { background: rgba(239, 68, 68, 0.15); color: #EF4444; }

.stat-info { display: flex; flex-direction: column; }
.stat-value { font-size: 1.5rem; font-weight: 700; color: var(--nyc-navy); }
.stat-label { font-size: 0.8rem; color: var(--nyc-steel-light); }

.bridges-section {
  background: white;
  border-radius: var(--radius-lg);
  box-shadow: var(--shadow-sm);
  overflow: hidden;
}

.section-header {
  padding: 1.25rem 1.5rem;
  border-bottom: 1px solid var(--nyc-gray-200);
}

.section-header h3 {
  font-size: 1rem;
  font-weight: 700;
  color: var(--nyc-navy);
}

.bridges-table {
  overflow-x: auto;
}

table {
  width: 100%;
  border-collapse: collapse;
}

thead {
  background: var(--nyc-gray-100);
}

th {
  padding: 0.85rem 1.25rem;
  text-align: left;
  font-size: 0.75rem;
  font-weight: 700;
  color: var(--nyc-steel);
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

td {
  padding: 1rem 1.25rem;
  border-top: 1px solid var(--nyc-gray-200);
  font-size: 0.9rem;
  color: var(--nyc-navy);
}

tr:hover {
  background: var(--nyc-gray-100);
}

.bridge-name {
  display: flex;
  align-items: center;
  gap: 0.6rem;
  font-weight: 500;
}

.bridge-icon {
  width: 18px;
  height: 18px;
  color: var(--nyc-steel-light);
}

.status-badge {
  display: inline-block;
  padding: 0.3rem 0.7rem;
  border-radius: 9999px;
  font-size: 0.75rem;
  font-weight: 600;
}

.status--success {
  background: rgba(34, 197, 94, 0.15);
  color: #16A34A;
}

.status--warning {
  background: rgba(245, 158, 11, 0.15);
  color: #D97706;
}

.status--info {
  background: rgba(59, 130, 246, 0.15);
  color: #2563EB;
}

.level-badge {
  display: inline-block;
  padding: 0.3rem 0.7rem;
  border-radius: 9999px;
  font-size: 0.75rem;
  font-weight: 600;
}

.level--high {
  background: rgba(239, 68, 68, 0.15);
  color: #DC2626;
}

.level--medium {
  background: rgba(245, 158, 11, 0.15);
  color: #D97706;
}

.level--low {
  background: rgba(34, 197, 94, 0.15);
  color: #16A34A;
}

.position-badge {
  display: inline-flex;
  align-items: center;
  gap: 0.35rem;
  padding: 0.3rem 0.7rem;
  border-radius: 9999px;
  font-size: 0.75rem;
  font-weight: 600;
}

.position-badge svg {
  width: 14px;
  height: 14px;
}

.position--up {
  background: rgba(239, 68, 68, 0.15);
  color: #DC2626;
}

.position--down {
  background: rgba(34, 197, 94, 0.15);
  color: #16A34A;
}

.actions {
  display: flex;
  gap: 0.5rem;
}

.btn-action {
  width: 34px;
  height: 34px;
  border: none;
  border-radius: var(--radius-md);
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  transition: all 0.2s;
}

.btn-action svg {
  width: 18px;
  height: 18px;
}

.btn-toggle {
  background: rgba(255, 193, 7, 0.15);
  color: #D97706;
}

.btn-toggle:hover:not(:disabled) {
  background: rgba(255, 193, 7, 0.3);
}

.btn-toggle:disabled {
  opacity: 0.4;
  cursor: not-allowed;
}

.btn-view {
  background: rgba(59, 130, 246, 0.15);
  color: #3B82F6;
}

.btn-view:hover {
  background: rgba(59, 130, 246, 0.3);
}

.modal-overlay {
  position: fixed;
  inset: 0;
  background: rgba(10, 22, 40, 0.6);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 1000;
  padding: 1rem;
}

.modal {
  background: white;
  border-radius: var(--radius-lg);
  width: 100%;
  max-width: 480px;
  box-shadow: var(--shadow-xl);
}

.modal-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 1.25rem 1.5rem;
  border-bottom: 1px solid var(--nyc-gray-200);
}

.modal-header h3 {
  font-size: 1.1rem;
  font-weight: 700;
  color: var(--nyc-navy);
}

.modal-close {
  width: 32px;
  height: 32px;
  border: none;
  background: var(--nyc-gray-100);
  border-radius: var(--radius-md);
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  color: var(--nyc-steel);
  transition: all 0.2s;
}

.modal-close:hover {
  background: var(--nyc-gray-200);
  color: var(--nyc-navy);
}

.modal-close svg {
  width: 18px;
  height: 18px;
}

.modal-body {
  padding: 1.5rem;
  display: flex;
  flex-direction: column;
  gap: 1rem;
}

.detail-row {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.detail-label {
  font-size: 0.9rem;
  color: var(--nyc-steel);
}

.detail-value {
  font-size: 0.9rem;
  font-weight: 600;
  color: var(--nyc-navy);
}

.modal-footer {
  display: flex;
  justify-content: flex-end;
  gap: 0.75rem;
  padding: 1.25rem 1.5rem;
  border-top: 1px solid var(--nyc-gray-200);
}

.btn-modal {
  padding: 0.6rem 1.25rem;
  border-radius: var(--radius-md);
  font-size: 0.9rem;
  font-weight: 600;
  cursor: pointer;
  transition: all 0.2s;
  border: none;
}

.btn-secondary {
  background: var(--nyc-gray-100);
  color: var(--nyc-navy);
}

.btn-secondary:hover {
  background: var(--nyc-gray-200);
}

.btn-primary {
  background: linear-gradient(135deg, var(--nyc-taxi) 0%, var(--nyc-taxi-dark) 100%);
  color: var(--nyc-navy);
}

.btn-primary:hover:not(:disabled) {
  transform: translateY(-1px);
  box-shadow: 0 4px 12px rgba(255, 193, 7, 0.4);
}

.btn-primary:disabled {
  opacity: 0.6;
  cursor: not-allowed;
}
</style>
