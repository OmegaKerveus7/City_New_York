import { useState, type FormEvent } from "react";
import { useNavigate } from "react-router-dom";
import { registrar } from "../services/api";

export default function RegistroPage() {
  const navigate = useNavigate();
  const [nombreUsuario, setNombreUsuario] = useState("");
  const [apellidosUsuario, setApellidosUsuario] = useState("");
  const [correo, setCorreo] = useState("");
  const [contrasena, setContrasena] = useState("");
  const [confirmarContrasena, setConfirmarContrasena] = useState("");
  const [cargando, setCargando] = useState(false);
  const [error, setError] = useState("");
  const [exito, setExito] = useState(false);

  async function handleSubmit(e: FormEvent) {
    e.preventDefault();
    setError("");

    if (contrasena !== confirmarContrasena) {
      setError("Las contraseñas no coinciden");
      return;
    }

    if (contrasena.length < 6) {
      setError("La contraseña debe tener al menos 6 caracteres");
      return;
    }

    setCargando(true);
    try {
      const respuesta = await registrar({
        nombreUsuario,
        apellidosUsuario,
        correo,
        contrasena,
        idRol: 2,
      });

      if (respuesta.codigo === 200) {
        setExito(true);
      } else {
        setError(respuesta.mensaje || "Error al crear usuario");
      }
    } catch (err) {
      setError(err instanceof Error ? err.message : "No se pudo crear la cuenta");
    } finally {
      setCargando(false);
    }
  }

  if (exito) {
    return (
      <div className="flex min-h-screen items-center justify-center bg-gradient-to-br from-slate-100 via-blue-50 to-slate-100 p-4">
        <div className="rounded-2xl bg-white p-8 shadow-2xl text-center max-w-md w-full">
          <div className="text-6xl mb-4">🎉</div>
          <h2 className="text-2xl font-bold text-gray-900 mb-2">¡Cuenta creada exitosamente!</h2>
          <p className="text-gray-600 mb-6">Tu usuario ha sido registrado en el sistema.</p>
          <button
            onClick={() => navigate("/login")}
            className="w-full rounded-xl bg-blue-900 py-3 text-sm font-semibold text-white transition hover:bg-blue-800"
          >
            Ir al Login
          </button>
        </div>
      </div>
    );
  }

  return (
    <div className="flex min-h-screen items-center justify-center bg-gradient-to-br from-slate-100 via-blue-50 to-slate-100 p-4">
      <div className="w-full max-w-md overflow-hidden rounded-2xl bg-white shadow-2xl">
        <div className="bg-gradient-to-r from-blue-900 to-blue-700 p-6 text-center">
          <h1 className="text-xl font-bold text-white">Crear Cuenta</h1>
          <p className="mt-1 text-blue-200">Sistema de Gestión Municipal</p>
        </div>

        <form onSubmit={handleSubmit} className="space-y-4 p-6">
          <div className="grid grid-cols-2 gap-4">
            <div className="space-y-1">
              <label htmlFor="nombre" className="text-sm font-medium text-slate-700">Nombre</label>
              <input
                id="nombre"
                type="text"
                value={nombreUsuario}
                onChange={(e) => setNombreUsuario(e.target.value)}
                placeholder="Tu nombre"
                required
                className="w-full rounded-xl border border-slate-300 py-2 px-3 text-sm outline-none focus:border-blue-500 focus:ring-2 focus:ring-blue-200"
              />
            </div>
            <div className="space-y-1">
              <label htmlFor="apellidos" className="text-sm font-medium text-slate-700">Apellidos</label>
              <input
                id="apellidos"
                type="text"
                value={apellidosUsuario}
                onChange={(e) => setApellidosUsuario(e.target.value)}
                placeholder="Tus apellidos"
                required
                className="w-full rounded-xl border border-slate-300 py-2 px-3 text-sm outline-none focus:border-blue-500 focus:ring-2 focus:ring-blue-200"
              />
            </div>
          </div>

          <div className="space-y-1">
            <label htmlFor="correo" className="text-sm font-medium text-slate-700">Correo electrónico</label>
            <input
              id="correo"
              type="email"
              value={correo}
              onChange={(e) => setCorreo(e.target.value)}
              placeholder="correo@ejemplo.com"
              required
              className="w-full rounded-xl border border-slate-300 py-2 px-3 text-sm outline-none focus:border-blue-500 focus:ring-2 focus:ring-blue-200"
            />
          </div>

          <div className="space-y-1">
            <label htmlFor="contrasena" className="text-sm font-medium text-slate-700">Contraseña</label>
            <input
              id="contrasena"
              type="password"
              value={contrasena}
              onChange={(e) => setContrasena(e.target.value)}
              placeholder="Mínimo 6 caracteres"
              required
              className="w-full rounded-xl border border-slate-300 py-2 px-3 text-sm outline-none focus:border-blue-500 focus:ring-2 focus:ring-blue-200"
            />
          </div>

          <div className="space-y-1">
            <label htmlFor="confirmar" className="text-sm font-medium text-slate-700">Confirmar contraseña</label>
            <input
              id="confirmar"
              type="password"
              value={confirmarContrasena}
              onChange={(e) => setConfirmarContrasena(e.target.value)}
              placeholder="Repite tu contraseña"
              required
              className="w-full rounded-xl border border-slate-300 py-2 px-3 text-sm outline-none focus:border-blue-500 focus:ring-2 focus:ring-blue-200"
            />
          </div>

          {error && (
            <p className="rounded-xl bg-red-50 px-3 py-2 text-sm font-medium text-red-600">{error}</p>
          )}

          <button
            type="submit"
            disabled={cargando}
            className="w-full rounded-xl bg-blue-900 py-2.5 text-sm font-semibold text-white transition hover:bg-blue-800 disabled:cursor-not-allowed disabled:opacity-60"
          >
            {cargando ? "Creando cuenta..." : "Crear Cuenta"}
          </button>
        </form>

        <p className="pb-6 text-center text-sm text-slate-500">
          ¿Ya tienes cuenta?{" "}
          <button
            type="button"
            onClick={() => navigate("/login")}
            className="font-semibold text-blue-600 hover:text-blue-700 hover:underline"
          >
            Inicia sesión
          </button>
        </p>
      </div>
    </div>
  );
}
