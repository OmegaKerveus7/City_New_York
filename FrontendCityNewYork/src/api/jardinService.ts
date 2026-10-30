import http from './http'
import { EstadoComponente } from './estadoService'

export interface JardinEstado {
  componentes: EstadoComponente[]
  bomba: string | null
  agua: string | null
  luces: string | null
  total: number
}

export interface JardinLogItem {
  Id: number
  Instruccion: string
  Resultado: string
  Valor: number | null
  Nivel: number | null
  Tipo: string
  Fecha: string
}

/**
 * Control del Central Park: fuente y luces del jardin (esclavo I2C 0x0A).
 */
export const jardinService = {
  async encenderFuente(): Promise<any> {
    const response = await http.post('/jardin/fuente/encender')
    return response.data
  },

  async apagarFuente(): Promise<any> {
    const response = await http.post('/jardin/fuente/apagar')
    return response.data
  },

  async pulsoFuente(segundos: number = 10): Promise<any> {
    const response = await http.post('/jardin/fuente/pulso', segundos)
    return response.data
  },

  async statusFuente(): Promise<any> {
    const response = await http.post('/jardin/fuente/status')
    return response.data
  },

  async encenderLuces(): Promise<any> {
    const response = await http.post('/jardin/luces/encender')
    return response.data
  },

  async apagarLuces(): Promise<any> {
    const response = await http.post('/jardin/luces/apagar')
    return response.data
  },

  async obtenerEstado(): Promise<JardinEstado> {
    const response = await http.get<JardinEstado>('/jardin/estado')
    return response.data
  },

  async obtenerLog(limite: number = 20): Promise<JardinLogItem[]> {
    const response = await http.get<JardinLogItem[]>('/jardin/log', { params: { limite } })
    return response.data
  }
}
