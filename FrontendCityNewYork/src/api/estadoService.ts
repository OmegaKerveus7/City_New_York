import http from './http'

export interface EstadoComponente {
  Clave: string
  Modulo: string
  Valor: string | null
  ValorNumero: number | null
  Texto: string | null
  IpDispositivo: string | null
  FechaActualizacion: string
}

export interface EstadoCiudad {
  componentes: EstadoComponente[]
  total: number
  ultimaActualizacion: string | null
}

/**
 * Estado en vivo de la maquina.
 * Lo llena el ESP32 (POST /api/estado) y lo lee el dashboard.
 */
export const estadoService = {
  async obtenerTodos(): Promise<EstadoCiudad> {
    const response = await http.get<EstadoCiudad>('/estado/todos')
    return response.data
  },

  async obtener(clave: string): Promise<EstadoComponente> {
    const response = await http.get<EstadoComponente>(`/estado/${encodeURIComponent(clave)}`)
    return response.data
  }
}
