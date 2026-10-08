-- =====================================================
-- Stored Procedures para el sistema de instrucciones
-- Ciudad New York - Puente Elevadizo
-- =====================================================

USE Ciudad_New_York;
GO

-- =====================================================
-- SP: Obtener todas las instrucciones
-- =====================================================
IF EXISTS (SELECT 1 FROM sys.procedures WHERE name = 'SP_ObtenerInstrucciones')
    DROP PROCEDURE SP_ObtenerInstrucciones;
GO

CREATE PROCEDURE SP_ObtenerInstrucciones
AS
BEGIN
    SET NOCOUNT ON;

    SELECT
        i.id_instruccion AS ID_INSTRUCCION,
        i.codigo AS CODIGO,
        i.componente AS COMPONENTE,
        i.numero_instruccion AS NUMERO_INSTRUCCION,
        i.descripcion AS DESCRIPCION,
        i.parametros AS PARAMETROS,
        i.activa AS ACTIVA,
        i.fecha_creacion AS FECHA_CREACION
    FROM Instruccion i
    WHERE i.activa = 1
    ORDER BY i.componente, i.numero_instruccion;
END;
GO

PRINT 'SP_ObtenerInstrucciones creado';
GO

-- =====================================================
-- SP: Obtener instrucciones pendientes para un dispositivo
-- =====================================================
IF EXISTS (SELECT 1 FROM sys.procedures WHERE name = 'SP_ObtenerInstruccionesPendientes')
    DROP PROCEDURE SP_ObtenerInstruccionesPendientes;
GO

CREATE PROCEDURE SP_ObtenerInstruccionesPendientes
    @tipo_dispositivo VARCHAR(10)
AS
BEGIN
    SET NOCOUNT ON;

    -- Primero marcar como procesadas las instrucciones antiguas (mas de 5 minutos)
    UPDATE Instruccion_Pendiente
    SET procesada = 1, fecha_procesamiento = GETDATE()
    WHERE procesada = 0
      AND DATEADD(MINUTE, 5, fecha_creacion) < GETDATE();

    -- Obtener instrucciones pendientes para este dispositivo
    SELECT
        ip.id_pendiente AS ID_PENDIENTE,
        i.id_instruccion AS ID_INSTRUCCION,
        i.codigo AS CODIGO,
        i.componente AS COMPONENTE,
        i.numero_instruccion AS NUMERO_INSTRUCCION,
        i.descripcion AS DESCRIPCION,
        i.parametros AS PARAMETROS,
        ip.parametros_ejecucion AS PARAMETROS_EJECUCION,
        ip.fecha_creacion AS FECHA_CREACION
    FROM Instruccion_Pendiente ip
    INNER JOIN Instruccion i ON ip.id_instruccion = i.id_instruccion
    INNER JOIN Dispositivo d ON ip.id_dispositivo_destino = d.id_dispositivo
    WHERE d.tipo = @tipo_dispositivo
      AND ip.procesada = 0
      AND i.activa = 1
    ORDER BY ip.fecha_creacion ASC;
END;
GO

PRINT 'SP_ObtenerInstruccionesPendientes creado';
GO

-- =====================================================
-- SP: Crear instruccion pendiente (para ejecucion remota)
-- =====================================================
IF EXISTS (SELECT 1 FROM sys.procedures WHERE name = 'SP_CrearInstruccionPendiente')
    DROP PROCEDURE SP_CrearInstruccionPendiente;
GO

CREATE PROCEDURE SP_CrearInstruccionPendiente
    @codigo_instruccion VARCHAR(20),
    @parametros VARCHAR(MAX) = NULL,
    @id_usuario INT = NULL
AS
BEGIN
    SET NOCOUNT ON;

    DECLARE @id_instruccion INT;
    DECLARE @id_dispositivo INT;

    -- Obtener el ID de la instruccion
    SELECT @id_instruccion = id_instruccion
    FROM Instruccion
    WHERE codigo = @codigo_instruccion AND activa = 1;

    IF @id_instruccion IS NULL
    BEGIN
        SELECT -1 AS ID_RESULTADO, 'Instruccion no encontrada o inactiva' AS MENSAJE;
        RETURN;
    END

    -- Obtener el ID del dispositivo destino segun el componente
    DECLARE @componente VARCHAR(10);
    SELECT @componente = componente FROM Instruccion WHERE id_instruccion = @id_instruccion;

    SELECT @id_dispositivo = id_dispositivo
    FROM Dispositivo
    WHERE tipo = @componente AND activo = 1;

    IF @id_dispositivo IS NULL
    BEGIN
        SELECT -1 AS ID_RESULTADO, 'Dispositivo no encontrado o inactivo' AS MENSAJE;
        RETURN;
    END

    -- Crear la instruccion pendiente
    INSERT INTO Instruccion_Pendiente (id_instruccion, id_dispositivo_destino, parametros_ejecucion, creada_por)
    VALUES (@id_instruccion, @id_dispositivo, @parametros, @id_usuario);

    DECLARE @id_pendiente INT = SCOPE_IDENTITY();

    -- Registrar en log
    INSERT INTO Log_Instruccion (id_instruccion, id_usuario, id_dispositivo, tipo_ejecucion, resultado)
    VALUES (@id_instruccion, @id_usuario, @id_dispositivo, 'REMOTA', 'PENDIENTE');

    -- Retornar id_instruccion para que el ESP32 pueda reportar la respuesta
    SELECT @id_instruccion AS ID_RESULTADO, 'Instruccion creada exitosamente' AS MENSAJE;
END;
GO

PRINT 'SP_CrearInstruccionPendiente creado';
GO

-- =====================================================
-- SP: Registrar respuesta de ejecucion (desde ESP32)
-- =====================================================
IF EXISTS (SELECT 1 FROM sys.procedures WHERE name = 'SP_RegistrarRespuestaInstruccion')
    DROP PROCEDURE SP_RegistrarRespuestaInstruccion;
GO

CREATE PROCEDURE SP_RegistrarRespuestaInstruccion
    @id_instruccion INT,
    @resultado VARCHAR(50),
    @respuesta_dispositivo VARCHAR(500) = NULL,
    @ip_dispositivo VARCHAR(50) = NULL,
    @angulo_resultado INT = NULL,
    @tiempo_respuesta_ms INT = NULL
AS
BEGIN
    SET NOCOUNT ON;

    DECLARE @id_dispositivo INT;

    -- Obtener el dispositivo segun la instruccion
    SELECT @id_dispositivo = d.id_dispositivo
    FROM Instruccion_Pendiente ip
    INNER JOIN Dispositivo d ON ip.id_dispositivo_destino = d.id_dispositivo
    WHERE ip.id_instruccion = @id_instruccion
      AND ip.procesada = 0;

    -- Marcar como procesada la instruccion pendiente mas antigua para esta instruccion
    UPDATE TOP(1) Instruccion_Pendiente
    SET procesada = 1, fecha_procesamiento = GETDATE()
    WHERE id_instruccion = @id_instruccion AND procesada = 0;

    -- Actualizar el log mas reciente para esta instruccion que aun no tiene respuesta
    UPDATE TOP(1) Log_Instruccion
    SET resultado = @resultado,
        respuesta_dispositivo = @respuesta_dispositivo,
        ip_dispositivo = @ip_dispositivo,
        angulo_resultado = @angulo_resultado,
        tiempo_respuesta_ms = @tiempo_respuesta_ms
    WHERE id_instruccion = @id_instruccion
      AND resultado = 'PENDIENTE';

    -- Si no encontro ninguno pendiente, insertar nuevo
    IF @@ROWCOUNT = 0
    BEGIN
        -- Obtener el usuario que creo la instruccion
        DECLARE @id_usuario INT;
        SELECT TOP 1 @id_usuario = creada_por
        FROM Instruccion_Pendiente
        WHERE id_instruccion = @id_instruccion;

        INSERT INTO Log_Instruccion (id_instruccion, id_usuario, id_dispositivo, tipo_ejecucion, resultado, respuesta_dispositivo, ip_dispositivo, angulo_resultado, tiempo_respuesta_ms)
        VALUES (@id_instruccion, @id_usuario, @id_dispositivo, 'REMOTA', @resultado, @respuesta_dispositivo, @ip_dispositivo, @angulo_resultado, @tiempo_respuesta_ms);
    END

    -- Actualizar ultimo ping del dispositivo
    IF @id_dispositivo IS NOT NULL
        UPDATE Dispositivo SET ultimo_ping = GETDATE() WHERE id_dispositivo = @id_dispositivo;

    SELECT 1 AS ID_RESULTADO, 'Respuesta registrada' AS MENSAJE;
END;
GO

PRINT 'SP_RegistrarRespuestaInstruccion creado';
GO

-- =====================================================
-- SP: Registrar instruccion local (desde boton fisico)
-- =====================================================
IF EXISTS (SELECT 1 FROM sys.procedures WHERE name = 'SP_RegistrarInstruccionLocal')
    DROP PROCEDURE SP_RegistrarInstruccionLocal;
GO

CREATE PROCEDURE SP_RegistrarInstruccionLocal
    @codigo_instruccion VARCHAR(20),
    @resultado VARCHAR(50),
    @respuesta_dispositivo VARCHAR(500) = NULL,
    @ip_dispositivo VARCHAR(50) = NULL,
    @angulo_resultado INT = NULL
AS
BEGIN
    SET NOCOUNT ON;

    DECLARE @id_instruccion INT;
    DECLARE @id_dispositivo INT;

    -- Obtener el ID de la instruccion
    SELECT @id_instruccion = id_instruccion
    FROM Instruccion
    WHERE codigo = @codigo_instruccion AND activa = 1;

    IF @id_instruccion IS NULL
    BEGIN
        SELECT -1 AS ID_RESULTADO, 'Instruccion no encontrada' AS MENSAJE;
        RETURN;
    END

    -- Obtener el ID del dispositivo
    DECLARE @componente VARCHAR(10);
    SELECT @componente = componente FROM Instruccion WHERE id_instruccion = @id_instruccion;

    SELECT @id_dispositivo = id_dispositivo
    FROM Dispositivo
    WHERE tipo = @componente AND activo = 1;

    -- Registrar directamente en log como ejecucion local
    INSERT INTO Log_Instruccion (id_instruccion, id_dispositivo, tipo_ejecucion, resultado, respuesta_dispositivo, ip_dispositivo, angulo_resultado)
    VALUES (@id_instruccion, @id_dispositivo, 'LOCAL', @resultado, @respuesta_dispositivo, @ip_dispositivo, @angulo_resultado);

    -- Actualizar ultimo ping del dispositivo
    UPDATE Dispositivo SET ultimo_ping = GETDATE() WHERE id_dispositivo = @id_dispositivo;

    DECLARE @id_log INT = SCOPE_IDENTITY();
    SELECT @id_log AS ID_RESULTADO, 'Instruccion local registrada' AS MENSAJE;
END;
GO

PRINT 'SP_RegistrarInstruccionLocal creado';
GO

-- =====================================================
-- SP: Obtener log de instrucciones
-- =====================================================
IF EXISTS (SELECT 1 FROM sys.procedures WHERE name = 'SP_ObtenerLogInstrucciones')
    DROP PROCEDURE SP_ObtenerLogInstrucciones;
GO

CREATE PROCEDURE SP_ObtenerLogInstrucciones
    @limite INT = 50
AS
BEGIN
    SET NOCOUNT ON;

    SELECT TOP (@limite)
        l.id_log AS ID_LOG,
        i.codigo AS CODIGO,
        i.componente AS COMPONENTE,
        i.descripcion AS DESCRIPCION,
        l.tipo_ejecucion AS TIPO_EJECUCION,
        l.resultado AS RESULTADO,
        l.respuesta_dispositivo AS RESPUESTA,
        l.angulo_resultado AS ANGULO_RESULTADO,
        l.fecha_ejecucion AS FECHA_EJECUCION,
        l.tiempo_respuesta_ms AS TIEMPO_RESPUESTA,
        u.nombre_usuario AS USUARIO,
        d.nombre AS DISPOSITIVO
    FROM Log_Instruccion l
    INNER JOIN Instruccion i ON l.id_instruccion = i.id_instruccion
    LEFT JOIN Usuario u ON l.id_usuario = u.id_usuario
    LEFT JOIN Dispositivo d ON l.id_dispositivo = d.id_dispositivo
    ORDER BY l.fecha_ejecucion DESC;
END;
GO

PRINT 'SP_ObtenerLogInstrucciones creado';
GO

-- =====================================================
-- SP: Verificar conexion (heartbeat)
-- =====================================================
IF EXISTS (SELECT 1 FROM sys.procedures WHERE name = 'SP_VerificarConexion')
    DROP PROCEDURE SP_VerificarConexion;
GO

CREATE PROCEDURE SP_VerificarConexion
    @tipo_dispositivo VARCHAR(10)
AS
BEGIN
    SET NOCOUNT ON;

    DECLARE @id_dispositivo INT;
    DECLARE @nombre VARCHAR(50);

    SELECT @id_dispositivo = id_dispositivo, @nombre = nombre
    FROM Dispositivo
    WHERE tipo = @tipo_dispositivo AND activo = 1;

    IF @id_dispositivo IS NULL
    BEGIN
        SELECT -1 AS ID_DISPOSITIVO, 'Dispositivo no registrado' AS ESTADO;
        RETURN;
    END

    -- Actualizar ultimo ping
    UPDATE Dispositivo SET ultimo_ping = GETDATE() WHERE id_dispositivo = @id_dispositivo;

    SELECT @id_dispositivo AS ID_DISPOSITIVO, @nombre AS NOMBRE, 'OK' AS ESTADO;
END;
GO

PRINT 'SP_VerificarConexion creado';
GO

-- =====================================================
-- SP: Obtener estado del sistema (para frontend)
-- =====================================================
IF EXISTS (SELECT 1 FROM sys.procedures WHERE name = 'SP_ObtenerEstadoSistema')
    DROP PROCEDURE SP_ObtenerEstadoSistema;
GO

CREATE PROCEDURE SP_ObtenerEstadoSistema
AS
BEGIN
    SET NOCOUNT ON;

    -- Estado de dispositivos
    SELECT
        d.id_dispositivo AS ID_DISPOSITIVO,
        d.nombre AS NOMBRE,
        d.tipo AS TIPO,
        d.ip AS IP,
        d.ultimo_ping AS ULTIMO_PING,
        CASE
            WHEN d.ultimo_ping IS NULL THEN 'NUNCA'
            WHEN DATEDIFF(SECOND, d.ultimo_ping, GETDATE()) < 10 THEN 'ONLINE'
            WHEN DATEDIFF(SECOND, d.ultimo_ping, GETDATE()) < 60 THEN 'RECIEN'
            ELSE 'OFFLINE'
        END AS ESTADO
    FROM Dispositivo d
    WHERE d.activo = 1;

    -- Ultima instruccion ejecutada
    SELECT TOP 1
        i.codigo AS CODIGO,
        i.descripcion AS DESCRIPCION,
        l.resultado AS RESULTADO,
        l.tipo_ejecucion AS TIPO_EJECUCION,
        l.fecha_ejecucion AS FECHA,
        l.angulo_resultado AS ANGULO
    FROM Log_Instruccion l
    INNER JOIN Instruccion i ON l.id_instruccion = i.id_instruccion
    WHERE l.tipo_ejecucion = 'LOCAL'
    ORDER BY l.fecha_ejecucion DESC;
END;
GO

PRINT 'SP_ObtenerEstadoSistema creado';
GO

PRINT '============================================';
PRINT 'Stored Procedures de instrucciones creados!';
PRINT '============================================';
