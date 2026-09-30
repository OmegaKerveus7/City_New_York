import { createContext, useContext, useEffect, useRef, useState, useCallback, type ReactNode } from "react";
import type { Usuario } from "../services/api";
import { guardarToken, guardarUsuario, obtenerUsuario, eliminarToken, obtenerToken } from "../services/api";

interface AuthContextValue {
  usuario: Usuario | null;
  token: string | null;
  iniciarSesion: (usuario: Usuario, token: string) => void;
  cerrarSesion: () => void;
  registrarActividad: () => void;
}

const AuthContext = createContext<AuthContextValue | null>(null);
const USUARIO_KEY = "ciudadny.usuario";
const ACTIVIDAD_KEY = "ciudadny.ultima_actividad";

export const TIMEOUT_INACTIVIDAD_MS = 2 * 60 * 60 * 1000;
export const AVISO_INACTIVIDAD_MS = 60 * 1000;

export function AuthProvider({ children }: { children: ReactNode }) {
  const [usuario, setUsuario] = useState<Usuario | null>(() => {
    return obtenerUsuario();
  });

  const [token, setToken] = useState<string | null>(obtenerToken);

  const actividadRef = useRef<number>(Date.now());
  const [mostrarAviso, setMostrarAviso] = useState(false);
  const [segundosRestantes, setSegundosRestantes] = useState(
    Math.ceil(AVISO_INACTIVIDAD_MS / 1000),
  );

  const cerrarSesion = useCallback(() => {
    localStorage.removeItem(USUARIO_KEY);
    localStorage.removeItem(ACTIVIDAD_KEY);
    eliminarToken();
    setUsuario(null);
    setToken(null);
  }, []);

  const registrarActividad = useCallback(() => {
    actividadRef.current = Date.now();
    localStorage.setItem(ACTIVIDAD_KEY, String(actividadRef.current));
  }, []);

  const iniciarSesion = useCallback((u: Usuario, t: string) => {
    guardarToken(t);
    guardarUsuario(u);
    setUsuario(u);
    setToken(t);
    actividadRef.current = Date.now();
    localStorage.setItem(ACTIVIDAD_KEY, String(actividadRef.current));
  }, []);

  const handleExpiro = useCallback(() => {
    setMostrarAviso(false);
    cerrarSesion();
    window.location.href = "/login";
  }, [cerrarSesion]);

  const handleAviso = useCallback(() => {
    if (!usuario) return;
    setMostrarAviso(true);
    setSegundosRestantes(Math.ceil(AVISO_INACTIVIDAD_MS / 1000));
  }, [usuario]);

  const handleExtender = useCallback(() => {
    registrarActividad();
    setMostrarAviso(false);
    return Promise.resolve(true);
  }, [registrarActividad]);

  const handleCerrarSesion = useCallback(() => {
    setMostrarAviso(false);
    cerrarSesion();
    window.location.href = "/login";
  }, [cerrarSesion]);

  useEffect(() => {
    if (!usuario) return;

    const eventos: (keyof WindowEventMap)[] = [
      "mousedown",
      "keydown",
      "scroll",
      "touchstart",
      "click",
      "mousemove",
    ];

    const onActivity = () => {
      if (!mostrarAviso) registrarActividad();
    };

    for (const ev of eventos) {
      window.addEventListener(ev, onActivity, { passive: true });
    }

    return () => {
      for (const ev of eventos) {
        window.removeEventListener(ev, onActivity);
      }
    };
  }, [usuario, mostrarAviso, registrarActividad]);

  useEffect(() => {
    if (!usuario) return;

    let timerAviso: ReturnType<typeof setTimeout> | null = null;
    let timerExpiro: ReturnType<typeof setTimeout> | null = null;

    function reset() {
      if (timerAviso) clearTimeout(timerAviso);
      if (timerExpiro) clearTimeout(timerExpiro);
      timerAviso = setTimeout(handleAviso, TIMEOUT_INACTIVIDAD_MS - AVISO_INACTIVIDAD_MS);
      timerExpiro = setTimeout(handleExpiro, TIMEOUT_INACTIVIDAD_MS);
    }

    reset();

    return () => {
      if (timerAviso) clearTimeout(timerAviso);
      if (timerExpiro) clearTimeout(timerExpiro);
    };
  }, [usuario, handleAviso, handleExpiro]);

  useEffect(() => {
    if (!mostrarAviso) return;
    setSegundosRestantes(Math.ceil(AVISO_INACTIVIDAD_MS / 1000));
    const interval = setInterval(() => {
      setSegundosRestantes((s) => Math.max(0, s - 1));
    }, 1000);
    return () => clearInterval(interval);
  }, [mostrarAviso]);

  return (
    <AuthContext.Provider
      value={{
        usuario,
        token,
        iniciarSesion,
        cerrarSesion,
        registrarActividad,
      }}
    >
      {children}
      {mostrarAviso && (
        <div className="fixed inset-0 z-50 flex items-center justify-center bg-black/50">
          <div className="rounded-xl bg-white p-6 shadow-2xl">
            <h2 className="text-lg font-semibold text-gray-900">Sesión por expirar</h2>
            <p className="mt-2 text-sm text-gray-600">
              Tu sesión expira en <strong>{segundosRestantes}</strong> segundos.
            </p>
            <div className="mt-4 flex gap-3">
              <button
                onClick={handleExtender}
                className="rounded-lg bg-blue-600 px-4 py-2 text-sm font-medium text-white hover:bg-blue-500"
              >
                Extender sesión
              </button>
              <button
                onClick={handleCerrarSesion}
                className="rounded-lg bg-gray-200 px-4 py-2 text-sm font-medium text-gray-700 hover:bg-gray-300"
              >
                Cerrar sesión
              </button>
            </div>
          </div>
        </div>
      )}
    </AuthContext.Provider>
  );
}

export function useAuth(): AuthContextValue {
  const ctx = useContext(AuthContext);
  if (!ctx) throw new Error("useAuth debe usarse dentro de AuthProvider");
  return ctx;
}
