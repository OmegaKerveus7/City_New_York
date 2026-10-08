# City_New_York

## Project Structure
- `Bakend_New_York/API_rest_City/` — ASP.NET Core 8 minimal API project
- `BD_New_York/Conetnedor/docker-compose.yml` — SQL Server 2022 container
- `FrontendCityNewYork/` — Vue 3 frontend con diseño NYC y estructura en capas
- `Componentes_fisicos/` — Arduino/ESP32 código para control físico del puente

## Hardware - Control de Puente Elevadizo

### Arquitectura de Comunicación
```
Frontend (Vue) → Backend (ASP.NET) → ESP32 (ngrok) → Mega (UART) → Esclavo (I2C) → Servo
                          ↑
                    Polling cada 5s
```

### Componentes
| Dispositivo | Rol | Descripción |
|-------------|-----|-------------|
| ESP32 | Gateway WiFi + HTTP Server | Recibe polling del backend, envía comandos al Mega por UART con TXS0108E |
| Arduino Mega | Gateway I2C | Reenvía comandos al esclavo por I2C (0x08) |
| Arduino Uno (Esclavo) | Controlador Servo | Controla el servo MG996R en pin 9 |

### Conexiones Físicas
```
ESP32 (UART) → TXS0108E → Mega (Serial1)
Mega (I2C) → Esclavo (0x08)
Esclavo (Pin 9) → Servo MG996R
```

### Endpoints del ESP32 (puerto 80)
| Ruta | Método | Descripción |
|------|--------|-------------|
| `/abrir` | POST | Mueve servo a 85° (ABIERTO) |
| `/cerrar` | POST | Mueve servo a 180° (CERRADO) |
| `/estado` | GET | Retorna estado: { estado, angulo, megaOnline, esclavoOnline } |
| `/actualizar?angulo=n` | POST | Mueve servo a n grados (0-180) |
| `/verificar` | GET | Heartbeat del ESP32 |

### Botones Locales (GPIO)
| Pin | Función |
|-----|---------|
| GPIO 32 | Botón ABRIR físico |
| GPIO 33 | Botón CERRAR físico |

### Sistema de Instrucciones

#### Tablas BD
- `Instruccion` - Catálogo de instrucciones (Esc1X0100, Esc1X0200, etc.)
- `Permiso` - Permisos por rol para ejecutar instrucciones
- `Log_Instruccion` - Historial de ejecuciones
- `Dispositivo` - Registro de ESP32, Mega, Esclavo
- `Instruccion_Pendiente` - Cola de instrucciones por procesar

#### Códigos de Instrucciones
| Código | Componente | Descripción |
|--------|------------|-------------|
| Esc1X0100 | Esc1 | Subir puente (ABRIR) - 85° |
| Esc1X0200 | Esc1 | Bajar puente (CERRAR) - 180° |

#### Endpoints Backend
| Ruta | Método | Descripción |
|------|--------|-------------|
| `GET /api/instrucciones` | GET | Lista todas las instrucciones |
| `GET /api/instrucciones/pendientes/{tipo}` | GET | ESP32 consulta instrucciones pendientes |
| `POST /api/instrucciones` | POST | Crear instrucción pendiente |
| `POST /api/instrucciones/{id}/respuesta` | POST | ESP32 envía resultado de ejecución |
| `POST /api/instrucciones/local` | POST | Notificación de ejecución local (botón físico) |
| `GET /api/instrucciones/log` | GET | Historial de ejecuciones |
| `GET /api/instrucciones/verificar/{tipo}` | GET | Heartbeat del dispositivo |
| `GET /api/puente/estado` | GET | Estado del servo |
| `POST /api/puente/abrir` | POST | Crear instrucción ABRIR |
| `POST /api/puente/cerrar` | POST | Crear instrucción CERRAR |
| `GET /api/puente/log` | GET | Log del puente |
| `GET /api/puente/dispositivos` | GET | Estado de dispositivos |

### Archivos Arduino
- `Componentes_fisicos/Esp32_gateway/Esp32_gateway.ino` — ESP32 gateway WiFi-UART con polling
- `Componentes_fisicos/Mega_NY/Mega_NY.ino` — Mega gateway I2C-UART
- `Componentes_fisicos/esclavo_frontera/esclavo_frontera.ino` — Arduino Uno con servo MG996R

### Archivos BD
- `BD_New_York/Conetnedor/Setup_Instrucciones.sql` — Tablas del sistema de instrucciones
- `BD_New_York/Conetnedor/StoredProcedures_Instrucciones.sql` — SPs para instrucciones

### Configuración
- Backend ngrok: `https://warless-predestinately-bethann.ngrok-free.dev`
- WiFi ESP32: Omega / d5adc4a32689

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
