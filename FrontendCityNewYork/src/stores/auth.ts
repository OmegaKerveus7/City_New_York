import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import type { Usuario, LoginRequest, CrearUsuarioRequest } from '@/types'
import { authService } from '@/api'

export const useAuthStore = defineStore('auth', () => {
  const token = ref<string | null>(null)
  const usuario = ref<Usuario | null>(null)

  const isAuthenticated = computed(() => !!token.value)

  const userRole = computed(() => usuario.value?.Rol ?? null)

  const userName = computed(() =>
    usuario.value
      ? `${usuario.value.NombreUsuario} ${usuario.value.ApellidosUsuario}`
      : null
  )

  function initializeFromStorage() {
    const storedToken = localStorage.getItem('token')
    const storedUsuario = localStorage.getItem('usuario')

    if (storedToken) {
      token.value = storedToken
    }

    if (storedUsuario) {
      try {
        usuario.value = JSON.parse(storedUsuario)
      } catch (e) {
        console.error('Error parsing stored usuario:', e)
        localStorage.removeItem('usuario')
      }
    }
  }

  function mapBackendUsuario(data: any): Usuario {
    return {
      Id: data.id || data.Id || data.ID_USUARIO || 0,
      NombreUsuario: data.nombreUsuario || data.NombreUsuario || data.NOMBRE_USUARIO || '',
      ApellidosUsuario: data.apellidosUsuario || data.ApellidosUsuario || data.APELLIDOS_USUARIO || '',
      Correo: data.correo || data.Correo || data.CORREO || '',
      Rol: data.rol || data.Rol || data.NOMBRE_ROL || ''
    }
  }

  async function login(credentials: LoginRequest) {
    try {
      const response = await authService.login(credentials)
      console.log('Login response completo:', response)
      console.log('Token:', response.token)
      console.log('Usuario raw:', response.usuario)

      if (response.token && response.usuario) {
        const mappedUsuario = mapBackendUsuario(response.usuario)

        token.value = response.token
        usuario.value = mappedUsuario

        localStorage.setItem('token', response.token)
        localStorage.setItem('usuario', JSON.stringify(mappedUsuario))

        console.log('Usuario mapeado:', mappedUsuario)
      } else {
        throw new Error('Respuesta del servidor incompleta')
      }
    } catch (error) {
      console.error('Login error en store:', error)
      logout()
      throw error
    }
  }

  async function register(data: CrearUsuarioRequest) {
    return await authService.register(data)
  }

  function logout() {
    token.value = null
    usuario.value = null
    localStorage.removeItem('token')
    localStorage.removeItem('usuario')
  }

  initializeFromStorage()

  return {
    token,
    usuario,
    isAuthenticated,
    userRole,
    userName,
    login,
    register,
    logout
  }
})
