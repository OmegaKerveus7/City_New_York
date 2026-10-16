import http from './http'
import { instruccionesService } from './instruccionesService'

export interface LogItem {
  Id: number
  Instruccion: string
  Resultado: string
  Pasos: number
  Tipo: string
  Fecha: string
}

export const telefericoService = {
  async avanzar(pasos: number = 3000): Promise<any> {
    const nombre = `TELEFERICO_AVANZAR ${pasos}`
    return instruccionesService.enviarComando(nombre)
  },

  async regresar(pasos: number = 3000): Promise<any> {
    const nombre = `TELEFERICO_REGRESAR ${pasos}`
    return instruccionesService.enviarComando(nombre)
  },

  async parar(): Promise<any> {
    return instruccionesService.enviarComando('TELEFERICO_PARAR')
  },

  async obtenerLog(limite: number = 20): Promise<LogItem[]> {
    const response = await http.get<LogItem[]>('/teleferico/log', { params: { limite } })
    return response.data
  }
}
