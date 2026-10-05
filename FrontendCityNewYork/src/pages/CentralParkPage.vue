<script setup lang="ts">
import DashboardLayout from '@/layouts/DashboardLayout.vue'
</script>

<template>
  <DashboardLayout>
    <div class="central-park">
      <div class="welcome-banner animate-fade-in-up">
        <div class="banner-icon">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <path d="M12 3v18M5 10l7-7 7 7M5 21h14" />
          </svg>
        </div>
        <div class="banner-text">
          <h2>Control de Central Park</h2>
          <p>Gestión integral del mantenimiento y seguridad del parque</p>
        </div>
      </div>

      <div class="stats-grid">
        <div class="stat-card animate-fade-in-up delay-1">
          <div class="stat-icon stat-icon--green">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z" />
            </svg>
          </div>
          <div class="stat-info">
            <span class="stat-value">843</span>
            <span class="stat-label">Árboles</span>
          </div>
        </div>

        <div class="stat-card animate-fade-in-up delay-2">
          <div class="stat-icon stat-icon--blue">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <path d="M14.7 6.3a1 1 0 000 1.4l1.6 1.6a1 1 0 001.4 0l3.77-3.77a6 6 0 01-7.94 7.94l-6.91 6.91a2.12 2.12 0 01-3-3l6.91-6.91a6 6 0 017.94-7.94l-3.76 3.76z" />
            </svg>
          </div>
          <div class="stat-info">
            <span class="stat-value">127</span>
            <span class="stat-label">Zonas</span>
          </div>
        </div>

        <div class="stat-card animate-fade-in-up delay-3">
          <div class="stat-icon stat-icon--yellow">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <path d="M17 21v-2a4 4 0 00-4-4H5a4 4 0 00-4 4v2" />
              <circle cx="9" cy="7" r="4" />
              <path d="M23 21v-2a4 4 0 00-3-3.87" />
              <path d="M16 3.13a4 4 0 010 7.75" />
            </svg>
          </div>
          <div class="stat-info">
            <span class="stat-value">42</span>
            <span class="stat-label">Guardias</span>
          </div>
        </div>

        <div class="stat-card animate-fade-in-up delay-4">
          <div class="stat-icon stat-icon--red">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <circle cx="12" cy="12" r="10" />
              <polyline points="12 6 12 12 16 14" />
            </svg>
          </div>
          <div class="stat-info">
            <span class="stat-value">24/7</span>
            <span class="stat-label">Vigilancia</span>
          </div>
        </div>
      </div>

      <div class="content-grid">
        <div class="content-card animate-fade-in-up delay-2">
          <div class="card-header">
            <h3>Mantenimiento Reciente</h3>
            <button class="btn-secondary">Ver Todo</button>
          </div>
          <div class="maintenance-list">
            <div class="maintenance-item">
              <div class="maintenance-status maintenance-status--success"></div>
              <div class="maintenance-content">
                <span class="maintenance-title">Poda de árboles - Zona Norte</span>
                <span class="maintenance-meta">Completado • Hace 2 horas</span>
              </div>
            </div>
            <div class="maintenance-item">
              <div class="maintenance-status maintenance-status--warning"></div>
              <div class="maintenance-content">
                <span class="maintenance-title">Reparación de sendero - Zona Este</span>
                <span class="maintenance-meta">En progreso • Desde 4 horas</span>
              </div>
            </div>
            <div class="maintenance-item">
              <div class="maintenance-status maintenance-status--info"></div>
              <div class="maintenance-content">
                <span class="maintenance-title">Limpieza de lago - Belvedere</span>
                <span class="maintenance-meta">Programado • Mañana 6:00 AM</span>
              </div>
            </div>
            <div class="maintenance-item">
              <div class="maintenance-status maintenance-status--danger"></div>
              <div class="maintenance-content">
                <span class="maintenance-title">Inspección de juegos - Infantil</span>
                <span class="maintenance-meta">Pendiente • 超期</span>
              </div>
            </div>
          </div>
        </div>

        <div class="content-card animate-fade-in-up delay-3">
          <div class="card-header">
            <h3>Estado de Zonas</h3>
          </div>
          <div class="zones-list">
            <div class="zone-item">
              <span class="zone-name">Zona Norte</span>
              <span class="zone-status zone-status--success">Óptimo</span>
            </div>
            <div class="zone-item">
              <span class="zone-name">Zona Sur</span>
              <span class="zone-status zone-status--success">Óptimo</span>
            </div>
            <div class="zone-item">
              <span class="zone-name">Zona Este</span>
              <span class="zone-status zone-status--warning">En mantenimiento</span>
            </div>
            <div class="zone-item">
              <span class="zone-name">Zona Oeste</span>
              <span class="zone-status zone-status--success">Óptimo</span>
            </div>
            <div class="zone-item">
              <span class="zone-name">Centro</span>
              <span class="zone-status zone-status--success">Óptimo</span>
            </div>
          </div>
        </div>
      </div>
    </div>
  </DashboardLayout>
</template>

<style scoped>
.central-park {
  max-width: 1400px;
}

.welcome-banner {
  background: linear-gradient(135deg, #059669 0%, #047857 100%);
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
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
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
.stat-icon--blue { background: rgba(59, 130, 246, 0.15); color: #3B82F6; }
.stat-icon--yellow { background: rgba(255, 193, 7, 0.15); color: #F59E0B; }
.stat-icon--red { background: rgba(239, 68, 68, 0.15); color: #EF4444; }

.stat-info { display: flex; flex-direction: column; }
.stat-value { font-size: 1.5rem; font-weight: 700; color: var(--nyc-navy); }
.stat-label { font-size: 0.8rem; color: var(--nyc-steel-light); }

.content-grid {
  display: grid;
  grid-template-columns: 1.5fr 1fr;
  gap: 1.5rem;
}

@media (max-width: 1024px) {
  .content-grid { grid-template-columns: 1fr; }
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

.btn-secondary {
  background: var(--nyc-gray-100);
  color: var(--nyc-navy);
  border: none;
  padding: 0.5rem 1rem;
  border-radius: var(--radius-md);
  font-size: 0.8rem;
  font-weight: 600;
  cursor: pointer;
  transition: background 0.2s;
}

.btn-secondary:hover {
  background: var(--nyc-gray-200);
}

.maintenance-list {
  display: flex;
  flex-direction: column;
  gap: 1rem;
}

.maintenance-item {
  display: flex;
  gap: 0.85rem;
  align-items: flex-start;
  padding: 0.75rem;
  background: var(--nyc-gray-100);
  border-radius: var(--radius-md);
}

.maintenance-status {
  width: 10px;
  height: 10px;
  border-radius: 50%;
  margin-top: 0.35rem;
  flex-shrink: 0;
}

.maintenance-status--success { background: #22C55E; }
.maintenance-status--warning { background: #F59E0B; }
.maintenance-status--info { background: #3B82F6; }
.maintenance-status--danger { background: #EF4444; }

.maintenance-content {
  display: flex;
  flex-direction: column;
  gap: 0.15rem;
}

.maintenance-title {
  font-size: 0.85rem;
  color: var(--nyc-navy);
  font-weight: 500;
}

.maintenance-meta {
  font-size: 0.75rem;
  color: var(--nyc-steel-light);
}

.zones-list {
  display: flex;
  flex-direction: column;
  gap: 0.75rem;
}

.zone-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 0.75rem;
  background: var(--nyc-gray-100);
  border-radius: var(--radius-md);
}

.zone-name {
  font-size: 0.85rem;
  color: var(--nyc-navy);
  font-weight: 500;
}

.zone-status {
  font-size: 0.75rem;
  font-weight: 600;
  padding: 0.25rem 0.6rem;
  border-radius: 9999px;
}

.zone-status--success {
  background: rgba(34, 197, 94, 0.15);
  color: #16A34A;
}

.zone-status--warning {
  background: rgba(245, 158, 11, 0.15);
  color: #D97706;
}
</style>
