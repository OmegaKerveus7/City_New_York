-- =====================================================
-- Script para crear tablas del sistema de instrucciones
-- Ciudad New York - Puente Elevadizo
-- =====================================================

USE Ciudad_New_York;
GO

-- =====================================================
-- 1. Crear tabla Instruccion
-- =====================================================
IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'Instruccion')
BEGIN
    CREATE TABLE Instruccion (
        id_instruccion INT IDENTITY(1,1) PRIMARY KEY,
        codigo VARCHAR(20) NOT NULL UNIQUE,
        componente VARCHAR(10) NOT NULL,  -- 'Esp', 'Meg', 'Esc1', etc.
        numero_instruccion VARCHAR(10) NOT NULL,
        descripcion VARCHAR(255) NOT NULL,
        parametros VARCHAR(MAX) NULL,  -- JSON con parametros esperados
        activa BIT NOT NULL DEFAULT(1),
        fecha_creacion DATETIME DEFAULT GETDATE()
    );

    -- Insertar instrucciones iniciales para el puente elevadizo
    INSERT INTO Instruccion (codigo, componente, numero_instruccion, descripcion, parametros) VALUES
    ('Esc1X0100', 'Esc1', '0100', 'Subir puente elevadizo (ABRIR)', '{"angulo": 85}'),
    ('Esc1X0200', 'Esc1', '0200', 'Bajar puente elevadizo (CERRAR)', '{"angulo": 180}'),
    ('MegX0100', 'Meg', '0100', 'Iniciar comunicacion Mega', '{}'),
    ('EspX0100', 'Esp', '0100', 'Verificar conexion ESP32', '{}'),
    ('Esc1X0300', 'Esc1', '0300', 'Detener servo', '{"angulo": 90}');

    PRINT 'Tabla Instruccion creada con datos iniciales';
END
ELSE
BEGIN
    PRINT 'La tabla Instruccion ya existe';
END
GO

-- =====================================================
-- 2. Crear tabla Permiso
-- =====================================================
IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'Permiso')
BEGIN
    CREATE TABLE Permiso (
        id_permiso INT IDENTITY(1,1) PRIMARY KEY,
        id_rol INT NOT NULL,
        id_instruccion INT NOT NULL,
        puede_ejecutar BIT NOT NULL DEFAULT(1),
        FOREIGN KEY (id_rol) REFERENCES RolUsuario(id_rol),
        FOREIGN KEY (id_instruccion) REFERENCES Instruccion(id_instruccion),
        UNIQUE (id_rol, id_instruccion)
    );

    -- Dar permisos al rol Empleado (id_rol=2) para todas las instrucciones
    INSERT INTO Permiso (id_rol, id_instruccion, puede_ejecutar)
    SELECT 2, id_instruccion, 1 FROM Instruccion;

    -- Dar permisos al rol Administrador (id_rol=3) para todas las instrucciones
    INSERT INTO Permiso (id_rol, id_instruccion, puede_ejecutar)
    SELECT 3, id_instruccion, 1 FROM Instruccion;

    PRINT 'Tabla Permiso creada con permisos iniciales';
END
ELSE
BEGIN
    PRINT 'La tabla Permiso ya existe';
END
GO

-- =====================================================
-- 3. Crear tabla Log_Instruccion
-- =====================================================
IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'Log_Instruccion')
BEGIN
    CREATE TABLE Log_Instruccion (
        id_log INT IDENTITY(1,1) PRIMARY KEY,
        id_instruccion INT NOT NULL,
        id_usuario INT NULL,
        id_dispositivo INT NULL,  -- Para identificar ESP32 u otro dispositivo
        tipo_ejecucion VARCHAR(20) NOT NULL,  -- 'REMOTA' o 'LOCAL'
        fecha_ejecucion DATETIME DEFAULT GETDATE(),
        resultado VARCHAR(50) NOT NULL,  -- 'OK', 'ERROR', 'TIMEOUT'
        respuesta_dispositivo VARCHAR(500) NULL,
        ip_dispositivo VARCHAR(50) NULL,
        angulo_resultado INT NULL,
        tiempo_respuesta_ms INT NULL,
        FOREIGN KEY (id_instruccion) REFERENCES Instruccion(id_instruccion),
        FOREIGN KEY (id_usuario) REFERENCES Usuario(id_usuario)
    );

    PRINT 'Tabla Log_Instruccion creada';
END
ELSE
BEGIN
    PRINT 'La tabla Log_Instruccion ya existe';
END
GO

-- =====================================================
-- 4. Crear tabla Dispositivo (para registrar ESP32, Mega, etc.)
-- =====================================================
IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'Dispositivo')
BEGIN
    CREATE TABLE Dispositivo (
        id_dispositivo INT IDENTITY(1,1) PRIMARY KEY,
        nombre VARCHAR(50) NOT NULL UNIQUE,
        tipo VARCHAR(20) NOT NULL,  -- 'Esp', 'Meg', 'Esc1', etc.
        ip VARCHAR(50) NULL,
        mac_address VARCHAR(50) NULL,
        ultimo_ping DATETIME NULL,
        activo BIT NOT NULL DEFAULT(1),
        fecha_registro DATETIME DEFAULT GETDATE()
    );

    -- Registrar dispositivos iniciales
    INSERT INTO Dispositivo (nombre, tipo, activo) VALUES
    ('ESP32_Gateway', 'Esp', 1),
    ('Mega_Gateway', 'Meg', 1),
    ('Esclavo_1_Servo', 'Esc1', 1);

    PRINT 'Tabla Dispositivo creada con datos iniciales';
END
ELSE
BEGIN
    PRINT 'La tabla Dispositivo ya existe';
END
GO

-- =====================================================
-- 5. Crear tabla Instruccion_Pendiente (para cola de instrucciones)
-- =====================================================
IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'Instruccion_Pendiente')
BEGIN
    CREATE TABLE Instruccion_Pendiente (
        id_pendiente INT IDENTITY(1,1) PRIMARY KEY,
        id_instruccion INT NOT NULL,
        id_dispositivo_destino INT NOT NULL,
        parametros_ejecucion VARCHAR(MAX) NULL,
        creada_por INT NULL,
        fecha_creacion DATETIME DEFAULT GETDATE(),
        procesada BIT NOT NULL DEFAULT(0),
        fecha_procesamiento DATETIME NULL,
        FOREIGN KEY (id_instruccion) REFERENCES Instruccion(id_instruccion),
        FOREIGN KEY (id_dispositivo_destino) REFERENCES Dispositivo(id_dispositivo)
    );

    PRINT 'Tabla Instruccion_Pendiente creada';
END
ELSE
BEGIN
    PRINT 'La tabla Instruccion_Pendiente ya existe';
END
GO

PRINT '============================================';
PRINT 'Tablas de instrucciones creadas correctamente!';
PRINT '============================================';
