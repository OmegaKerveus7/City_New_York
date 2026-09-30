-- =====================================================
-- Script completo para configurar la base de datos
-- Ciudad New York - Sistema de Autenticación
-- =====================================================

USE Ciudad_New_York;
GO

-- =====================================================
-- 1. Crear tabla RolUsuario si no existe
-- =====================================================
IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'RolUsuario')
BEGIN
    CREATE TABLE RolUsuario (
        id_rol INT IDENTITY(1,1) PRIMARY KEY,
        nombre_rol VARCHAR(100) NOT NULL,
        activo BIT NOT NULL DEFAULT(1)
    );

    -- Insertar roles iniciales
    -- El primer rol (ID=1) tendrá activo=0 para que no aparezca en registro público
    INSERT INTO RolUsuario (nombre_rol, activo) VALUES ('Usuario Básico', 0);
    INSERT INTO RolUsuario (nombre_rol, activo) VALUES ('Empleado', 1);
    INSERT INTO RolUsuario (nombre_rol, activo) VALUES ('Administrador', 1);

    PRINT 'Tabla RolUsuario creada con datos iniciales';
END
ELSE
BEGIN
    PRINT 'La tabla RolUsuario ya existe';
END
GO

-- =====================================================
-- 2. Crear tabla Usuario si no existe
-- =====================================================
IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'Usuario')
BEGIN
    CREATE TABLE Usuario (
        id_usuario INT IDENTITY(1,1) PRIMARY KEY,
        nombre_usuario VARCHAR(100) NOT NULL,
        apellidos_usuario VARCHAR(100) NOT NULL,
        correo VARCHAR(255) NOT NULL UNIQUE,
        contrasena VARCHAR(255) NOT NULL,
        activo BIT NOT NULL DEFAULT(1)
    );

    PRINT 'Tabla Usuario creada';
END
ELSE
BEGIN
    PRINT 'La tabla Usuario ya existe';
END
GO

-- =====================================================
-- 3. Crear tabla UsuarioRol si no existe
-- =====================================================
IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'UsuarioRol')
BEGIN
    CREATE TABLE UsuarioRol (
        id_usuario INT NOT NULL,
        id_rol_usuario INT NOT NULL,
        PRIMARY KEY (id_usuario, id_rol_usuario),
        FOREIGN KEY (id_usuario) REFERENCES Usuario(id_usuario),
        FOREIGN KEY (id_rol_usuario) REFERENCES RolUsuario(id_rol)
    );

    PRINT 'Tabla UsuarioRol creada';
END
ELSE
BEGIN
    PRINT 'La tabla UsuarioRol ya existe';
END
GO

-- =====================================================
-- 4. Crear vista VW_Usuario
-- =====================================================
IF EXISTS (SELECT 1 FROM sys.views WHERE name = 'VW_Usuario')
    DROP VIEW VW_Usuario;
GO

CREATE VIEW VW_Usuario
AS
SELECT
    u.id_usuario AS ID_USUARIO,
    u.nombre_usuario AS NOMBRE_USUARIO,
    u.apellidos_usuario AS APELLIDOS_USUARIO,
    u.correo AS CORREO,
    u.contrasena AS CONTRASENA,
    r.id_rol AS ID_ROL,
    r.nombre_rol AS NOMBRE_ROL,
    u.activo AS ACTIVO
FROM Usuario u
INNER JOIN UsuarioRol ur ON u.id_usuario = ur.id_usuario
INNER JOIN RolUsuario r ON ur.id_rol_usuario = r.id_rol
WHERE u.activo = 1;
GO

PRINT 'Vista VW_Usuario creada';
GO

-- =====================================================
-- 5. Crear SP_Login
-- =====================================================
IF EXISTS (SELECT 1 FROM sys.procedures WHERE name = 'SP_Login')
    DROP PROCEDURE SP_Login;
GO

CREATE PROCEDURE SP_Login
    @Correo VARCHAR(255),
    @Contrasena VARCHAR(255)
AS
BEGIN
    SET NOCOUNT ON;

    SELECT
        u.id_usuario AS ID_USUARIO,
        u.nombre_usuario AS NOMBRE_USUARIO,
        u.apellidos_usuario AS APELLIDOS_USUARIO,
        u.correo AS CORREO,
        u.contrasena AS CONTRASENA,
        r.id_rol AS ID_ROL,
        r.nombre_rol AS NOMBRE_ROL,
        u.activo AS ACTIVO
    FROM Usuario u
    INNER JOIN UsuarioRol ur ON u.id_usuario = ur.id_usuario
    INNER JOIN RolUsuario r ON ur.id_rol_usuario = r.id_rol
    WHERE u.correo = @Correo
      AND u.contrasena = @Contrasena
      AND u.activo = 1;
END;
GO

PRINT 'SP_Login creado';
GO

-- =====================================================
-- 6. Crear SP_CrearUsuario
-- =====================================================
IF EXISTS (SELECT 1 FROM sys.procedures WHERE name = 'SP_CrearUsuario')
    DROP PROCEDURE SP_CrearUsuario;
GO

CREATE PROCEDURE SP_CrearUsuario
    @NombreUsuario VARCHAR(100),
    @ApellidosUsuario VARCHAR(100),
    @Correo VARCHAR(255),
    @Contrasena VARCHAR(255),
    @IdRol INT
AS
BEGIN
    SET NOCOUNT ON;

    DECLARE @IdUsuario INT;

    BEGIN TRY
        BEGIN TRANSACTION;

        IF EXISTS (SELECT 1 FROM Usuario WHERE correo = @Correo)
        BEGIN
            SELECT -1 AS IdUsuario, 'El correo ya está registrado' AS Mensaje;
            ROLLBACK TRANSACTION;
            RETURN;
        END;

        INSERT INTO Usuario (nombre_usuario, apellidos_usuario, correo, contrasena, activo)
        VALUES (@NombreUsuario, @ApellidosUsuario, @Correo, @Contrasena, 1);

        SET @IdUsuario = SCOPE_IDENTITY();

        INSERT INTO UsuarioRol (id_usuario, id_rol_usuario)
        VALUES (@IdUsuario, @IdRol);

        COMMIT TRANSACTION;

        SELECT @IdUsuario AS IdUsuario, 'Usuario creado exitosamente' AS Mensaje;
    END TRY
    BEGIN CATCH
        ROLLBACK TRANSACTION;
        SELECT -1 AS IdUsuario, ERROR_MESSAGE() AS Mensaje;
    END CATCH;
END;
GO

PRINT 'SP_CrearUsuario creado';
GO

-- =====================================================
-- 7. Crear SP_ObtenerRoles
-- =====================================================
IF EXISTS (SELECT 1 FROM sys.procedures WHERE name = 'SP_ObtenerRoles')
    DROP PROCEDURE SP_ObtenerRoles;
GO

CREATE PROCEDURE SP_ObtenerRoles
AS
BEGIN
    SET NOCOUNT ON;

    SELECT
        id_rol AS ID_ROL,
        nombre_rol AS NOMBRE_ROL
    FROM RolUsuario
    WHERE activo = 1
    ORDER BY id_rol ASC;
END;
GO

PRINT 'SP_ObtenerRoles creado';
GO

-- =====================================================
-- 8. Usuario de prueba (descomentar si es necesario)
-- =====================================================
-- INSERT INTO Usuario (nombre_usuario, apellidos_usuario, correo, contrasena, activo)
-- VALUES ('Admin', 'Sistema', 'admin@ciudadny.com', 'admin123', 1);
--
-- INSERT INTO UsuarioRol (id_usuario, id_rol_usuario)
-- VALUES (SCOPE_IDENTITY(), 3); -- Rol Administrador
GO

PRINT '============================================';
PRINT 'Base de datos configurada correctamente!';
PRINT '============================================';
PRINT 'Roles disponibles para registro:';
SELECT id_rol, nombre_rol, activo FROM RolUsuario;
