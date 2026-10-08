import http from './http'

export interface PuenteEstado {
  estado: 'ABIERTO' | 'CERRADO'
  angulo: number
  megaOnline?: boolean
  esclavoOnline?: boolean
}

export interface LogItem {
  Id: number
  Instruccion: string
  Resultado: string
  Angulo: number
  Tipo: string
  Fecha: string
}

export interface Instruccion {
  Id: number
  Nombre: string
  Descripcion: string
  Activa: boolean
}

export const puenteService = {
  async obtenerEstado(): Promise<PuenteEstado> {
    const response = await http.get<PuenteEstado>('/puente/estado')
    return response.data
  },

  async abrir(): Promise<any> {
    const response = await http.post('/puente/abrir')
    return response.data
  },

  async cerrar(): Promise<any> {
    const response = await http.post('/puente/cerrar')
    return response.data
  },

  async obtenerLog(limite: number = 20): Promise<LogItem[]> {
    const response = await http.get<LogItem[]>('/puente/log', { params: { limite } })
    return response.data
  },

  async obtenerInstrucciones(): Promise<Instruccion[]> {
    const response = await http.get<Instruccion[]>('/instrucciones')
    return response.data
  }
}
