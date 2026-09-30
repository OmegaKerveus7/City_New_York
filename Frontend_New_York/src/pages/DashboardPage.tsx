import { useAuth } from "../context/AuthContext";

export default function DashboardPage() {
  const { usuario } = useAuth();

  const today = new Date();
  const formattedDate = today.toLocaleDateString('es-ES', {
    weekday: 'long',
    year: 'numeric',
    month: 'long',
    day: 'numeric'
  });

  return (
    <div className="space-y-6">
      <div className="rounded-2xl bg-gradient-to-r from-blue-900 to-blue-700 p-6 text-white">
        <h1 className="text-2xl font-bold">¡Bienvenido, {usuario?.nombreUsuario}!</h1>
        <p className="mt-1 text-blue-200">{formattedDate}</p>
      </div>

      <div className="grid gap-4 md:grid-cols-2 lg:grid-cols-4">
        <div className="rounded-xl bg-white p-5 shadow-sm border border-slate-200">
          <div className="flex items-center gap-4">
            <div className="rounded-lg bg-blue-100 p-3 text-2xl">👥</div>
            <div>
              <p className="text-sm text-slate-500">Usuarios</p>
              <p className="text-xl font-semibold text-slate-900">--</p>
            </div>
          </div>
        </div>

        <div className="rounded-xl bg-white p-5 shadow-sm border border-slate-200">
          <div className="flex items-center gap-4">
            <div className="rounded-lg bg-green-100 p-3 text-2xl">🏢</div>
            <div>
              <p className="text-sm text-slate-500">Áreas</p>
              <p className="text-xl font-semibold text-slate-900">--</p>
            </div>
          </div>
        </div>

        <div className="rounded-xl bg-white p-5 shadow-sm border border-slate-200">
          <div className="flex items-center gap-4">
            <div className="rounded-lg bg-amber-100 p-3 text-2xl">📋</div>
            <div>
              <p className="text-sm text-slate-500">Trámites</p>
              <p className="text-xl font-semibold text-slate-900">--</p>
            </div>
          </div>
        </div>

        <div className="rounded-xl bg-white p-5 shadow-sm border border-slate-200">
          <div className="flex items-center gap-4">
            <div className="rounded-lg bg-purple-100 p-3 text-2xl">📄</div>
            <div>
              <p className="text-sm text-slate-500">Documentos</p>
              <p className="text-xl font-semibold text-slate-900">--</p>
            </div>
          </div>
        </div>
      </div>

      <div className="grid gap-4 md:grid-cols-2">
        <div className="rounded-xl bg-white p-5 shadow-sm border border-slate-200">
          <h3 className="font-semibold text-slate-900 mb-4">Accesos Rápidos</h3>
          <div className="space-y-3">
            <div className="flex items-center gap-3 rounded-lg bg-slate-50 p-3">
              <span className="text-xl">👤</span>
              <span className="text-sm text-slate-700">Gestión de Usuarios</span>
            </div>
            <div className="flex items-center gap-3 rounded-lg bg-slate-50 p-3">
              <span className="text-xl">🏛️</span>
              <span className="text-sm text-slate-700">Áreas Municipales</span>
            </div>
            <div className="flex items-center gap-3 rounded-lg bg-slate-50 p-3">
              <span className="text-xl">📊</span>
              <span className="text-sm text-slate-700">Reportes</span>
            </div>
          </div>
        </div>

        <div className="rounded-xl bg-white p-5 shadow-sm border border-slate-200">
          <h3 className="font-semibold text-slate-900 mb-4">Estado del Sistema</h3>
          <div className="space-y-3">
            <div className="flex items-center justify-between rounded-lg bg-slate-50 p-3">
              <div className="flex items-center gap-3">
                <span className="h-3 w-3 rounded-full bg-green-500"></span>
                <span className="text-sm text-slate-700">API Backend</span>
              </div>
              <span className="text-sm font-medium text-green-600">En línea</span>
            </div>
            <div className="flex items-center justify-between rounded-lg bg-slate-50 p-3">
              <div className="flex items-center gap-3">
                <span className="h-3 w-3 rounded-full bg-green-500"></span>
                <span className="text-sm text-slate-700">Base de Datos</span>
              </div>
              <span className="text-sm font-medium text-green-600">Conectado</span>
            </div>
            <div className="flex items-center justify-between rounded-lg bg-slate-50 p-3">
              <div className="flex items-center gap-3">
                <span className="h-3 w-3 rounded-full bg-amber-500"></span>
                <span className="text-sm text-slate-700">Modo</span>
              </div>
              <span className="text-sm font-medium text-amber-600">Desarrollo</span>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
