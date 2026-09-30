import { Outlet, useNavigate } from "react-router-dom";
import { useEffect, useState } from "react";
import Sidebar from "./Sidebar";
import Topbar from "./Topbar";
import { useAuth } from "../context/AuthContext";

export default function DashboardLayout() {
  const [colapsado, setColapsado] = useState(false);
  const [movil, setMovil] = useState(() => window.innerWidth < 768);
  const [menuMovil, setMenuMovil] = useState(false);
  const { usuario, token } = useAuth();
  const navigate = useNavigate();

  useEffect(() => {
    const resize = () => {
      setMovil(window.innerWidth < 768);
      if (window.innerWidth >= 768) setMenuMovil(false);
    };
    window.addEventListener('resize', resize);
    return () => window.removeEventListener('resize', resize);
  }, []);

  useEffect(() => {
    if (!usuario || !token) {
      navigate("/login", { replace: true });
    }
  }, [usuario, token, navigate]);

  if (!usuario || !token) return null;

  return (
    <div className="flex h-screen w-full overflow-hidden bg-slate-50">
      {movil ? (
        menuMovil && (
          <>
            <button
              aria-label="Cerrar menú"
              className="fixed inset-0 z-30 bg-slate-900/40"
              onClick={() => setMenuMovil(false)}
            />
            <div className="fixed inset-y-0 left-0 z-40 flex">
              <Sidebar colapsado={false} onToggle={() => setMenuMovil(false)} />
            </div>
          </>
        )
      ) : (
        <Sidebar colapsado={colapsado} onToggle={() => setColapsado(v => !v)} />
      )}

      <div className="flex min-w-0 flex-1 flex-col overflow-hidden">
        <Topbar onToggleSidebar={() => movil ? setMenuMovil(v => !v) : setColapsado(v => !v)} />
        <main className="flex-1 overflow-y-auto p-4 sm:p-6">
          <Outlet />
        </main>
      </div>
    </div>
  );
}
