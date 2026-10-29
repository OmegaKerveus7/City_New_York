# City_New_York

Maqueta funcional de la ciudad de Nueva York: frontend Vue 3, backend ASP.NET Core 8, SQL Server en Docker y hardware Arduino/ESP32.

**Leyenda:** ✅ implementado y funcionando · 🚧 planificado (especificación propuesta, aún sin código)

---

## Estructura del proyecto

```
City_New_York/
├── Bakend_New_York/API_rest_City/     # ASP.NET Core 8 (minimal API + controllers)
├── BD_New_York/Conetnedor/            # Docker Compose (SQL Server 2022) + scripts SQL
├── FrontendCityNewYork/               # Vue 3 + Vite + Pinia + Vue Router
├── Componentes_fisicos/               # Sketches Arduino / ESP32
└── AGENTS.md
```

---

## Backend (ASP.NET Core 8) ✅

Ubicación: `Bakend_New_York/API_rest_City/`

```
Controllers/   Auth, Instrucciones, Puente, Teleferico, Elevador, Test
Services/      InstruccionService (cola de instrucciones), JwtService
BD/            DBContext (Dapper + SqlClient)
Models/ DTO/   Login, Usuario, Rol, requests/responses
```

### Módulos y endpoints

| Módulo | Ruta | Método | Descripción |
|--------|------|--------|-------------|
| **Auth** | `/api/auth/login` | POST | Devuelve `{ token, usuario }` |
| | `/api/auth/registrar` | POST | Registro de usuario |
| | `/api/auth/roles` | GET | Catálogo de roles |
| **Instrucciones** | `/api/instrucciones` | GET | Catálogo de instrucciones activas |
| | `/api/instrucciones` | POST | Encola `{ "nombre": "SERVO_SUBIR" }` (crea la instrucción si no existe) |
| | `/api/instrucciones/pendientes` | GET | **Polling del ESP32** (1 instrucción por llamada) |
| | `/api/instrucciones/respuesta` | POST | ESP32 reporta `{ idPendiente, resultado, angulo, ipDispositivo }` |
| | `/api/instrucciones/local` | POST | Notifica ejecución por botón físico (`tipo_ejecucion = LOCAL`) |
| | `/api/instrucciones/log` | GET | Historial global (`?limite=20`) |
| **Puente** | `/api/puente/abrir` | POST | Encola `SERVO_SUBIR` |
| | `/api/puente/cerrar` | POST | Encola `SERVO_BAJAR` |
| | `/api/puente/estado` | GET | `{ estado, angulo, megaOnline, esclavoOnline }` |
| | `/api/puente/log` | GET | Historial |
| **Teleférico** | `/api/teleferico/avanzar` | POST | Body opcional `3000` (pasos; por defecto 3000) |
| | `/api/teleferico/regresar` | POST | Body opcional `3000` (pasos; por defecto 3000) |
| | `/api/teleferico/parar` | POST | Detiene el motor |
| | `/api/teleferico/log` | GET | Historial filtrado `TELEFERICO%` |
| **Elevador** | `/api/elevador/subir` | POST | Body opcional `niveles` (por defecto 1) |
| | `/api/elevador/bajar` | POST | Body opcional `niveles` (por defecto 1) |
| | `/api/elevador/ir` | POST | `{ "nivel": 3 }` (rango 0–5) |
| | `/api/elevador/parar` | POST | Detiene el elevador |
| | `/api/elevador/calibrar` | POST | Baja hasta piso 0 (fin de carrera) |
| | `/api/elevador/estado` | GET | `{ nivel, megaOnline, esclavoOnline, ultimaInstruccion }` |
| | `/api/elevador/log` | GET | Historial filtrado por nivel |

> El body de `avanzar`/`regresar` es un entero JSON crudo (`3000`, no `{ "pasos": 3000 }`) porque usa `[FromBody] int?`.

### Servicio de instrucciones

`InstruccionService.CrearPendiente(nombre, descripcion?)` es el **único punto de encolado**:
1. Busca la instrucción en el catálogo con bloqueo (`UPDLOCK, HOLDLOCK`).
2. Si no existe, la crea automáticamente (`activa = 1`).
3. Evita duplicados: si ya hay una pendiente sin procesar devuelve `YaEstabaPendiente = true`.
4. Los movimientos largos (`TELEFERICO_*`, `SUBIR_NIVEL`, `BAJAR_NIVEL`, `IR_A_NIVEL`, `CALIBRAR`) también consideran duplicado si se procesaron hace menos de **15 s**.

`GET /api/instrucciones/pendientes` usa transacción `ReadCommitted`, `TOP 1` con `UPDLOCK`, y reentrega una instrucción si quedó colgada más de **30 s** (`fecha_procesamiento`).

### Autenticación

- JWT emitido por `JwtService`, guardado en `localStorage` y enviado como `Authorization: Bearer <token>`.
- Guards en el router: `requiresAuth` → `/login`, `requiresGuest` → `/`.

---

## Base de datos (SQL Server 2022 en Docker) ✅

`BD_New_York/Conetnedor/`

| Script | Contenido |
|--------|-----------|
| `Setup_Completo_BD.sql` | Esquema completo (usuarios, roles, log) |
| `Setup_Instrucciones.sql` | Versión detallada del sistema de instrucciones |
| `Setup_Instrucciones_Simple.sql` | **Versión vigente**: tablas + catálogo inicial |
| `StoredProcedures_Auth.sql` | SPs de autenticación |
| `StoredProcedures_Instrucciones.sql` | SPs de instrucciones |

### Tablas

| Tabla | Función |
|-------|---------|
| `Instruccion` | Catálogo (`id_instruccion`, `nombre` UNIQUE, `descripcion`, `activa`) |
| `Instruccion_Pendiente` | Cola (`id_pendiente`, `id_instruccion`, `procesada`, `fecha_procesamiento`) |
| `Log_Instruccion` | Historial (`nombre_instruccion`, `resultado`, `angulo_resultado`, `ip_dispositivo`, `tipo_ejecucion` = `REMOTA`/`LOCAL`, `fecha_ejecucion`) |

> `angulo_resultado` se reutiliza como **valor numérico genérico**: ángulo del servo, pasos del teleférico o nivel del elevador.

### Catálogo de instrucciones

| Nombre | Descripción |
|--------|-------------|
| `SERVO_SUBIR` | Abrir puente elevadizo |
| `SERVO_BAJAR` | Cerrar puente elevadizo |
| `SERVO_STOP` | Detener servo |
| `PING` | Verificar conexión |
| `TELEFERICO_AVANZAR [N]` | Avanzar N pasos |
| `TELEFERICO_REGRESAR [N]` | Regresar N pasos |
| `TELEFERICO_PARAR` | Detener motor |
| `SUBIR_NIVEL N` / `BAJAR_NIVEL N` | Elevador N niveles |
| `IR_A_NIVEL N` | Elevador al piso N (0–5) |
| `ELEVADOR_PARAR` | Detener elevador |
| `ELEVADOR_STATUS` | Reportar estado |
| `CALIBRAR` | Bajar a piso 0 hasta fin de carrera |

Los comandos con parámetro (`SUBIR_NIVEL 2`) se crean solos al encolarse.

---

## Hardware ✅

### Arquitectura general

```
Frontend (Vue)
    ↓ HTTP
Backend (ASP.NET)  ←── polling cada 5 s ── ESP32 (WiFi, ngrok)
    ↑                                         ↓ UART (TXS0108E, 9600 baud)
    └──────── /respuesta ─────────────────────┤
                                              Arduino (I2C maestro)
                                              ├─ Esclavo 0x08 → Servo puente + Stepper teleférico
                                              └─ Esclavo 0x09 → Stepper elevador (+ LED)
```

### Flujo de una instrucción

```
Frontend → Backend: POST /api/puente/abrir
Backend → BD: inserta en Instruccion_Pendiente
ESP32 ← Backend: GET /api/instrucciones/pendientes (cada 5 s)
ESP32 → Arduino: "SERVO_SUBIR" (tal cual, sin traducción)
Arduino → Esclavo: "SERVO_SUBIR" por I2C
Esclavo → Arduino: "POS:115" (estado del servo)
Arduino → ESP32: "RESP POS:115"
ESP32 → Backend: POST /api/instrucciones/respuesta
Backend → BD: actualiza Instruccion_Pendiente + inserta en Log_Instruccion
```

### Puente elevadizo

| Elemento | Detalle |
|----------|---------|
| Servo | MG996R en **pin 6** del esclavo |
| `ANGULO_ABIERTO` | **115°** |
| `ANGULO_CERRADO` | **180°** |
| I2C | Dirección **0x08** |
| Movimiento | Gradual, 1° cada 20 ms (no bloqueante) |
| Estados reportados | `ABIERTO` (110–130°), `CERRADO` (≥170°), `INTERMEDIO` |

### Teleférico

| Elemento | Detalle |
|----------|---------|
| Motor | Stepper 28BYJ-48 + ULN2003 en **pines 8, 9, 10, 11** |
| Orden de bobinas | `IN1, IN3, IN2, IN4` |
| `STEPS_VUELTA` | 10500 pasos |
| Velocidad | 5 RPM |
| Pasos por defecto (API) | 3000 |
| Comandos | `TELEFERICO_AVANZAR [N]`, `TELEFERICO_REGRESAR [N]`, `TELEFERICO_PARAR`, `TELEFERICO_STATUS` |

Mismo esclavo 0x08 que el puente; el motor avanza un paso por iteración de `loop()` (no bloqueante).

### Elevador (Empire State)

| Elemento | Detalle |
|----------|---------|
| Actuador | Stepper 28BYJ-48 + ULN2003 en **pines 8, 9, 10, 11** |
| I2C | Dirección **0x09** |
| `STEPS_POR_VUELTA` | 2048 |
| `PASOS_POR_NIVEL` | 5000 |
| Niveles | 0 a 5 (6 pisos) |
| Fines de carrera | Piso 0 → **pin 12**, piso máx → **pin 13** (`INPUT_PULLUP`) |
| LED indicador | Pin **4** (`Mega_NY.ino`) / pin **3** (`esclavo_Empire_State.ino`), parpadea 1 s |
| Reporte I2C | `NIVEL:03\|PAS:xxxxx` |

### ESP32 gateway

Dos sketches, el vigente es `Esp32_NY.ino` (puente + elevador):

| Elemento | Detalle |
|----------|---------|
| UART al Arduino | `Serial2` RX **16**, TX **17**, 9600 baud |
| Botones locales | GPIO **32** (abrir), GPIO **33** (cerrar) |
| Polling backend | 5000 ms |
| Keepalive al Arduino | `PING` cada 3000 ms |
| Timeout Arduino offline | 10000 ms |
| WiFi | SSID `Omega` / pass `d5adc4a32689` |
| Backend | `https://warless-predestinately-bethann.ngrok-free.dev` |

Endpoints HTTP locales (puerto 80):

| Ruta | Método | Descripción |
|------|--------|-------------|
| `/abrir` | POST | Servo a 115° |
| `/cerrar` | POST | Servo a 180° |
| `/estado` | GET | `{ estado, angulo, megaOnline, esclavoOnline }` |
| `/actualizar?angulo=n` | POST | Servo a n grados (0–180) |
| `/verificar` | GET | Heartbeat del ESP32 |
| `/elevador/subir?niveles=n` | POST | Subir n niveles |
| `/elevador/bajar?niveles=n` | POST | Bajar n niveles |
| `/elevador/ir?nivel=n` | POST | Ir al nivel n (0–5) |
| `/elevador/parar` | POST | Detener elevador |
| `/elevador/status` | GET | `{ nivel, pasosRestantes, esclavo2Online }` |
| `/elevador/calibrar` | POST | Calibrar a piso 0 |

### Inventario de firmware

| Archivo | Rol | Estado |
|---------|-----|--------|
| `Componentes_fisicos/Esp32_NY/Esp32_NY.ino` | Gateway WiFi puente + elevador | ✅ vigente |
| `Componentes_fisicos/Esp32_gateway/Esp32_gateway.ino` | Gateway WiFi solo puente | ⚠️ versión anterior |
| `Componentes_fisicos/esclavo_frontera/esclavo_frontera.ino` | Esclavo 0x08: servo + teleférico | ✅ |
| `Componentes_fisicos/esclavo_Empire_State/esclavo_Empire_State.ino` | Esclavo 0x09: elevador | ✅ |
| `Componentes_fisicos/Mega_NY/Mega_NY.ino` | Esclavo 0x09 (duplicado del anterior, LED en pin 4) | ⚠️ nombre engañoso |
| `Componentes_fisicos/test/test.ino` | Pruebas | — |

> ⚠️ **Pendiente:** el sketch del **Arduino maestro I2C** (el que recibe por `Serial1` desde el ESP32 y hace `Wire.beginTransmission` hacia 0x08/0x09) no está en el repositorio. `Mega_NY.ino` contiene en realidad el código del esclavo del elevador.

### Observaciones técnicas

- `PuenteController.ObtenerEstado()` y `ObtenerLog()` leen `Log_Instruccion` **sin filtrar por módulo**, así que pueden devolver registros del teleférico o del elevador. Los controllers de teleférico y elevador sí filtran por nombre.
- `PuenteController` inserta directo en `Instruccion_Pendiente` (no pasa por `InstruccionService`), por lo que no aplica la deduplicación.
- `ElevadorController.ObtenerEstado()` deduce `megaOnline`/`esclavoOnline` del último log, no de un heartbeat real.

---

## Central Park 🚧

Módulo nuevo. Página existente: `FrontendCityNewYork/src/pages/CentralParkPage.vue` (hoy solo maqueta visual con estadísticas estáticas).

### Jardín + Fuente 🚧

Área ajardinada dentro de Central Park con una fuente controlable desde el frontend.

**Especificación propuesta**

| Elemento | Detalle propuesto |
|----------|-------------------|
| Actuador | Bomba de agua 5–12 V vía módulo relé |
| Control | Esclavo I2C dedicado (**0x0A**) o relé directo en el gateway |
| Seguridad | Sensor de nivel de agua (flotador) para no operar la bomba en seco |
| Iluminación | Tira LED del jardín (opcional, mismo esclavo) |
| Instrucciones | `FUENTE_ENCENDER`, `FUENTE_APAGAR`, `FUENTE_PULSO N` (N segundos), `JARDIN_LUCES_ON`, `JARDIN_LUCES_OFF` |
| Endpoints | `POST /api/jardin/fuente/encender`, `POST /api/jardin/fuente/apagar`, `GET /api/jardin/estado`, `GET /api/jardin/log` |
| UI | Tarjeta de control de fuente en `CentralParkPage.vue` (encender/apagar, estado de bomba y nivel de agua) |

### Sistema de semáforos 🚧

Semáforos vehiculares/peatonales de la ciudad, controlados manual o automáticamente.

**Especificación propuesta**

| Elemento | Detalle propuesto |
|----------|-------------------|
| Luces | Rojo / Amarillo / Verde por carril (LED) |
| Control | Esclavo I2C dedicado (**0x0A** o **0x0B**) |
| Modos | `MANUAL` (color fijo) y `AUTO` (ciclo temporizado) |
| Ciclo AUTO | Verde 8 s → Amarillo 2 s → Rojo 8 s (configurable) |
| Instrucciones | `SEMAFORO_NS_ROJO/AMARILLO/VERDE`, `SEMAFORO_EO_ROJO/AMARILLO/VERDE`, `SEMAFORO_MODO_AUTO`, `SEMAFORO_PARAR` |
| Endpoints | `POST /api/semaforo/cambiar` (`{ carril, color }`), `POST /api/semaforo/auto`, `POST /api/semaforo/parar`, `GET /api/semaforo/estado`, `GET /api/semaforo/log` |
| UI | Nueva página `SemaforoPage.vue`, ruta `/semaforo`, dentro del menú **Punto de Entrada** |

> Los nombres, pines, direcciones I2C y tiempos son **propuestas** alineadas con la arquitectura actual; se confirman al implementar. Ambos módulos reutilizan `InstruccionService` y las mismas tres tablas, sin cambios de esquema (el color/estado se guarda en `angulo_resultado` como código numérico o en `resultado` como texto).

---

## Frontend (Vue 3 + Vite + Pinia) ✅

### Rutas

| Ruta | Página | Módulo |
|------|--------|--------|
| `/login` | `LoginPage.vue` | Auth (guest) |
| `/register` | `RegisterPage.vue` | Auth (guest) |
| `/` | `DashboardPage.vue` | Control de Ciudad |
| `/central-park` | `CentralParkPage.vue` | Central Park (🚧 jardín/fuente) |
| `/puente-elevadizo` | `PuenteElevadizoPage.vue` | Punto de Entrada |
| `/teleferico` | `TelefericoPage.vue` | Punto de Entrada |
| `/empire-state/elevador` | `ElevadorPage.vue` | Empire State (🚧 semáforos por definir) |

Cualquier ruta desconocida redirige a `/`.

### Menú lateral (`DashboardLayout.vue`)

```
Control de Ciudad          → /
Control de Central Park    → /central-park
Punto de Entrada  ▾
  ├─ Puente Elevadizo      → /puente-elevadizo
  └─ Teleférico            → /teleferico
Empire State  ▾
  └─ Elevador              → /empire-state/elevador
```

### Estructura en capas

```
FrontendCityNewYork/src/
├── api/          http.ts (axios + interceptors JWT), authService.ts
├── layouts/      DashboardLayout.vue, AuthLayout.vue
├── pages/        Login, Register, Dashboard, CentralPark,
│                 PuenteElevadizo, Teleferico, Elevador
├── router/       index.ts (rutas + guards)
├── stores/       auth.ts (Pinia)
├── types/        auth.ts
├── style.css     variables NYC + animaciones
├── App.vue
└── package.json
```

### Diseño

- Empresarial moderno, paleta NYC: taxi amarillo, navy, acero, rojo ladrillo.
- Skyline SVG animado de Manhattan, taxis y estrellas.
- Animaciones `fade-in-up`, `slide`, `scale` con `delay-1..4`.
- Responsive móvil/escritorio.

```css
--nyc-navy: #0A1628;
--nyc-taxi: #FFC107;
--nyc-red: #C53030;
--nyc-steel: #4A5568;
```

---

## Configuración y ejecución

### Backend

```bash
cd Bakend_New_York/API_rest_City
dotnet run
```

- HTTP `http://localhost:5238` · HTTPS `https://localhost:7137`
- Endpoint por defecto: `/weatherforecast`

```bash
dotnet build Bakend_New_York/API_rest_City/API_rest_City.csproj
```

### Base de datos

```bash
cd BD_New_York/Conetnedor
docker compose up -d
```

- Host `1444` → contenedor `1433`
- SA password: `Arquitectura2_New_york` (solo desarrollo)

### Frontend

```bash
cd FrontendCityNewYork
bun install     # solo la primera vez
bun run dev     # http://localhost:3000
```

### Hardware / red

- Backend expuesto por ngrok: `https://warless-predestinately-bethann.ngrok-free.dev`
- WiFi ESP32: `Omega` / `d5adc4a32689`
- Al cambiar la URL de ngrok hay que reflashear tanto el ESP32 como `PuenteController.esp32BaseUrl`.
