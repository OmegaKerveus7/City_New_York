import http from './http'

export interface ElevadorEstado {
  nivel: number
  megaOnline: boolean
  esclavoOnline: boolean
  ultimaInstruccion?: string | null
}

export interface LogItem {
  Id: number
  Instruccion: string
  Resultado: string
  Nivel: number
  Tipo: string
  Fecha: string
}

export const elevadorService = {
  async obtenerEstado(): Promise<ElevadorEstado> {
    const response = await http.get<ElevadorEstado>('/elevador/estado')
    return response.data
  },

  async subir(niveles: number = 1): Promise<any> {
    const response = await http.post('/elevador/subir', niveles)
    return response.data
  },

  async bajar(niveles: number = 1): Promise<any> {
    const response = await http.post('/elevador/bajar', niveles)
    return response.data
  },

  async ir(nivel: number): Promise<any> {
    const response = await http.post('/elevador/ir', { nivel })
    return response.data
  },

  async parar(): Promise<any> {
    const response = await http.post('/elevador/parar')
    return response.data
  },

  async calibrar(): Promise<any> {
    const response = await http.post('/elevador/calibrar')
    return response.data
  },

  async obtenerLog(limite: number = 20): Promise<LogItem[]> {
    const response = await http.get<LogItem[]>('/elevador/log', { params: { limite } })
    return response.data
  }
}