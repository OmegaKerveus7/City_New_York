# City_New_York

## Project Structure
- `Bakend_New_York/API_rest_City/` — ASP.NET Core 8 minimal API project
- `BD_New_York/Conetnedor/docker-compose.yml` — SQL Server 2022 container
- `FrontendCityNewYork/` — Vue 3 frontend con diseño NYC y estructura en capas

## Run the API
```bash
cd Bakend_New_York/API_rest_City
dotnet run
```
- HTTP: http://localhost:5238
- HTTPS: https://localhost:7137
- Default endpoint: `/weatherforecast`

## Database
SQL Server via Docker (port 1444 host → 1433 container):
```bash
cd BD_New_York/Conetnedor
docker compose up -d
```
- SA password: `Arquitectura2_New_york` (dev only)

## Build
```bash
dotnet build Bakend_New_York/API_rest_City/API_rest_City.csproj
```

## Frontend (Vue 3)
Sistema de control de la ciudad de New York con autenticación JWT y diseño empresarial NYC.

### Diseño
- **Estilo**: Empresarial moderno con colores típicos de NYC (amarillo taxi, azul navy, acero)
- **Animaciones**: Fade-in, slide, scale con delays escalonados
- **Skyline**: SVG animado de Manhattan con taxis y estrellas
- **Responsive**: Adaptable a móvil y escritorio

### Estructura en Capas
```
FrontendCityNewYork/
├── public/
│   └── logo_new_York.webp  # Logo NYC optimizado
├── src/
│   ├── api/          # HTTP con interceptors JWT
│   │   ├── http.ts       # Axios instance
│   │   └── authService.ts # Login, registro, roles
│   ├── layouts/      # Layouts de páginas
│   │   └── DashboardLayout.vue # Layout con sidebar y menú
│   ├── pages/        # Vistas principales
│   │   ├── LoginPage.vue     # Login con skyline NYC
│   │   ├── RegisterPage.vue  # Registro de usuarios
│   │   ├── DashboardPage.vue # Control de Ciudad
│   │   ├── CentralParkPage.vue # Control de Central Park
│   │   └── PuenteElevadizoPage.vue # Puentes elevadizos
│   ├── router/       # Rutas + guards
│   │   └── index.ts
│   ├── stores/       # Pinia stores
│   │   └── auth.ts   # Auth state + actions
│   ├── types/        # TypeScript types
│   │   └── auth.ts   # LoginRequest, Usuario, etc
│   └── style.css     # Variables NYC + animaciones
├── App.vue
└── package.json
```

### Autenticación
- **Login**: POST `/api/auth/login` → { token, usuario }
- **Registro**: POST `/api/auth/registrar`
- **Roles**: GET `/api/auth/roles`
- **Token**: Almacenado en localStorage + header Authorization
- **Guards**: Redirección automática /login ↔ /dashboard

### Menú de Navegación
| Ruta | Módulo | Descripción |
|------|---------|-------------|
| `/` | Control de Ciudad | Dashboard principal con estadísticas |
| `/central-park` | Central Park | Gestión del parque y mantenimiento |
| `/puente-elevadizo` | Punto de Entrada | Control de puentes elevadizos |

### Colores NYC
```css
--nyc-navy: #0A1628        /* Azul navy Manhattan */
--nyc-taxi: #FFC107        /* Amarillo taxi NYC */
--nyc-red: #C53030         /* Rojo ladrillo */
--nyc-steel: #4A5568       /* Gris acero puente */
```

### Run Frontend
```bash
cd FrontendCityNewYork
bun install   # Solo primera vez
bun run dev   # http://localhost:3000
```
