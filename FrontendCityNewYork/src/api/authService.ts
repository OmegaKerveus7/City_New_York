import http from './http'
import type { LoginRequest, LoginResponse, CrearUsuarioRequest, ApiResponse, Rol } from '@/types'

export const authService = {
  async login(data: LoginRequest): Promise<LoginResponse> {
    try {
      const response = await http.post<any>('/auth/login', data)
      console.log('Raw response:', response)
      console.log('Response data:', response.data)
      return response.data as LoginResponse
    } catch (error: any) {
      console.error('Auth service error:', error)
      if (error.response?.data) {
        throw error.response.data
      }
      throw error
    }
  },

  async register(data: CrearUsuarioRequest): Promise<ApiResponse> {
    const response = await http.post<ApiResponse>('/auth/registrar', data)
    return response.data
  },

  async getRoles(): Promise<Rol[]> {
    const response = await http.get<ApiResponse<Rol[]>>('/auth/roles')
    return (response.data.data as Rol[]) || []
  },

  logout(): void {
    localStorage.removeItem('token')
    localStorage.removeItem('usuario')
  }
}
