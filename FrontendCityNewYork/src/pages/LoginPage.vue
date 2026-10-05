<script setup lang="ts">
import { ref } from 'vue'
import { useRouter } from 'vue-router'
import { useAuthStore } from '@/stores'
import type { LoginRequest } from '@/types'

const router = useRouter()
const authStore = useAuthStore()

const credentials = ref<LoginRequest>({
  Correo: '',
  Contrasena: ''
})

const error = ref('')
const loading = ref(false)
const backendError = ref(false)

const handleLogin = async () => {
  if (!credentials.value.Correo || !credentials.value.Contrasena) {
    error.value = 'Por favor complete todos los campos'
    return
  }

  error.value = ''
  backendError.value = false
  loading.value = true

  try {
    await authStore.login(credentials.value)
    console.log('Login successful, redirecting...')
    console.log('Token:', authStore.token)
    console.log('Usuario:', authStore.usuario)
    router.push('/')
  } catch (err: any) {
    console.error('Login error:', err)

    if (err.code === 'ERR_NETWORK' || err.message?.includes('Network') || !err.response) {
      backendError.value = true
      error.value = 'No se pudo conectar con el servidor. Verifique que el backend esté activo.'
    } else {
      backendError.value = false
      error.value = err.response?.data?.mensaje || err.response?.data || 'Credenciales incorrectas. Intente nuevamente.'
    }
  } finally {
    loading.value = false
  }
}
</script>

<template>
  <div class="login-page">
    <div class="login-panel">
      <div class="login-panel__image animate-slide-left">
        <div class="nyc-background">
          <div class="skyline">
            <svg viewBox="0 0 400 200" preserveAspectRatio="xMidYMax slice">
              <defs>
                <linearGradient id="buildingGrad" x1="0%" y1="0%" x2="0%" y2="100%">
                  <stop offset="0%" style="stop-color:#1a2d4a;stop-opacity:1" />
                  <stop offset="100%" style="stop-color:#0f1f35;stop-opacity:1" />
                </linearGradient>
                <linearGradient id="windowGrad" x1="0%" y1="0%" x2="0%" y2="100%">
                  <stop offset="0%" style="stop-color:#FFD54F;stop-opacity:0.9" />
                  <stop offset="100%" style="stop-color:#FFC107;stop-opacity:0.7" />
                </linearGradient>
              </defs>
              <rect x="10" y="80" width="25" height="120" fill="url(#buildingGrad)" />
              <rect x="40" y="50" width="30" height="150" fill="url(#buildingGrad)" />
              <rect x="75" y="100" width="20" height="100" fill="url(#buildingGrad)" />
              <rect x="100" y="30" width="35" height="170" fill="url(#buildingGrad)" />
              <rect x="140" y="90" width="25" height="110" fill="url(#buildingGrad)" />
              <rect x="170" y="60" width="30" height="140" fill="url(#buildingGrad)" />
              <rect x="205" y="40" width="40" height="160" fill="url(#buildingGrad)" />
              <rect x="250" y="80" width="22" height="120" fill="url(#buildingGrad)" />
              <rect x="277" y="55" width="32" height="145" fill="url(#buildingGrad)" />
              <rect x="314" y="70" width="28" height="130" fill="url(#buildingGrad)" />
              <rect x="347" y="45" width="35" height="155" fill="url(#buildingGrad)" />
              <rect x="0" y="195" width="400" height="10" fill="#0A1628" />
            </svg>
          </div>

          <div class="taxi-icon">
            <svg viewBox="0 0 64 32" fill="none">
              <rect x="4" y="8" width="56" height="16" rx="4" fill="#FFC107" />
              <rect x="8" y="4" width="48" height="12" rx="3" fill="#FFC107" />
              <rect x="12" y="12" width="16" height="8" rx="1" fill="#1a1a2e" />
              <rect x="36" y="12" width="16" height="8" rx="1" fill="#1a1a2e" />
              <circle cx="16" cy="24" r="5" fill="#2d3748" />
              <circle cx="48" cy="24" r="5" fill="#2d3748" />
              <circle cx="16" cy="24" r="2" fill="#718096" />
              <circle cx="48" cy="24" r="2" fill="#718096" />
              <rect x="28" y="6" width="8" height="4" rx="1" fill="#C53030" />
            </svg>
          </div>

          <div class="stars">
            <div class="star star-1"></div>
            <div class="star star-2"></div>
            <div class="star star-3"></div>
            <div class="star star-4"></div>
            <div class="star star-5"></div>
          </div>
        </div>

        <div class="login-panel__overlay">
          <div class="login-panel__content">
            <div class="brand-badge animate-fade-in">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="brand-icon">
                <path d="M3 21h18M9 8h1M9 12h1M9 16h1M14 8h1M14 12h1M14 16h1M5 21V5a2 2 0 012-2h10a2 2 0 012 2v16" />
              </svg>
            </div>
            <h1 class="panel-title">City of New York</h1>
            <p class="panel-subtitle">Sistema de Gestión y Control Municipal</p>
            <div class="panel-stats">
              <div class="stat-item">
                <span class="stat-value">5</span>
                <span class="stat-label">Boroughs</span>
              </div>
              <div class="stat-item">
                <span class="stat-value">8.3M</span>
                <span class="stat-label">Habitantes</span>
              </div>
              <div class="stat-item">
                <span class="stat-value">24/7</span>
                <span class="stat-label">Servicio</span>
              </div>
            </div>
          </div>
        </div>
      </div>

      <div class="login-panel__form animate-slide-right">
        <div class="form-container">
          <div class="form-header">
            <img src="/logo_new_York.webp" alt="Logo" class="form-logo" />
            <h2>Bienvenido</h2>
            <p>Inicie sesión para acceder al panel de control</p>
          </div>

          <form @submit.prevent="handleLogin" class="login-form">
            <div v-if="error" class="error-alert animate-scale-in">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="error-icon">
                <circle cx="12" cy="12" r="10" />
                <line x1="12" y1="8" x2="12" y2="12" />
                <line x1="12" y1="16" x2="12.01" y2="16" />
              </svg>
              <span>{{ error }}</span>
            </div>

            <div v-if="backendError" class="backend-error animate-scale-in">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="error-icon">
                <path d="M10.29 3.86L1.82 18a2 2 0 001.71 3h16.94a2 2 0 001.71-3L13.71 3.86a2 2 0 00-3.42 0z" />
                <line x1="12" y1="9" x2="12" y2="13" />
                <line x1="12" y1="17" x2="12.01" y2="17" />
              </svg>
              <div class="backend-error-content">
                <strong>Backend desconectado</strong>
                <p>Asegúrese de que el servidor .NET esté corriendo en http://localhost:5238</p>
              </div>
            </div>

            <div class="field-group">
              <label for="correo">Correo Electrónico</label>
              <div class="field-input-wrapper">
                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="field-icon">
                  <path d="M4 4h16c1.1 0 2 .9 2 2v12c0 1.1-.9 2-2 2H4c-1.1 0-2-.9-2-2V6c0-1.1.9-2 2-2z" />
                  <polyline points="22,6 12,13 2,6" />
                </svg>
                <input
                  id="correo"
                  v-model="credentials.Correo"
                  type="email"
                  placeholder="correo@nyc.gov"
                  autocomplete="email"
                />
              </div>
            </div>

            <div class="field-group">
              <label for="contrasena">Contraseña</label>
              <div class="field-input-wrapper">
                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="field-icon">
                  <rect x="3" y="11" width="18" height="11" rx="2" ry="2" />
                  <path d="M7 11V7a5 5 0 0110 0v4" />
                </svg>
                <input
                  id="contrasena"
                  v-model="credentials.Contrasena"
                  type="password"
                  placeholder="••••••••"
                  autocomplete="current-password"
                />
              </div>
            </div>

            <button type="submit" :disabled="loading" class="btn-primary">
              <span v-if="loading" class="spinner"></span>
              <span v-else>Iniciar Sesión</span>
              <svg v-if="!loading" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="btn-arrow">
                <line x1="5" y1="12" x2="19" y2="12" />
                <polyline points="12 5 19 12 12 19" />
              </svg>
            </button>
          </form>

          <div class="form-footer">
            <p>¿No tienes cuenta?
              <router-link to="/register" class="link-accent">Crear cuenta nueva</router-link>
            </p>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<style scoped>
.login-page {
  min-height: 100vh;
  display: flex;
  align-items: center;
  justify-content: center;
  background: linear-gradient(135deg, var(--nyc-navy) 0%, var(--nyc-navy-light) 100%);
  padding: 1.5rem;
}

.login-panel {
  display: flex;
  width: 100%;
  max-width: 1000px;
  min-height: 580px;
  border-radius: var(--radius-xl);
  overflow: hidden;
  box-shadow: var(--shadow-xl);
}

.login-panel__image {
  flex: 1;
  position: relative;
  display: none;
  overflow: hidden;
  background: linear-gradient(180deg, #1a2d4a 0%, #0f1f35 40%, #0A1628 100%);
}

@media (min-width: 768px) {
  .login-panel__image { display: block; }
}

.nyc-background {
  position: absolute;
  inset: 0;
  overflow: hidden;
}

.skyline {
  position: absolute;
  bottom: 0;
  left: 0;
  right: 0;
  height: 200px;
}

.skyline svg { width: 100%; height: 100%; }

.taxi-icon {
  position: absolute;
  bottom: 15px;
  right: 20px;
  width: 64px;
  animation: taxiDrive 8s ease-in-out infinite;
}

@keyframes taxiDrive {
  0%, 100% { transform: translateX(-20px); }
  50% { transform: translateX(10px); }
}

.stars {
  position: absolute;
  top: 20px;
  left: 0;
  right: 0;
  height: 60px;
}

.star {
  position: absolute;
  width: 3px;
  height: 3px;
  background: white;
  border-radius: 50%;
  animation: twinkle 2s ease-in-out infinite;
}

.star-1 { top: 10px; left: 15%; animation-delay: 0s; }
.star-2 { top: 25px; left: 35%; animation-delay: 0.4s; }
.star-3 { top: 8px; left: 55%; animation-delay: 0.8s; }
.star-4 { top: 20px; left: 75%; animation-delay: 1.2s; }
.star-5 { top: 30px; left: 90%; animation-delay: 1.6s; }

@keyframes twinkle {
  0%, 100% { opacity: 0.3; }
  50% { opacity: 1; }
}

.login-panel__overlay {
  position: absolute;
  inset: 0;
  background: linear-gradient(180deg, rgba(10,22,40,0.2) 0%, rgba(10,22,40,0.6) 50%, rgba(10,22,40,0.9) 100%);
  display: flex;
  align-items: flex-end;
  padding: 2.5rem;
}

.login-panel__content { color: white; }

.brand-badge {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  width: 48px;
  height: 48px;
  background: var(--nyc-taxi);
  color: var(--nyc-navy);
  border-radius: var(--radius-md);
  margin-bottom: 1.5rem;
}

.brand-icon { width: 24px; height: 24px; }

.panel-title {
  font-size: 1.75rem;
  font-weight: 700;
  margin-bottom: 0.25rem;
  color: white;
  letter-spacing: -0.5px;
}

.panel-subtitle {
  color: var(--nyc-gray-300);
  font-size: 0.9rem;
  margin-bottom: 2rem;
}

.panel-stats { display: flex; gap: 1.5rem; }
.stat-item { display: flex; flex-direction: column; }
.stat-value { font-size: 1.5rem; font-weight: 700; color: var(--nyc-taxi); }
.stat-label {
  font-size: 0.75rem;
  color: var(--nyc-gray-300);
  text-transform: uppercase;
  letter-spacing: 1px;
}

.login-panel__form {
  flex: 1;
  background: white;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 2rem;
}

.form-container {
  width: 100%;
  max-width: 360px;
}

.form-header {
  text-align: center;
  margin-bottom: 2rem;
}

.form-logo {
  width: 56px;
  height: 56px;
  object-fit: contain;
  margin-bottom: 1rem;
}

.form-header h2 {
  font-size: 1.5rem;
  font-weight: 700;
  color: var(--nyc-navy);
  margin-bottom: 0.25rem;
}

.form-header p {
  color: var(--nyc-steel-light);
  font-size: 0.9rem;
}

.login-form {
  display: flex;
  flex-direction: column;
  gap: 1.25rem;
}

.error-alert, .backend-error {
  display: flex;
  align-items: flex-start;
  gap: 0.6rem;
  padding: 0.75rem 1rem;
  border-radius: var(--radius-md);
  font-size: 0.85rem;
}

.error-alert {
  background: #FFF5F5;
  border: 1px solid #FEB2B2;
  color: var(--nyc-danger);
}

.backend-error {
  background: #FFFBEB;
  border: 1px solid #FCD34D;
  color: #92400E;
}

.backend-error-content {
  display: flex;
  flex-direction: column;
  gap: 0.25rem;
}

.backend-error-content strong {
  display: block;
}

.backend-error-content p {
  font-size: 0.8rem;
  margin: 0;
}

.error-icon {
  width: 18px;
  height: 18px;
  flex-shrink: 0;
  margin-top: 0.1rem;
}

.field-group {
  display: flex;
  flex-direction: column;
  gap: 0.4rem;
}

.field-group label {
  font-size: 0.8rem;
  font-weight: 600;
  color: var(--nyc-bridge);
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

.field-input-wrapper {
  position: relative;
  display: flex;
  align-items: center;
}

.field-icon {
  position: absolute;
  left: 12px;
  width: 18px;
  height: 18px;
  color: var(--nyc-steel-light);
  pointer-events: none;
}

.field-input-wrapper input {
  width: 100%;
  padding: 0.7rem 0.75rem 0.7rem 2.5rem;
  border: 2px solid var(--nyc-gray-200);
  border-radius: var(--radius-md);
  font-size: 0.95rem;
  color: var(--nyc-navy);
  background: var(--nyc-gray-100);
  transition: border-color 0.2s, background 0.2s;
}

.field-input-wrapper input:focus {
  outline: none;
  border-color: var(--nyc-taxi);
  background: white;
}

.btn-primary {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 0.5rem;
  width: 100%;
  padding: 0.8rem 1.5rem;
  background: linear-gradient(135deg, var(--nyc-taxi) 0%, var(--nyc-taxi-dark) 100%);
  color: var(--nyc-navy);
  border: none;
  border-radius: var(--radius-md);
  font-size: 0.95rem;
  font-weight: 700;
  cursor: pointer;
  transition: transform 0.2s, box-shadow 0.2s;
  margin-top: 0.5rem;
}

.btn-primary:hover:not(:disabled) {
  transform: translateY(-1px);
  box-shadow: 0 4px 16px rgba(255, 193, 7, 0.4);
}

.btn-primary:disabled {
  opacity: 0.7;
  cursor: not-allowed;
}

.btn-arrow { width: 18px; height: 18px; }

.spinner {
  width: 20px;
  height: 20px;
  border: 3px solid rgba(10,22,40,0.3);
  border-top-color: var(--nyc-navy);
  border-radius: 50%;
  animation: spin 0.6s linear infinite;
}

@keyframes spin { to { transform: rotate(360deg); } }

.form-footer {
  text-align: center;
  margin-top: 1.5rem;
  padding-top: 1.5rem;
  border-top: 1px solid var(--nyc-gray-200);
}

.form-footer p { font-size: 0.85rem; color: var(--nyc-steel); }

.link-accent {
  color: var(--nyc-navy);
  font-weight: 600;
  text-decoration: none;
  border-bottom: 2px solid var(--nyc-taxi);
  padding-bottom: 1px;
  transition: color 0.2s;
}

.link-accent:hover { color: var(--nyc-taxi-dark); }
</style>
