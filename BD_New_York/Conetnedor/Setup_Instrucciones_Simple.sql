-- =====================================================
-- Script SIMPLIFICADO para sistema de instrucciones
-- Ciudad New York - Puente Elevadizo
-- Compatible con DBeaver
-- =====================================================

-- =====================================================
-- 1. Crear tabla Instruccion (catalogo simple)
-- =====================================================
CREATE TABLE IF NOT EXISTS Instruccion (
    id_instruccion INT IDENTITY(1,1) PRIMARY KEY,
    nombre VARCHAR(50) NOT NULL UNIQUE,
    descripcion VARCHAR(255) NOT NULL,
    activa BIT NOT NULL DEFAULT 1,
    fecha_creacion DATETIME DEFAULT GETDATE()
);

-- =====================================================
-- 2. Crear tabla Instruccion_Pendiente (cola)
-- =====================================================
CREATE TABLE IF NOT EXISTS Instruccion_Pendiente (
    id_pendiente INT IDENTITY(1,1) PRIMARY KEY,
    id_instruccion INT NOT NULL,
    fecha_creacion DATETIME DEFAULT GETDATE(),
    procesada BIT NOT NULL DEFAULT 0,
    fecha_procesamiento DATETIME NULL,
    FOREIGN KEY (id_instruccion) REFERENCES Instruccion(id_instruccion)
);

-- =====================================================
-- 3. Crear tabla Log_Instruccion (historial)
-- =====================================================
CREATE TABLE IF NOT EXISTS Log_Instruccion (
    id_log INT IDENTITY(1,1) PRIMARY KEY,
    nombre_instruccion VARCHAR(50) NOT NULL,
    resultado VARCHAR(50) NOT NULL,
    angulo_resultado INT NULL,
    ip_dispositivo VARCHAR(50) NULL,
    tipo_ejecucion VARCHAR(20) NOT NULL,
    fecha_ejecucion DATETIME DEFAULT GETDATE()
);

-- =====================================================
-- 4. Insertar instrucciones iniciales
-- =====================================================
INSERT INTO Instruccion (nombre, descripcion) VALUES ('SERVO_SUBIR', 'Subir puente elevadizo');
INSERT INTO Instruccion (nombre, descripcion) VALUES ('SERVO_BAJAR', 'Bajar puente elevadizo');
INSERT INTO Instruccion (nombre, descripcion) VALUES ('SERVO_STOP', 'Detener servo');
INSERT INTO Instruccion (nombre, descripcion) VALUES ('PING', 'Verificar conexion');
