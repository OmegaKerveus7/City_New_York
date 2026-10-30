import http from './http'
import { EstadoComponente } from './estadoService'

export type CarrilSemaforo = 'NS' | 'EO'
export type ColorSemaforo = 'ROJO' | 'AMARILLO' | 'VERDE'

export interface SemaforoEstado {
  componentes: EstadoComponente[]
  ns: string | null
  eo: string | null
  modo: string | null
  total: number
}

export interface SemaforoLogItem {
  Id: number
  Instruccion: string
  Resultado: string
  Valor: number | null
  Tipo: string
  Fecha: string
}

/**
 * Control de los semaforos (los maneja el Arduino Mega maestro).
 */
export const semaforoService = {
  async cambiar(carril: CarrilSemaforo, color: ColorSemaforo): Promise<any> {
    const response = await http.post('/semaforo/cambiar', { carril, color })
    return response.data
  },

  async modoAutomatico(): Promise<any> {
    const response = await http.post('/semaforo/auto')
    return response.data
  },

  async parar(): Promise<any> {
    const response = await http.post('/semaforo/parar')
    return response.data
  },

  async status(): Promise<any> {
    const response = await http.post('/semaforo/status')
    return response.data
  },

  async obtenerEstado(): Promise<SemaforoEstado> {
    const response = await http.get<SemaforoEstado>('/semaforo/estado')
    return response.data
  },

  async obtenerLog(limite: number = 20): Promise<SemaforoLogItem[]> {
    const response = await http.get<SemaforoLogItem[]>('/semaforo/log', { params: { limite } })
    return response.data
  }
}
