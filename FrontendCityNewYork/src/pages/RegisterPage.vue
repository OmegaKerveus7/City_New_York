<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import { useAuthStore } from '@/stores'
import type { CrearUsuarioRequest, Rol } from '@/types'
import { authService } from '@/api'

const router = useRouter()
const authStore = useAuthStore()

const form = ref<CrearUsuarioRequest>({
  NombreUsuario: '',
  ApellidosUsuario: '',
  Correo: '',
  Contrasena: '',
  IdRol: 0
})

const confirmPass = ref('')
const roles = ref<Rol[]>([])
const error = ref('')
const success = ref('')
const loading = ref(false)

onMounted(async () => {
  try {
    roles.value = await authService.getRoles()
    if (roles.value.length > 0) {
      form.value.IdRol = roles.value[0].Id
    }
  } catch {
    error.value = 'No se pudieron cargar los roles'
  }
})

const handleRegister = async () => {
  const f = form.value
  if (!f.NombreUsuario || !f.ApellidosUsuario || !f.Correo || !f.Contrasena) {
    error.value = 'Por favor complete todos los campos'
    return
  }
  if (f.Contrasena !== confirmPass.value) {
    error.value = 'Las contraseñas no coinciden'
    return
  }
  if (f.Contrasena.length < 6) {
    error.value = 'La contraseña debe tener al menos 6 caracteres'
    return
  }

  error.value = ''
  success.value = ''
  loading.value = true

  try {
    await authStore.register(f)
    success.value = 'Cuenta creada correctamente. Redirigiendo...'
    setTimeout(() => router.push('/login'), 1500)
  } catch (err: any) {
    error.value = err.response?.data?.mensaje || 'Error al crear la cuenta'
  } finally {
    loading.value = false
  }
}
</script>

<template>
  <div class="register-page">
    <div class="register-panel">
      <div class="register-panel__image animate-slide-left">
        <img src="/logo_new_York.webp" alt="New York City" class="city-image" />
        <div class="register-panel__overlay">
          <div class="register-panel__content">
            <div class="brand-badge animate-fade-in">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="brand-icon">
                <path d="M3 21h18M9 8h1M9 12h1M9 16h1M14 8h1M14 12h1M14 16h1M5 21V5a2 2 0 012-2h10a2 2 0 012 2v16" />
              </svg>
            </div>
            <h1 class="panel-title">City of New York</h1>
            <p class="panel-subtitle">Únete al equipo de gestión municipal</p>
            <div class="feature-list">
              <div class="feature-item">
                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M22 11.08V12a10 10 0 11-5.93-9.14"/><polyline points="22 4 12 14.01 9 11.01"/></svg>
                <span>Acceso al panel de control</span>
              </div>
              <div class="feature-item">
                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M22 11.08V12a10 10 0 11-5.93-9.14"/><polyline points="22 4 12 14.01 9 11.01"/></svg>
                <span>Gestión de la ciudad en tiempo real</span>
              </div>
              <div class="feature-item">
                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M22 11.08V12a10 10 0 11-5.93-9.14"/><polyline points="22 4 12 14.01 9 11.01"/></svg>
                <span>Herramientas administrativas</span>
              </div>
            </div>
          </div>
        </div>
      </div>

      <div class="register-panel__form animate-slide-right">
        <div class="form-container">
          <div class="form-header">
            <img src="/logo_new_York.webp" alt="Logo" class="form-logo" />
            <h2>Crear Cuenta</h2>
            <p>Registre un nuevo usuario en el sistema</p>
          </div>

          <form @submit.prevent="handleRegister" class="register-form">
            <div v-if="error" class="error-alert animate-scale-in">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="error-icon">
                <circle cx="12" cy="12" r="10" /><line x1="12" y1="8" x2="12" y2="12" /><line x1="12" y1="16" x2="12.01" y2="16" />
              </svg>
              <span>{{ error }}</span>
            </div>

            <div v-if="success" class="success-alert animate-scale-in">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="success-icon">
                <path d="M22 11.08V12a10 10 0 11-5.93-9.14"/><polyline points="22 4 12 14.01 9 11.01"/>
              </svg>
              <span>{{ success }}</span>
            </div>

            <div class="field-row">
              <div class="field-group">
                <label>Nombre</label>
                <div class="field-input-wrapper">
                  <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="field-icon">
                    <path d="M20 21v-2a4 4 0 00-4-4H8a4 4 0 00-4 4v2" /><circle cx="12" cy="7" r="4" />
                  </svg>
                  <input v-model="form.NombreUsuario" type="text" placeholder="John" />
                </div>
              </div>
              <div class="field-group">
                <label>Apellidos</label>
                <div class="field-input-wrapper">
                  <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="field-icon">
                    <path d="M20 21v-2a4 4 0 00-4-4H8a4 4 0 00-4 4v2" /><circle cx="12" cy="7" r="4" />
                  </svg>
                  <input v-model="form.ApellidosUsuario" type="text" placeholder="Doe" />
                </div>
              </div>
            </div>

            <div class="field-group">
              <label>Correo Electrónico</label>
              <div class="field-input-wrapper">
                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="field-icon">
                  <path d="M4 4h16c1.1 0 2 .9 2 2v12c0 1.1-.9 2-2 2H4c-1.1 0-2-.9-2-2V6c0-1.1.9-2 2-2z" /><polyline points="22,6 12,13 2,6" />
                </svg>
                <input v-model="form.Correo" type="email" placeholder="correo@nyc.gov" />
              </div>
            </div>

            <div class="field-group">
              <label>Rol</label>
              <div class="field-input-wrapper">
                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="field-icon">
                  <path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z" />
                </svg>
                <select v-model="form.IdRol">
                  <option v-for="rol in roles" :key="rol.Id" :value="rol.Id">
                    {{ rol.Nombre }}
                  </option>
                </select>
              </div>
            </div>

            <div class="field-row">
              <div class="field-group">
                <label>Contraseña</label>
                <div class="field-input-wrapper">
                  <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="field-icon">
                    <rect x="3" y="11" width="18" height="11" rx="2" ry="2" /><path d="M7 11V7a5 5 0 0110 0v4" />
                  </svg>
                  <input v-model="form.Contrasena" type="password" placeholder="••••••••" />
                </div>
              </div>
              <div class="field-group">
                <label>Confirmar</label>
                <div class="field-input-wrapper">
                  <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="field-icon">
                    <rect x="3" y="11" width="18" height="11" rx="2" ry="2" /><path d="M7 11V7a5 5 0 0110 0v4" />
                  </svg>
                  <input v-model="confirmPass" type="password" placeholder="••••••••" />
                </div>
              </div>
            </div>

            <button type="submit" :disabled="loading" class="btn-primary">
              <span v-if="loading" class="spinner"></span>
              <span v-else>Crear Cuenta</span>
              <svg v-if="!loading" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="btn-arrow">
                <line x1="5" y1="12" x2="19" y2="12" /><polyline points="12 5 19 12 12 19" />
              </svg>
            </button>
          </form>

          <div class="form-footer">
            <p>¿Ya tienes cuenta?
              <router-link to="/login" class="link-accent">Iniciar Sesión</router-link>
            </p>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<style scoped>
.register-page {
  min-height: 100vh;
  display: flex;
  align-items: center;
  justify-content: center;
  background: linear-gradient(135deg, var(--nyc-navy) 0%, var(--nyc-navy-light) 100%);
  padding: 1.5rem;
}

.register-panel {
  display: flex;
  width: 100%;
  max-width: 1050px;
  border-radius: var(--radius-xl);
  overflow: hidden;
  box-shadow: var(--shadow-xl);
}

.register-panel__image {
  flex: 1;
  position: relative;
  display: none;
  overflow: hidden;
}

@media (min-width: 768px) {
  .register-panel__image { display: block; }
}

.city-image {
  width: 100%;
  height: 100%;
  object-fit: cover;
  display: block;
}

.register-panel__overlay {
  position: absolute;
  inset: 0;
  background: linear-gradient(180deg, rgba(10,22,40,0.3) 0%, rgba(10,22,40,0.85) 100%);
  display: flex;
  align-items: flex-end;
  padding: 2.5rem;
}

.register-panel__content { color: white; }

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
  color: white;
  margin-bottom: 0.25rem;
}

.panel-subtitle {
  color: var(--nyc-gray-300);
  font-size: 0.9rem;
  margin-bottom: 1.5rem;
}

.feature-list {
  display: flex;
  flex-direction: column;
  gap: 0.75rem;
}

.feature-item {
  display: flex;
  align-items: center;
  gap: 0.6rem;
  color: var(--nyc-gray-200);
  font-size: 0.85rem;
}

.feature-item svg {
  width: 18px;
  height: 18px;
  color: var(--nyc-taxi);
  flex-shrink: 0;
}

.register-panel__form {
  flex: 1;
  background: white;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 2rem;
}

.form-container {
  width: 100%;
  max-width: 420px;
}

.form-header {
  text-align: center;
  margin-bottom: 1.5rem;
}

.form-logo {
  width: 48px;
  height: 48px;
  object-fit: contain;
  margin-bottom: 0.75rem;
}

.form-header h2 {
  font-size: 1.4rem;
  font-weight: 700;
  color: var(--nyc-navy);
  margin-bottom: 0.25rem;
}

.form-header p {
  color: var(--nyc-steel-light);
  font-size: 0.85rem;
}

.register-form {
  display: flex;
  flex-direction: column;
  gap: 1rem;
}

.error-alert, .success-alert {
  display: flex;
  align-items: center;
  gap: 0.6rem;
  padding: 0.7rem 1rem;
  border-radius: var(--radius-md);
  font-size: 0.85rem;
}

.error-alert {
  background: #FFF5F5;
  border: 1px solid #FEB2B2;
  color: var(--nyc-danger);
}

.success-alert {
  background: #F0FFF4;
  border: 1px solid #9AE6B4;
  color: var(--nyc-success);
}

.error-icon, .success-icon { width: 18px; height: 18px; flex-shrink: 0; }

.field-row {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 0.75rem;
}

.field-group {
  display: flex;
  flex-direction: column;
  gap: 0.35rem;
}

.field-group label {
  font-size: 0.75rem;
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
  left: 10px;
  width: 16px;
  height: 16px;
  color: var(--nyc-steel-light);
  pointer-events: none;
}

.field-input-wrapper input,
.field-input-wrapper select {
  width: 100%;
  padding: 0.6rem 0.7rem 0.6rem 2.2rem;
  border: 2px solid var(--nyc-gray-200);
  border-radius: var(--radius-md);
  font-size: 0.9rem;
  color: var(--nyc-navy);
  background: var(--nyc-gray-100);
  transition: border-color 0.2s, background 0.2s;
}

.field-input-wrapper select {
  appearance: none;
  cursor: pointer;
}

.field-input-wrapper input:focus,
.field-input-wrapper select:focus {
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
  padding: 0.75rem 1.5rem;
  background: linear-gradient(135deg, var(--nyc-taxi) 0%, var(--nyc-taxi-dark) 100%);
  color: var(--nyc-navy);
  border: none;
  border-radius: var(--radius-md);
  font-size: 0.95rem;
  font-weight: 700;
  cursor: pointer;
  transition: transform 0.2s, box-shadow 0.2s;
  margin-top: 0.25rem;
}

.btn-primary:hover:not(:disabled) {
  transform: translateY(-1px);
  box-shadow: 0 4px 16px rgba(255, 193, 7, 0.4);
}

.btn-primary:disabled { opacity: 0.7; cursor: not-allowed; }
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
  margin-top: 1.25rem;
  padding-top: 1.25rem;
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
