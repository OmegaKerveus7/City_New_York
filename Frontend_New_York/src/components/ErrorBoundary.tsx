import { Component, type ReactNode } from "react";

interface Props {
  children: ReactNode;
}

interface State {
  hasError: boolean;
  error: Error | null;
}

export class ErrorBoundary extends Component<Props, State> {
  constructor(props: Props) {
    super(props);
    this.state = { hasError: false, error: null };
  }

  static getDerivedStateFromError(error: Error): State {
    return { hasError: true, error };
  }

  componentDidCatch(error: Error, errorInfo: React.ErrorInfo) {
    console.error("ErrorBoundary caught an error:", error, errorInfo);
  }

  render() {
    if (this.state.hasError) {
      return (
        <div className="min-h-screen flex items-center justify-center bg-slate-100">
          <div className="bg-white rounded-2xl shadow-xl p-8 max-w-md w-full mx-4">
            <div className="text-center">
              <div className="text-5xl mb-4">⚠️</div>
              <h1 className="text-xl font-bold text-gray-900 mb-2">Algo salió mal</h1>
              <p className="text-gray-600 mb-4">
                {this.state.error?.message || "Ha ocurrido un error inesperado"}
              </p>
              <button
                onClick={() => window.location.href = "/login"}
                className="bg-blue-900 text-white px-6 py-2 rounded-lg font-medium hover:bg-blue-800 transition"
              >
                Volver al login
              </button>
            </div>
          </div>
        </div>
      );
    }

    return this.props.children;
  }
}
