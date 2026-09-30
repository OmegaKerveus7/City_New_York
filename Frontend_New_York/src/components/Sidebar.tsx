import { useNavigate, useLocation } from "react-router-dom";

interface SidebarProps {
  colapsado: boolean;
  onToggle: () => void;
}

export default function Sidebar({ colapsado, onToggle }: SidebarProps) {
  const navigate = useNavigate();
  const location = useLocation();

  const menuItems = [
    { path: "/inicio", label: "Dashboard", icon: "📊" },
    { path: "/usuarios", label: "Usuarios", icon: "👥" },
    { path: "/areas", label: "Áreas", icon: "🏢" },
    { path: "/tramites", label: "Trámites", icon: "📋" },
    { path: "/documentos", label: "Documentos", icon: "📄" },
    { path: "/reportes", label: "Reportes", icon: "📁" },
    { path: "/configuracion", label: "Configuración", icon: "⚙️" },
  ];

  return (
    <aside className={`flex h-full flex-col border-r border-slate-200 bg-white transition-all duration-300 ${colapsado ? "w-16" : "w-64"}`}>
      <div className="flex items-center gap-3 border-b border-slate-200 p-4">
        <div className="flex h-10 w-10 items-center justify-center rounded-lg bg-blue-900 text-lg font-bold text-white">
          CNY
        </div>
        {!colapsado && (
          <div>
            <h1 className="text-sm font-semibold text-slate-900">Ciudad New York</h1>
            <p className="text-xs text-slate-500">Gestión Municipal</p>
          </div>
        )}
        <button
          onClick={onToggle}
          className="ml-auto rounded-lg p-1.5 text-slate-400 hover:bg-slate-100 hover:text-slate-600"
        >
          {colapsado ? "→" : "←"}
        </button>
      </div>

      <nav className="flex-1 overflow-y-auto p-3">
        <ul className="space-y-1">
          {menuItems.map((item) => (
            <li key={item.path}>
              <button
                onClick={() => navigate(item.path)}
                className={`flex w-full items-center gap-3 rounded-lg px-3 py-2.5 text-sm font-medium transition ${
                  location.pathname === item.path
                    ? "bg-blue-50 text-blue-700"
                    : "text-slate-600 hover:bg-slate-50 hover:text-slate-900"
                }`}
              >
                <span className="text-lg">{item.icon}</span>
                {!colapsado && <span>{item.label}</span>}
              </button>
            </li>
          ))}
        </ul>
      </nav>

      <div className="border-t border-slate-200 p-3">
        <button
          onClick={() => navigate("/login")}
          className="flex w-full items-center gap-3 rounded-lg px-3 py-2.5 text-sm font-medium text-red-600 hover:bg-red-50"
        >
          <span className="text-lg">🚪</span>
          {!colapsado && <span>Cerrar Sesión</span>}
        </button>
      </div>
    </aside>
  );
}
