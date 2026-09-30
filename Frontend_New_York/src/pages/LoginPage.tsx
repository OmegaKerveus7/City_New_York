import { useState, type FormEvent } from "react";
import { useNavigate } from "react-router-dom";
import { login } from "../services/api";
import { useAuth } from "../context/AuthContext";

export default function LoginPage() {
  const navigate = useNavigate();
  const { iniciarSesion } = useAuth();
  const [correo, setCorreo] = useState("");
  const [contrasena, setContrasena] = useState("");
  const [mostrar, setMostrar] = useState(false);
  const [cargando, setCargando] = useState(false);
  const [error, setError] = useState("");

  async function handleSubmit(e: FormEvent) {
    e.preventDefault();
    if (!correo || !contrasena) {
      setError("Ingresa tu correo y contraseña.");
      return;
    }
    setCargando(true);
    setError("");
    try {
      const respuesta = await login(correo, contrasena);
      if (respuesta.codigo !== 200 || !respuesta.usuario || !respuesta.token) {
        throw new Error(respuesta.mensaje || "No se pudo iniciar sesión.");
      }
      iniciarSesion(respuesta.usuario, respuesta.token);
      navigate("/inicio", { replace: true });
    } catch (err) {
      setError(err instanceof Error ? err.message : "No se pudo iniciar sesión.");
    } finally {
      setCargando(false);
    }
  }

  return (
    <div className="flex min-h-screen items-center justify-center bg-gradient-to-br from-slate-100 via-blue-50 to-slate-100 p-4">
      <div className="grid w-full max-w-4xl overflow-hidden rounded-3xl bg-white shadow-2xl md:grid-cols-2">
        <div className="hidden flex-col items-center justify-center gap-6 bg-gradient-to-br from-blue-900 to-blue-700 p-10 md:flex">
          <div className="text-center text-white">
            <h1 className="text-3xl font-bold">Ciudad New York</h1>
            <p className="mt-2 text-blue-200">Sistema de Gestión Municipal</p>
          </div>
          <div className="mt-8 text-center text-white/80">
            <p className="text-sm">Administra áreas, trámites y documentos de forma eficiente</p>
          </div>
        </div>

        <div className="flex flex-col justify-center gap-6 p-8 sm:p-12">
          <div className="flex flex-col items-center gap-3 md:items-start">
            <h1 className="text-2xl font-bold text-slate-900">Iniciar sesión</h1>
            <p className="text-sm text-slate-500">Accede con tu cuenta municipal</p>
          </div>

          <form onSubmit={handleSubmit} className="space-y-4">
            <div className="space-y-1">
              <label htmlFor="correo" className="text-sm font-medium text-slate-700">
                Correo electrónico
              </label>
              <input
                id="correo"
                type="email"
                value={correo}
                onChange={(e) => setCorreo(e.target.value)}
                placeholder="correo@ejemplo.com"
                autoComplete="email"
                className="w-full rounded-xl border border-slate-300 py-2.5 px-3 text-sm outline-none transition focus:border-blue-500 focus:ring-2 focus:ring-blue-200"
              />
            </div>

            <div className="space-y-1">
              <label htmlFor="contrasena" className="text-sm font-medium text-slate-700">
                Contraseña
              </label>
              <div className="relative">
                <input
                  id="contrasena"
                  type={mostrar ? "text" : "password"}
                  value={contrasena}
                  onChange={(e) => setContrasena(e.target.value)}
                  placeholder="********"
                  autoComplete="current-password"
                  className="w-full rounded-xl border border-slate-300 py-2.5 px-3 pr-10 text-sm outline-none transition focus:border-blue-500 focus:ring-2 focus:ring-blue-200"
                />
                <button
                  type="button"
                  onClick={() => setMostrar((m) => !m)}
                  className="absolute right-3 top-1/2 -translate-y-1/2 text-slate-400 hover:text-slate-600"
                >
                  {mostrar ? "Ocultar" : "Mostrar"}
                </button>
              </div>
            </div>

            {error && (
              <p className="rounded-xl bg-red-50 px-3 py-2 text-sm font-medium text-red-600">{error}</p>
            )}

            <button
              type="submit"
              disabled={cargando}
              className="flex w-full items-center justify-center gap-2 rounded-xl bg-blue-900 py-2.5 text-sm font-semibold text-white transition hover:bg-blue-800 disabled:cursor-not-allowed disabled:opacity-60"
            >
              {cargando ? "Ingresando..." : "Ingresar"}
            </button>
          </form>

          <p className="text-center text-sm text-slate-500">
            ¿No tienes cuenta?{" "}
            <button
              type="button"
              onClick={() => navigate("/registro")}
              className="font-semibold text-blue-600 hover:text-blue-700 hover:underline"
            >
              Crear cuenta
            </button>
          </p>
        </div>
      </div>
    </div>
  );
}
