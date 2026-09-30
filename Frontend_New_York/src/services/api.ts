export interface Usuario {
  idUsuario: number;
  nombreUsuario: string;
  apellidosUsuario: string;
  correo: string;
  contrasena?: string;
  rol: string;
  activo: boolean;
}

export interface LoginResponse {
  token: string;
  expira: string;
}

export interface LoginApiResponse {
  codigo: number;
  mensaje: string;
  token: string | null;
  usuario: Usuario | null;
}

export interface ResponseDto<T = unknown> {
  codigo: number;
  mensaje: string;
  data: T;
}

export interface RegistroRequest {
  nombreUsuario: string;
  apellidosUsuario: string;
  correo: string;
  contrasena: string;
  idRol: number;
}

const BASE_URL = "/api/auth";
const TOKEN_KEY = "ciudadny.token";
const USUARIO_KEY = "ciudadny.usuario";

function getToken(): string | null {
  return localStorage.getItem(TOKEN_KEY);
}

async function request<T>(
  path: string,
  init?: RequestInit
): Promise<T> {
  const token = getToken();
  const headers: Record<string, string> = {
    "Content-Type": "application/json",
    ...(token ? { Authorization: `Bearer ${token}` } : {}),
    ...(init?.headers as Record<string, string> || {}),
  };

  const res = await fetch(`${BASE_URL}${path}`, {
    ...init,
    headers,
  });

  if (res.status === 401) {
    localStorage.removeItem(TOKEN_KEY);
    localStorage.removeItem(USUARIO_KEY);
    window.location.href = "/login";
    throw new Error("Sesión expirada. Inicie sesión nuevamente.");
  }

  if (!res.ok) {
    const text = await res.text();
    let mensaje = `Error ${res.status}`;

    if (text) {
      try {
        const respuesta = JSON.parse(text) as { mensaje?: string };
        mensaje = respuesta.mensaje || mensaje;
      } catch {
        mensaje = text;
      }
    }

    throw new Error(mensaje);
  }

  return (await res.json()) as T;
}

export async function login(
  correo: string,
  contrasena: string
): Promise<LoginApiResponse> {
  return request<LoginApiResponse>("/login", {
    method: "POST",
    body: JSON.stringify({ correo, contrasena }),
  });
}

export async function registrar(
  datos: RegistroRequest
): Promise<ResponseDto<{ idUsuario: number }>> {
  return request<ResponseDto<{ idUsuario: number }>>("/registrar", {
    method: "POST",
    body: JSON.stringify(datos),
  });
}

export function guardarToken(token: string) {
  localStorage.setItem(TOKEN_KEY, token);
}

export function guardarUsuario(usuario: Usuario) {
  localStorage.setItem(USUARIO_KEY, JSON.stringify(usuario));
}

export function obtenerUsuario(): Usuario | null {
  const raw = localStorage.getItem(USUARIO_KEY);
  if (!raw) return null;
  try {
    return JSON.parse(raw) as Usuario;
  } catch {
    return null;
  }
}

export function eliminarToken() {
  localStorage.removeItem(TOKEN_KEY);
  localStorage.removeItem(USUARIO_KEY);
}

export function obtenerToken(): string | null {
  return getToken();
}

export function estaAutenticado(): boolean {
  return getToken() !== null;
}
