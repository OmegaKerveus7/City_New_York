<script setup lang="ts">
import DashboardLayout from '@/layouts/DashboardLayout.vue'
import { useAuthStore } from '@/stores'

const authStore = useAuthStore()
</script>

<template>
  <DashboardLayout>
    <div class="city-dashboard">
      <div class="welcome-banner animate-fade-in-up">
        <div class="welcome-text">
          <h2>¡Bienvenido, {{ authStore.usuario?.NombreUsuario }}!</h2>
          <p>Has iniciado sesión correctamente en el sistema de control municipal.</p>
        </div>
        <div class="welcome-badge">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <path d="M22 11.08V12a10 10 0 11-5.93-9.14" />
            <polyline points="22 4 12 14.01 9 11.01" />
          </svg>
        </div>
      </div>

      <div class="stats-grid">
        <div class="stat-card animate-fade-in-up delay-1">
          <div class="stat-icon stat-icon--yellow">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <path d="M17 21v-2a4 4 0 00-4-4H5a4 4 0 00-4 4v2" />
              <circle cx="9" cy="7" r="4" />
              <path d="M23 21v-2a4 4 0 00-3-3.87" />
              <path d="M16 3.13a4 4 0 010 7.75" />
            </svg>
          </div>
          <div class="stat-info">
            <span class="stat-value">1,248</span>
            <span class="stat-label">Empleados</span>
          </div>
        </div>

        <div class="stat-card animate-fade-in-up delay-2">
          <div class="stat-icon stat-icon--blue">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <path d="M3 9l9-7 9 7v11a2 2 0 01-2 2H5a2 2 0 01-2-2z" />
              <polyline points="9 22 9 12 15 12 15 22" />
            </svg>
          </div>
          <div class="stat-info">
            <span class="stat-value">59</span>
            <span class="stat-label">Sectores</span>
          </div>
        </div>

        <div class="stat-card animate-fade-in-up delay-3">
          <div class="stat-icon stat-icon--red">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <path d="M22 12h-4l-3 9L9 3l-3 9H2" />
            </svg>
          </div>
          <div class="stat-info">
            <span class="stat-value">847</span>
            <span class="stat-label">Incidentes</span>
          </div>
        </div>

        <div class="stat-card animate-fade-in-up delay-4">
          <div class="stat-icon stat-icon--green">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <polyline points="22 12 18 12 15 21 9 3 6 12 2 12" />
            </svg>
          </div>
          <div class="stat-info">
            <span class="stat-value">98%</span>
            <span class="stat-label">Eficiencia</span>
          </div>
        </div>
      </div>

      <div class="content-grid">
        <div class="content-card animate-fade-in-up delay-2">
          <div class="card-header">
            <h3>Actividad Reciente</h3>
            <span class="badge badge--success">En línea</span>
          </div>
          <div class="activity-list">
            <div class="activity-item">
              <div class="activity-dot activity-dot--success"></div>
              <div class="activity-content">
                <span class="activity-title">Reporte de servicios completado</span>
                <span class="activity-time">Hace 5 minutos</span>
              </div>
            </div>
            <div class="activity-item">
              <div class="activity-dot activity-dot--warning"></div>
              <div class="activity-content">
                <span class="activity-title">Nuevo incidente reportado en Manhattan</span>
                <span class="activity-time">Hace 12 minutos</span>
              </div>
            </div>
            <div class="activity-item">
              <div class="activity-dot activity-dot--info"></div>
              <div class="activity-content">
                <span class="activity-title">Actualización de sistema completada</span>
                <span class="activity-time">Hace 1 hora</span>
              </div>
            </div>
            <div class="activity-item">
              <div class="activity-dot activity-dot--danger"></div>
              <div class="activity-content">
                <span class="activity-title">Alerta de calidad del aire</span>
                <span class="activity-time">Hace 2 horas</span>
              </div>
            </div>
          </div>
        </div>

        <div class="content-card animate-fade-in-up delay-3">
          <div class="card-header">
            <h3>Estado del Sistema</h3>
          </div>
          <div class="system-status">
            <div class="status-item">
              <div class="status-indicator status-indicator--online"></div>
              <span class="status-label">API Backend</span>
              <span class="status-value">Operativo</span>
            </div>
            <div class="status-item">
              <div class="status-indicator status-indicator--online"></div>
              <span class="status-label">Base de Datos</span>
              <span class="status-value">Conectada</span>
            </div>
            <div class="status-item">
              <div class="status-indicator status-indicator--online"></div>
              <span class="status-label">Autenticación</span>
              <span class="status-value">Activa</span>
            </div>
            <div class="status-item">
              <div class="status-indicator status-indicator--warning"></div>
              <span class="status-label">Servidor de Reportes</span>
              <span class="status-value">Mantenimiento</span>
            </div>
          </div>
        </div>
      </div>
    </div>
  </DashboardLayout>
</template>

<style scoped>
.city-dashboard {
  max-width: 1400px;
}

.welcome-banner {
  background: linear-gradient(135deg, var(--nyc-navy) 0%, var(--nyc-navy-light) 100%);
  border-radius: var(--radius-lg);
  padding: 1.5rem 2rem;
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 2rem;
  box-shadow: var(--shadow-lg);
}

.welcome-text h2 {
  color: white;
  font-size: 1.25rem;
  margin-bottom: 0.35rem;
}

.welcome-text p {
  color: var(--nyc-gray-300);
  font-size: 0.85rem;
}

.welcome-badge {
  width: 52px;
  height: 52px;
  background: var(--nyc-taxi);
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  animation: borderGlow 2s infinite;
}

.welcome-badge svg {
  width: 26px;
  height: 26px;
  color: var(--nyc-navy);
}

.stats-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
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

.stat-icon svg {
  width: 24px;
  height: 24px;
}

.stat-icon--yellow {
  background: rgba(255, 193, 7, 0.15);
  color: #F59E0B;
}

.stat-icon--blue {
  background: rgba(59, 130, 246, 0.15);
  color: #3B82F6;
}

.stat-icon--red {
  background: rgba(239, 68, 68, 0.15);
  color: #EF4444;
}

.stat-icon--green {
  background: rgba(34, 197, 94, 0.15);
  color: #22C55E;
}

.stat-info {
  display: flex;
  flex-direction: column;
}

.stat-value {
  font-size: 1.5rem;
  font-weight: 700;
  color: var(--nyc-navy);
}

.stat-label {
  font-size: 0.8rem;
  color: var(--nyc-steel-light);
}

.content-grid {
  display: grid;
  grid-template-columns: 1.5fr 1fr;
  gap: 1.5rem;
}

@media (max-width: 1024px) {
  .content-grid {
    grid-template-columns: 1fr;
  }
}

.content-card {
  background: white;
  border-radius: var(--radius-lg);
  padding: 1.5rem;
  box-shadow: var(--shadow-sm);
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 1.25rem;
}

.card-header h3 {
  font-size: 1rem;
  font-weight: 700;
  color: var(--nyc-navy);
}

.badge {
  font-size: 0.7rem;
  font-weight: 600;
  padding: 0.25rem 0.6rem;
  border-radius: 9999px;
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

.badge--success {
  background: rgba(34, 197, 94, 0.15);
  color: #16A34A;
}

.activity-list {
  display: flex;
  flex-direction: column;
  gap: 1rem;
}

.activity-item {
  display: flex;
  gap: 0.85rem;
  align-items: flex-start;
}

.activity-dot {
  width: 10px;
  height: 10px;
  border-radius: 50%;
  margin-top: 0.35rem;
  flex-shrink: 0;
}

.activity-dot--success { background: #22C55E; }
.activity-dot--warning { background: #F59E0B; }
.activity-dot--info { background: #3B82F6; }
.activity-dot--danger { background: #EF4444; }

.activity-content {
  display: flex;
  flex-direction: column;
  gap: 0.15rem;
}

.activity-title {
  font-size: 0.85rem;
  color: var(--nyc-navy);
}

.activity-time {
  font-size: 0.75rem;
  color: var(--nyc-steel-light);
}

.system-status {
  display: flex;
  flex-direction: column;
  gap: 0.85rem;
}

.status-item {
  display: flex;
  align-items: center;
  gap: 0.75rem;
}

.status-indicator {
  width: 10px;
  height: 10px;
  border-radius: 50%;
  flex-shrink: 0;
}

.status-indicator--online { background: #22C55E; }
.status-indicator--warning { background: #F59E0B; }
.status-indicator--offline { background: #EF4444; }

.status-label {
  flex: 1;
  font-size: 0.85rem;
  color: var(--nyc-navy);
}

.status-value {
  font-size: 0.8rem;
  font-weight: 600;
  color: var(--nyc-steel-light);
}
</style>
