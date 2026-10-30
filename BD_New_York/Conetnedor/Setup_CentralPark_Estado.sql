-- =====================================================
-- Central Park (fuente + luces) y estado en vivo de componentes
-- Ciudad New York - Compatible con DBeaver / SQL Server 2022
-- =====================================================
-- Ejecutar este script sobre la base Ciudad_New_York.
-- Es idempotente: se puede volver a ejecutar sin romper nada.
-- =====================================================

-- =====================================================
-- 1. Tabla Estado_Componente (telemetría del ESP32)
--    Una fila por componente con clave "MODULO.CAMPO"
--    Ejemplos: PUENTE.ANGULO, ELEVADOR.NIVEL, FUENTE.BOMBA, SEMAFORO.NS
-- =====================================================
IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'Estado_Componente')
BEGIN
    CREATE TABLE Estado_Componente (
        clave VARCHAR(50) NOT NULL PRIMARY KEY,
        modulo VARCHAR(30) NOT NULL,
        valor VARCHAR(50) NULL,
        valor_numero INT NULL,
        texto VARCHAR(255) NULL,
        ip_dispositivo VARCHAR(50) NULL,
        fecha_actualizacion DATETIME NOT NULL DEFAULT GETDATE()
    );
    PRINT 'Tabla Estado_Componente creada';
END;

-- =====================================================
-- 2. El log guarda ahora el nivel del elevador aparte del angulo
-- =====================================================
IF NOT EXISTS (
    SELECT 1 FROM sys.columns
    WHERE name = 'nivel_resultado' AND object_id = OBJECT_ID('Log_Instruccion')
)
BEGIN
    ALTER TABLE Log_Instruccion ADD nivel_resultado INT NULL;
    PRINT 'Columna nivel_resultado agregada a Log_Instruccion';
END;

-- =====================================================
-- 3. Instrucciones del Central Park
--    (FUENTE_PULSO n se crea sola al encolarse)
-- =====================================================
IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'FUENTE_ENCENDER')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('FUENTE_ENCENDER', 'Encender la fuente del Central Park');

IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'FUENTE_APAGAR')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('FUENTE_APAGAR', 'Apagar la fuente del Central Park');

IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'FUENTE_PULSO')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('FUENTE_PULSO', 'Encender la fuente N segundos y apagarla');

IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'FUENTE_STATUS')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('FUENTE_STATUS', 'Reportar estado de la fuente y el sensor de agua');

IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'JARDIN_LUCES_ON')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('JARDIN_LUCES_ON', 'Encender las luces del jardin');

IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'JARDIN_LUCES_OFF')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('JARDIN_LUCES_OFF', 'Apagar las luces del jardin');

-- =====================================================
-- 4. Instrucciones de los semáforos
-- =====================================================
IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'SEMAFORO_NS_ROJO')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('SEMAFORO_NS_ROJO', 'Semaforo Norte-Sur en rojo');

IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'SEMAFORO_NS_AMARILLO')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('SEMAFORO_NS_AMARILLO', 'Semaforo Norte-Sur en amarillo');

IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'SEMAFORO_NS_VERDE')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('SEMAFORO_NS_VERDE', 'Semaforo Norte-Sur en verde');

IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'SEMAFORO_EO_ROJO')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('SEMAFORO_EO_ROJO', 'Semaforo Este-Oeste en rojo');

IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'SEMAFORO_EO_AMARILLO')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('SEMAFORO_EO_AMARILLO', 'Semaforo Este-Oeste en amarillo');

IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'SEMAFORO_EO_VERDE')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('SEMAFORO_EO_VERDE', 'Semaforo Este-Oeste en verde');

IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'SEMAFORO_AUTO')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('SEMAFORO_AUTO', 'Semaforos en modo automatico (ciclo temporizado)');

IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'SEMAFORO_PARAR')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('SEMAFORO_PARAR', 'Detener el ciclo automatico de los semaforos');

IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'SEMAFORO_STATUS')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('SEMAFORO_STATUS', 'Reportar estado de los semaforos');

-- =====================================================
-- 5. Instruccion de estado general (la pide el ESP32 cada 5 s)
-- =====================================================
IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'STATUS_ALL')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('STATUS_ALL', 'Pedir el estado general de todos los componentes');

-- =====================================================
-- 6. Estado inicial (el ESP32 lo va sobrescribiendo con datos reales)
-- =====================================================
IF NOT EXISTS (SELECT 1 FROM Estado_Componente WHERE clave = 'PUENTE.ANGULO')
    INSERT INTO Estado_Componente (clave, modulo, valor, texto) VALUES ('PUENTE.ANGULO', 'PUENTE', '0', 'Sin datos');

IF NOT EXISTS (SELECT 1 FROM Estado_Componente WHERE clave = 'PUENTE.ESTADO')
    INSERT INTO Estado_Componente (clave, modulo, valor, texto) VALUES ('PUENTE.ESTADO', 'PUENTE', 'DESCONOCIDO', 'Sin datos');

IF NOT EXISTS (SELECT 1 FROM Estado_Componente WHERE clave = 'TELEFERICO.PASOS')
    INSERT INTO Estado_Componente (clave, modulo, valor, texto) VALUES ('TELEFERICO.PASOS', 'TELEFERICO', '0', 'Sin datos');

IF NOT EXISTS (SELECT 1 FROM Estado_Componente WHERE clave = 'TELEFERICO.MOVIMIENTO')
    INSERT INTO Estado_Componente (clave, modulo, valor, texto) VALUES ('TELEFERICO.MOVIMIENTO', 'TELEFERICO', 'DETENIDO', 'Sin datos');

IF NOT EXISTS (SELECT 1 FROM Estado_Componente WHERE clave = 'ELEVADOR.NIVEL')
    INSERT INTO Estado_Componente (clave, modulo, valor, texto) VALUES ('ELEVADOR.NIVEL', 'ELEVADOR', '0', 'Sin datos');

IF NOT EXISTS (SELECT 1 FROM Estado_Componente WHERE clave = 'FUENTE.BOMBA')
    INSERT INTO Estado_Componente (clave, modulo, valor, texto) VALUES ('FUENTE.BOMBA', 'FUENTE', 'OFF', 'Sin datos');

IF NOT EXISTS (SELECT 1 FROM Estado_Componente WHERE clave = 'FUENTE.AGUA')
    INSERT INTO Estado_Componente (clave, modulo, valor, texto) VALUES ('FUENTE.AGUA', 'FUENTE', '0', 'Sin datos');

IF NOT EXISTS (SELECT 1 FROM Estado_Componente WHERE clave = 'FUENTE.LUCES')
    INSERT INTO Estado_Componente (clave, modulo, valor, texto) VALUES ('FUENTE.LUCES', 'FUENTE', 'OFF', 'Sin datos');

IF NOT EXISTS (SELECT 1 FROM Estado_Componente WHERE clave = 'SEMAFORO.NS')
    INSERT INTO Estado_Componente (clave, modulo, valor, texto) VALUES ('SEMAFORO.NS', 'SEMAFORO', 'ROJO', 'Sin datos');

IF NOT EXISTS (SELECT 1 FROM Estado_Componente WHERE clave = 'SEMAFORO.EO')
    INSERT INTO Estado_Componente (clave, modulo, valor, texto) VALUES ('SEMAFORO.EO', 'SEMAFORO', 'ROJO', 'Sin datos');

IF NOT EXISTS (SELECT 1 FROM Estado_Componente WHERE clave = 'SEMAFORO.MODO')
    INSERT INTO Estado_Componente (clave, modulo, valor, texto) VALUES ('SEMAFORO.MODO', 'SEMAFORO', 'MANUAL', 'Sin datos');

PRINT '=============================================';
PRINT ' Central Park + estado de componentes listo';
PRINT '=============================================';
