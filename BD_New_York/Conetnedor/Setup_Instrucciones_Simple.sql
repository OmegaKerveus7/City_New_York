-- =====================================================
-- Script SIMPLIFICADO para sistema de instrucciones
-- Ciudad New York - Puente Elevadizo
-- Compatible con DBeaver
-- =====================================================

-- =====================================================
-- 1. Crear tabla Instruccion (catalogo simple)
-- =====================================================
IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'Instruccion')
BEGIN
    CREATE TABLE Instruccion (
        id_instruccion INT IDENTITY(1,1) PRIMARY KEY,
        nombre VARCHAR(50) NOT NULL UNIQUE,
        descripcion VARCHAR(255) NOT NULL,
        activa BIT NOT NULL DEFAULT 1,
        fecha_creacion DATETIME DEFAULT GETDATE()
    );
END;

-- =====================================================
-- 2. Crear tabla Instruccion_Pendiente (cola)
-- =====================================================
IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'Instruccion_Pendiente')
BEGIN
    CREATE TABLE Instruccion_Pendiente (
        id_pendiente INT IDENTITY(1,1) PRIMARY KEY,
        id_instruccion INT NOT NULL,
        fecha_creacion DATETIME DEFAULT GETDATE(),
        procesada BIT NOT NULL DEFAULT 0,
        fecha_procesamiento DATETIME NULL,
        FOREIGN KEY (id_instruccion) REFERENCES Instruccion(id_instruccion)
    );
END;

-- =====================================================
-- 3. Crear tabla Log_Instruccion (historial)
-- =====================================================
IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'Log_Instruccion')
BEGIN
    CREATE TABLE Log_Instruccion (
        id_log INT IDENTITY(1,1) PRIMARY KEY,
        nombre_instruccion VARCHAR(50) NOT NULL,
        resultado VARCHAR(50) NOT NULL,
        angulo_resultado INT NULL,
        ip_dispositivo VARCHAR(50) NULL,
        tipo_ejecucion VARCHAR(20) NOT NULL,
        fecha_ejecucion DATETIME DEFAULT GETDATE()
    );
END;

-- =====================================================
-- 4. Insertar instrucciones iniciales
-- =====================================================
IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'SERVO_SUBIR')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('SERVO_SUBIR', 'Subir puente elevadizo');
IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'SERVO_BAJAR')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('SERVO_BAJAR', 'Bajar puente elevadizo');
IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'SERVO_STOP')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('SERVO_STOP', 'Detener servo');
IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'PING')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('PING', 'Verificar conexion');

-- =====================================================
-- Instrucciones del Elevador del Empire State
-- (Los comandos con parametro como SUBIR_NIVEL 2 o IR_A_NIVEL 3
--  se crean automaticamente al encolarse)
-- =====================================================
IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'SUBIR_NIVEL')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('SUBIR_NIVEL', 'Subir N niveles del elevador');
IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'BAJAR_NIVEL')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('BAJAR_NIVEL', 'Bajar N niveles del elevador');
IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'IR_A_NIVEL')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('IR_A_NIVEL', 'Ir al nivel N del elevador');
IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'ELEVADOR_PARAR')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('ELEVADOR_PARAR', 'Detener elevador');
IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'ELEVADOR_STATUS')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('ELEVADOR_STATUS', 'Reportar estado del elevador');
IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'CALIBRAR')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('CALIBRAR', 'Calibrar elevador (bajar hasta piso 0)');

-- =====================================================
-- 5. Central Park: fuente y luces del jardin
--    (Los comandos con parametro como FUENTE_PULSO 10
--     se crean automaticamente al encolarse)
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
-- 6. Semaforos
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
-- 7. Estado general de la maquina (telemetria al dashboard)
-- =====================================================
IF NOT EXISTS (SELECT 1 FROM Instruccion WHERE nombre = 'STATUS_ALL')
    INSERT INTO Instruccion (nombre, descripcion) VALUES ('STATUS_ALL', 'Pedir el estado general de todos los componentes');

-- =====================================================
-- 8. El log guarda el nivel del elevador aparte del angulo
--    (la tabla Estado_Componente se crea en Setup_CentralPark_Estado.sql)
-- =====================================================
IF NOT EXISTS (
    SELECT 1 FROM sys.columns
    WHERE name = 'nivel_resultado' AND object_id = OBJECT_ID('Log_Instruccion')
)
BEGIN
    ALTER TABLE Log_Instruccion ADD nivel_resultado INT NULL;
END;
