import type { ReactNode } from "react";
import { Routes, Route, Navigate } from "react-router-dom";
import LoginPage from "./pages/LoginPage";
import RegistroPage from "./pages/RegistroPage";
import DashboardPage from "./pages/DashboardPage";
import DashboardLayout from "./components/DashboardLayout";
import { useAuth } from "./context/AuthContext";
import { ErrorBoundary } from "./components/ErrorBoundary";

function RequireAuth({ children }: { children: ReactNode }) {
  const { usuario } = useAuth();
  if (!usuario) return <Navigate to="/login" replace />;
  return children;
}

function AppContent() {
  return (
    <Routes>
      <Route path="/login" element={<LoginPage />} />
      <Route path="/registro" element={<RegistroPage />} />

      <Route
        element={
          <RequireAuth>
            <DashboardLayout />
          </RequireAuth>
        }
      >
        <Route path="/inicio" element={<DashboardPage />} />
        <Route path="/usuarios" element={<div className="text-center py-10 text-slate-500">Módulo de Usuarios - En desarrollo</div>} />
        <Route path="/areas" element={<div className="text-center py-10 text-slate-500">Módulo de Áreas - En desarrollo</div>} />
        <Route path="/tramites" element={<div className="text-center py-10 text-slate-500">Módulo de Trámites - En desarrollo</div>} />
        <Route path="/documentos" element={<div className="text-center py-10 text-slate-500">Módulo de Documentos - En desarrollo</div>} />
        <Route path="/reportes" element={<div className="text-center py-10 text-slate-500">Módulo de Reportes - En desarrollo</div>} />
        <Route path="/configuracion" element={<div className="text-center py-10 text-slate-500">Módulo de Configuración - En desarrollo</div>} />
      </Route>

      <Route path="/" element={<Navigate to="/login" replace />} />
      <Route path="*" element={<Navigate to="/login" replace />} />
    </Routes>
  );
}

export default function App() {
  return (
    <ErrorBoundary>
      <AppContent />
    </ErrorBoundary>
  );
}
