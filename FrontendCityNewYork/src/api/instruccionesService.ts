import http from './http'

export interface Instruccion {
  Id: number
  Nombre: string
  Descripcion: string
  Activa: boolean
}

export interface LogItem {
  Id: number
  Instruccion: string
  Resultado: string
  Angulo: number
  Tipo: string
  Fecha: string
}

export const instruccionesService = {
  // Funcion unica para enviar comandos al hardware:
  // solo se agrega el nombre de la instruccion (ej: 'TELEFERICO_AVANZAR').
  // Si no existe en el catalogo, el backend la crea automaticamente.
  async enviarComando(nombre: string): Promise<{ id: number; mensaje: string; nombre: string }> {
    const response = await http.post('/instrucciones', { nombre })
    return response.data
  },

  async obtenerTodas(): Promise<Instruccion[]> {
    const response = await http.get<Instruccion[]>('/instrucciones')
    return response.data
  },

  async obtenerPendientes(): Promise<any[]> {
    const response = await http.get('/instrucciones/pendientes')
    return response.data
  },

  async log(limite: number = 20): Promise<LogItem[]> {
    const response = await http.get<LogItem[]>('/instrucciones/log', { params: { limite } })
    return response.data
  }
}
