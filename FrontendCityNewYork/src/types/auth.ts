export interface LoginRequest {
  Correo: string
  Contrasena: string
}

export interface CrearUsuarioRequest {
  NombreUsuario: string
  ApellidosUsuario: string
  Correo: string
  Contrasena: string
  IdRol: number
}

export interface Usuario {
  Id: number
  NombreUsuario: string
  ApellidosUsuario: string
  Correo: string
  Rol: string
}

export interface Rol {
  Id: number
  Nombre: string
}

export interface LoginResponse {
  codigo: number
  mensaje: string
  token: string
  usuario: Usuario
}

export interface ApiResponse<T = unknown> {
  codigo: number
  mensaje: string
  data: T | null
}
