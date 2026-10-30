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
| **Estado** | `/api/estado` | POST | **Telemetría del ESP32**: `{ ipDispositivo, estado: { "CLAVE": "valor" } }` |
| | `/api/estado/todos` | GET | Estado en vivo de todos los componentes (dashboard) |
| | `/api/estado/{clave}` | GET | Estado de un componente (`FUENTE.BOMBA`) |
| **Jardín** | `/api/jardin/fuente/encender` | POST | Encola `FUENTE_ENCENDER` |
| | `/api/jardin/fuente/apagar` | POST | Encola `FUENTE_APAGAR` |
| | `/api/jardin/fuente/pulso` | POST | Body opcional `15` (segundos; 1–600) → `FUENTE_PULSO 15` |
| | `/api/jardin/fuente/status` | POST | Encola `FUENTE_STATUS` |
| | `/api/jardin/luces/encender` | POST | Encola `JARDIN_LUCES_ON` |
| | `/api/jardin/luces/apagar` | POST | Encola `JARDIN_LUCES_OFF` |
| | `/api/jardin/estado` | GET | Estado en vivo de la fuente (telemetría) |
| | `/api/jardin/log` | GET | Historial `FUENTE%` / `JARDIN%` |
| **Semáforo** | `/api/semaforo/cambiar` | POST | Body `{ "carril": "NS", "color": "VERDE" }` |
| | `/api/semaforo/auto` | POST | Encola `SEMAFORO_AUTO` |
| | `/api/semaforo/parar` | POST | Encola `SEMAFORO_PARAR` |
| | `/api/semaforo/status` | POST | Encola `SEMAFORO_STATUS` |
| | `/api/semaforo/estado` | GET | Estado en vivo de los semáforos |
| | `/api/semaforo/log` | GET | Historial `SEMAFORO%` |

> El body de `avanzar`/`regresar` es un entero JSON crudo (`3000`, no `{ "pasos": 3000 }`) porque usa `[FromBody] int?`.
> El body de `jardin/fuente/pulso` también es un entero crudo (`15`).
> Los controladores de **Jardín** y **Semáforo** reutilizan `InstruccionService`, por lo que heredan la deduplicación de la cola.

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
| `Setup_Instrucciones_Simple.sql` | **Versión vigente**: tablas + catálogo inicial (incluye Central Park y semáforos) |
| `Setup_CentralPark_Estado.sql` | **Nuevo**: tabla `Estado_Componente`, columna `nivel_resultado`, catálogo de Central Park/semáforos y estado inicial |
| `StoredProcedures_Auth.sql` | SPs de autenticación |
| `StoredProcedures_Instrucciones.sql` | SPs de instrucciones |

### Tablas

| Tabla | Función |
|-------|---------|
| `Instruccion` | Catálogo (`id_instruccion`, `nombre` UNIQUE, `descripcion`, `activa`) |
| `Instruccion_Pendiente` | Cola (`id_pendiente`, `id_instruccion`, `procesada`, `fecha_procesamiento`) |
| `Log_Instruccion` | Historial (`nombre_instruccion`, `resultado`, `angulo_resultado`, `nivel_resultado`, `ip_dispositivo`, `tipo_ejecucion` = `REMOTA`/`LOCAL`, `fecha_ejecucion`) |
| `Estado_Componente` | **Estado en vivo** (`clave` PK = `MODULO.CAMPO`, `modulo`, `valor`, `valor_numero`, `texto`, `ip_dispositivo`, `fecha_actualizacion`) |

> `angulo_resultado` se reutiliza como **valor numérico genérico**: ángulo del servo o pasos del teleférico. El nivel del elevador se guarda aparte en `nivel_resultado` (ambos los envía el ESP32 en la respuesta).
> `Estado_Componente` es la tabla que alimenta el dashboard: una fila por componente, actualizada por `POST /api/estado`.

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
| `FUENTE_ENCENDER` | Encender la fuente del Central Park |
| `FUENTE_APAGAR` | Apagar la fuente del Central Park |
| `FUENTE_PULSO N` | Encender la fuente N segundos y apagarla |
| `FUENTE_STATUS` | Reportar estado de la fuente y del sensor de agua |
| `JARDIN_LUCES_ON` / `JARDIN_LUCES_OFF` | Luces del jardín |
| `SEMAFORO_NS_ROJO/AMARILLO/VERDE` | Semáforo del carril Norte-Sur |
| `SEMAFORO_EO_ROJO/AMARILLO/VERDE` | Semáforo del carril Este-Oeste |
| `SEMAFORO_AUTO` / `SEMAFORO_PARAR` | Ciclo automático / detener ciclo |
| `SEMAFORO_STATUS` | Reportar estado de los semáforos |
| `STATUS_ALL` | Pedir el estado general de todos los componentes |

Los comandos con parámetro (`SUBIR_NIVEL 2`, `FUENTE_PULSO 10`) se crean solos al encolarse.

---

## Hardware ✅

### Arquitectura general

```
Frontend (Vue)
    ↓ HTTP
Backend (ASP.NET)  ←── polling cada 5 s ── ESP32 (WiFi, ngrok)
    ↑                                         ↓ UART (TXS0108E, 9600 baud)
    │                                         │
    └──── /respuesta + /estado ───────────────┤
                                              Arduino Mega (I2C maestro + semáforos)
                                              ├─ Esclavo 0x08 → Servo puente + Stepper teleférico
                                              ├─ Esclavo 0x09 → Stepper elevador (+ LED)
                                              ├─ Esclavo 0x0A → Fuente + luces del Central Park
                                              └─ Semáforos NS/EO (pines propios del Mega)
```

### Flujo de una instrucción

```
Frontend → Backend: POST /api/puente/abrir
Backend → BD: inserta en Instruccion_Pendiente
ESP32 ← Backend: GET /api/instrucciones/pendientes (cada 5 s)
ESP32 → Mega: "SERVO_SUBIR" (tal cual, sin traducción)
Mega → Esclavo 0x08: "SERVO_SUBIR" por I2C
Esclavo → Mega: "POS:115|TELEFERICO:DETENIDO|PAS:00000"
Mega → ESP32: "RESP POS:115|..."
ESP32 → Backend: POST /api/instrucciones/respuesta
Backend → BD: actualiza Instruccion_Pendiente + inserta en Log_Instruccion
```

### Flujo del estado en vivo (telemetría al dashboard)

```
ESP32 → Mega: "STATUS_ALL" (cada 5 s)
Mega → ESP32: "ESTADO:PUENTE.ANGULO=115;PUENTE.ESTADO=ABIERTO;...;SEMAFORO.MODO=MANUAL"
ESP32 → Backend: POST /api/estado  { ipDispositivo, estado: { "CLAVE": "valor" } }
Backend → BD: upsert en Estado_Componente (una fila por clave)
Frontend → Backend: GET /api/estado/todos  →  dashboard (refresca cada 5 s)
```

### Maestro I2C (Arduino Mega) ✅

`Componentes_fisicos/Mega_NY/Mega_NY.ino` — el Mega dejó de ser solo un repetidor: **también ejecuta trabajo propio** (semáforos) y **agrega el estado de todos los módulos**.

| Elemento | Detalle |
|----------|---------|
| UART con el ESP32 | `Serial1` a 9600 baud (pines 18/19) |
| Bus I2C | `Wire.begin()` como maestro (pines 20/21) |
| Routing | `SERVO_*`/`TELEFERICO_*` → **0x08** · `SUBIR_NIVEL`/`BAJAR_NIVEL`/`IR_A_NIVEL`/`ELEVADOR_*`/`CALIBRAR` → **0x09** · `CENTRAL_ON`/`CENTRAL_OFF`/`FUENTE_*`/`JARDIN_*` → **0x0A** |
| Comandos locales | `PING` → `CICLO_MEGA:<n>` · `STATUS_ALL` → `ESTADO:...` · `SEMAFORO_*` |
| Respuestas | `RESP <estado del esclavo>` · `RESP OK:<COMANDO>` · `RESP ERROR:<motivo>` |
| Presencia de esclavos | Cada 5 s comprueba 0x08/0x09/0x0A y emite `ESCLAVO1=OK`, `ESCLAVO2=OK`, `ESCLAVO3=OK` |
| Semáforos carril NS | Rojo **22**, Amarillo **24**, Verde **26** |
| Semáforos carril EO | Rojo **23**, Amarillo **27**, Verde **25** |
| Modos | `AUTO` (ciclo temporizado) y `MANUAL` (color fijo) |
| Ciclo automático | Verde 8 s → Amarillo 2 s → Rojo 8 s, alternando NS/EO (`TIEMPO_VERDE_MS` / `TIEMPO_AMARILLO_MS`) |
| LED de actividad | Pin **13** (LED integrado) |

### Central Park: fuente y luces ✅

Esclavo **0x0A** (`Componentes_fisicos/Central_Park/Central_Park.ino`):

| Elemento | Detalle |
|----------|---------|
| Relé de la bomba | **pin 2** (activo en **BAJO**: ON = LOW) |
| Sensor de nivel (flotador) | **pin 8** (`INPUT_PULLUP`, LOW = hay agua) |
| Relé de las luces del jardín | **pin 9** (activo en BAJO) |
| LED de estado | `LED_BUILTIN` (parpadea con la fuente encendida) |
| Nivel de agua | Sensor analógico en **A0** (0–100 %) |
| Seguridad | La bomba **no arranca en seco**; `SIMULAR_SENSOR_AGUA true` permite probar en banco |
| Comandos | `CENTRAL_ON`, `CENTRAL_OFF`, `FUENTE_ENCENDER`, `FUENTE_APAGAR`, `FUENTE_PULSO N`, `FUENTE_STATUS`, `JARDIN_LUCES_ON`, `JARDIN_LUCES_OFF`, `PING` |
| Reporte I2C | `FUENTE:ON\|AGUA:42\|LUCES:OFF\|FLOT:1` |

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

Dos sketches, el vigente es `Esp32_NY.ino` (puente + elevador + Central Park + telemetría):

| Elemento | Detalle |
|----------|---------|
| UART al Arduino | `Serial2` RX **16**, TX **17**, 9600 baud |
| Botones locales | GPIO **32** (abrir), GPIO **33** (cerrar) |
| Polling backend | 5000 ms (`GET /api/instrucciones/pendientes`) |
| Keepalive al Arduino | `PING` cada 3000 ms |
| Pedido de estado | `STATUS_ALL` cada **5000 ms** (solo si no está procesando un comando) |
| Telemetría al backend | `POST /api/estado` cada 5 s con el estado agregado del maestro |
| Timeout Arduino offline | 10000 ms |
| WiFi | SSID `Omega` / pass `d5adc4a32689` |
| Backend | `https://warless-predestinately-bethann.ngrok-free.dev` |

Endpoints HTTP locales (puerto 80):

| Ruta | Método | Descripción |
|------|--------|-------------|
| `/abrir` | POST | Servo a 115° |
| `/cerrar` | POST | Servo a 180° |
| `/estado` | GET | `{ estado, angulo, teleferico, pasos, nivel, fuente, semaforoNS, semaforoEO, semaforoModo, megaOnline, esclavoOnline }` |
| `/verificar` | GET | Heartbeat del ESP32 |
| `/fuente/encender` | POST | Enciende la fuente |
| `/fuente/apagar` | POST | Apaga la fuente |
| `/fuente/status` | GET | `{ bomba, agua, luces }` |
| `/luces/on` | POST | Enciende las luces del jardín |
| `/luces/off` | POST | Apaga las luces del jardín |
| `/semaforo/auto` | POST | Ciclo automático de semáforos |
| `/semaforo/parar` | POST | Detiene el ciclo automático |

### Inventario de firmware

| Archivo | Rol | Estado |
|---------|-----|--------|
| `Componentes_fisicos/Mega_NY/Mega_NY.ino` | **Maestro I2C del Mega**: routing a 0x08/0x09/0x0A, semáforos y estado general | ✅ vigente |
| `Componentes_fisicos/Esp32_NY/Esp32_NY.ino` | Gateway WiFi: puente, elevador, Central Park y telemetría | ✅ vigente |
| `Componentes_fisicos/Esp32_gateway/Esp32_gateway.ino` | Gateway WiFi solo puente | ⚠️ versión anterior |
| `Componentes_fisicos/esclavo_frontera/esclavo_frontera.ino` | Esclavo 0x08: servo + teleférico | ✅ |
| `Componentes_fisicos/esclavo_Empire_State/esclavo_Empire_State.ino` | Esclavo 0x09: elevador | ✅ |
| `Componentes_fisicos/Central_Park/Central_Park.ino` | Esclavo 0x0A: fuente + luces del jardín | ✅ |
| `Componentes_fisicos/test/test.ino` | Pruebas | — |

> El **maestro I2C es `Mega_NY/Mega_NY.ino`** (no hay un archivo aparte): recibe por `Serial1` desde el ESP32, reparte a 0x08/0x09/0x0A y maneja los semáforos con sus propios pines.

### Observaciones técnicas

- `PuenteController.ObtenerEstado()` y `ObtenerLog()` leen `Log_Instruccion` **sin filtrar por módulo**, así que pueden devolver registros del teleférico o del elevador. Los controllers de teleférico, elevador, jardín y semáforo sí filtran por nombre.
- `PuenteController` inserta directo en `Instruccion_Pendiente` (no pasa por `InstruccionService`), por lo que no aplica la deduplicación.
- `ElevadorController.ObtenerEstado()` deduce `megaOnline`/`esclavoOnline` del último log, no de un heartbeat real.
- El estado que sí viene del dispositivo es el de **telemetría** (`Estado_Componente`): lo reporta el ESP32 cada 5 s con la línea `ESTADO:` del maestro.
- El ESP32 ignora las líneas que no son `RESP ...` (PING/PONG y presencia de esclavos) cuando espera la respuesta a un comando.

---

## Central Park ✅

Módulo implementado: página `FrontendCityNewYork/src/pages/CentralParkPage.vue` con control de la fuente, las luces del jardín y lectura del sensor de agua.

### Jardín + Fuente ✅

| Elemento | Detalle implementado |
|----------|-----------------------|
| Actuador | Bomba de agua 5–12 V vía módulo relé (pin 7 del esclavo 0x0A) |
| Control | Esclavo I2C dedicado **0x0A** |
| Seguridad | Sensor de nivel de agua (flotador, pin 8): la bomba no arranca en seco |
| Iluminación | Luces del jardín en el mismo esclavo (pin 9) |
| Instrucciones | `FUENTE_ENCENDER`, `FUENTE_APAGAR`, `FUENTE_PULSO N`, `FUENTE_STATUS`, `JARDIN_LUCES_ON`, `JARDIN_LUCES_OFF` |
| Endpoints | `POST /api/jardin/fuente/encender|apagar|pulso|status`, `POST /api/jardin/luces/encender|apagar`, `GET /api/jardin/estado`, `GET /api/jardin/log` |
| UI | Tarjeta "Fuente y Jardín" en `CentralParkPage.vue` (encender/apagar, pulso temporizado, barra de nivel de agua, chips de estado) |

### Sistema de semáforos ✅

| Elemento | Detalle implementado |
|----------|-----------------------|
| Luces | Rojo / Amarillo / Verde por carril (LED en los pines del Mega) |
| Control | **Arduino Mega maestro** (no hace falta esclavo I2C): NS = 22/24/26, EO = 23/27/25 |
| Modos | `MANUAL` (color fijo) y `AUTO` (ciclo temporizado en el maestro) |
| Ciclo AUTO | Verde 8 s → Amarillo 2 s → Rojo 8 s, alternando NS/EO |
| Instrucciones | `SEMAFORO_NS_ROJO/AMARILLO/VERDE`, `SEMAFORO_EO_ROJO/AMARILLO/VERDE`, `SEMAFORO_AUTO`, `SEMAFORO_PARAR`, `SEMAFORO_STATUS` |
| Endpoints | `POST /api/semaforo/cambiar|auto|parar|status`, `GET /api/semaforo/estado`, `GET /api/semaforo/log` |
| UI | Nueva página `SemaforoPage.vue`, ruta `/semaforo`, dentro del menú **Punto de Entrada** |

> Ambos módulos reutilizan `InstruccionService` y las mismas tablas, sin cambios de esquema para las instrucciones. El color/estado del semáforo se guarda en `Estado_Componente` (`SEMAFORO.NS`, `SEMAFORO.EO`, `SEMAFORO.MODO`).

### Estado en vivo de la maqueta ✅ Nuevo

| Elemento | Detalle |
|----------|---------|
| Origen | El Mega responde `STATUS_ALL` con `ESTADO:clave=valor;clave=valor;...` |
| Transporte | El ESP32 lo convierte a JSON y lo manda a `POST /api/estado` |
| Persistencia | `Estado_Componente` (upsert por clave) |
| Consumo | `GET /api/estado/todos` alimenta el panel "Estado en Vivo" del dashboard |
| Claves | `PUENTE.ANGULO`, `PUENTE.ESTADO`, `TELEFERICO.PASOS`, `TELEFERICO.MOVIMIENTO`, `ELEVADOR.NIVEL`, `FUENTE.BOMBA`, `FUENTE.AGUA`, `FUENTE.LUCES`, `SEMAFORO.NS`, `SEMAFORO.EO`, `SEMAFORO.MODO` |

> Los nombres, pines, direcciones I2C y tiempos quedaron **confirmados al implementar** (ver las tablas de arriba). Central Park y semáforos reutilizan `InstruccionService` y las mismas tablas, sin duplicar cola.

---

## Frontend (Vue 3 + Vite + Pinia) ✅

### Rutas

| Ruta | Página | Módulo |
|------|--------|--------|
| `/login` | `LoginPage.vue` | Auth (guest) |
| `/register` | `RegisterPage.vue` | Auth (guest) |
| `/` | `DashboardPage.vue` | Control de Ciudad |
| `/central-park` | `CentralParkPage.vue` | Central Park (fuente + luces ✅) |
| `/puente-elevadizo` | `PuenteElevadizoPage.vue` | Punto de Entrada |
| `/teleferico` | `TelefericoPage.vue` | Punto de Entrada |
| `/empire-state/elevador` | `ElevadorPage.vue` | Empire State |
| `/semaforo` | `SemaforoPage.vue` | Semáforos (✅ nuevo) |

Cualquier ruta desconocida redirige a `/`.

### Menú lateral (`DashboardLayout.vue`)

```
Control de Ciudad          → /
Control de Central Park    → /central-park
Punto de Entrada  ▾
  ├─ Puente Elevadizo      → /puente-elevadizo
  ├─ Teleférico            → /teleferico
  └─ Semáforos             → /semaforo
Empire State  ▾
  └─ Elevador              → /empire-state/elevador
```

### Estructura en capas

```
FrontendCityNewYork/src/
├── api/          http.ts (axios + interceptors JWT), authService.ts,
│                 instruccionesService.ts, telefericoService.ts,
│                 elevadorService.ts, estadoService.ts,
│                 jardinService.ts, semaforoService.ts
├── layouts/      DashboardLayout.vue, AuthLayout.vue
├── pages/        Login, Register, Dashboard, CentralPark,
│                 PuenteElevadizo, Teleferico, Elevador, Semaforo
├── router/       index.ts (rutas + guards)
├── stores/       auth.ts (Pinia)
├── types/        auth.ts
├── style.css     variables NYC + animaciones
├── App.vue
└── package.json
```

### Estado en vivo (dashboard)

- `estadoService.obtenerTodos()` → `GET /api/estado/todos`, se refresca **cada 5 s**.
- `DashboardPage.vue` muestra el panel **"Estado en Vivo de la Ciudad"** con puente, teleférico, elevador, fuente y semáforos.
- `CentralParkPage.vue` y `SemaforoPage.vue` consultan su propio módulo (`/api/jardin/estado`, `/api/semaforo/estado`) cada 5 s.

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
- Scripts (idempotentes, se pueden volver a ejecutar):

```bash
# esquema + catálogo de instrucciones (incluye Central Park y semáforos)
sqlcmd -S "localhost,1444" -U SA -P "Arquitectura2_New_york" -d "Ciudad_New_York" -i "BD_New_York\Conetnedor\Setup_Instrucciones_Simple.sql"

# tabla Estado_Componente + columna nivel_resultado + estado inicial
sqlcmd -S "localhost,1444" -U SA -P "Arquitectura2_New_york" -d "Ciudad_New_York" -i "BD_New_York\Conetnedor\Setup_CentralPark_Estado.sql"
```

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
