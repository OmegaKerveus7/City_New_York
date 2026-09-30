-- =====================================================
-- Vista para información completa de usuarios
-- =====================================================
USE Ciudad_New_York;
GO

CREATE OR ALTER VIEW VW_Usuario
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

-- =====================================================
-- SP_ObtenerRoles: Obtiene roles activos PARA REGISTRO
-- El rol con ID más bajo debe tener activo = 0 para no aparecer
-- =====================================================
CREATE OR ALTER PROCEDURE SP_ObtenerRoles
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
