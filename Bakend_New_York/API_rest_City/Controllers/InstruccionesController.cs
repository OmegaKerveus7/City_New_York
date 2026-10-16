using Microsoft.AspNetCore.Mvc;
using Dapper;
using API_rest_City.BD;
using API_rest_City.Services;
using System.Data;

namespace API_rest_City.Controllers
{
    [ApiController]
    [Route("api/[controller]")]
    public class InstruccionesController : ControllerBase
    {
        private readonly DBContext _dbContext;
        private readonly InstruccionService _instrucciones;

        public InstruccionesController(DBContext dbContext, InstruccionService instrucciones)
        {
            _dbContext = dbContext;
            _instrucciones = instrucciones;
        }

        // GET /api/instrucciones - Lista todas las instrucciones
        [HttpGet]
        public IActionResult ObtenerInstrucciones()
        {
            try
            {
                using var connection = _dbContext.CreateConnection();
                var instrucciones = connection.Query<dynamic>(
                    "SELECT id_instruccion AS Id, nombre AS Nombre, descripcion AS Descripcion, activa AS Activa FROM Instruccion WHERE activa = 1 ORDER BY id_instruccion"
                );
                return Ok(instrucciones);
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        // GET /api/instrucciones/pendientes - Para que ESP32 consulte
        [HttpGet("pendientes")]
        public IActionResult ObtenerPendientes()
        {
            try
            {
                using var connection = _dbContext.CreateConnection();
                connection.Open();
                using var transaction = connection.BeginTransaction(IsolationLevel.ReadCommitted);

                var sql = @"
                    SELECT TOP 1
                        ip.id_pendiente AS Id,
                        i.nombre AS Nombre,
                        i.descripcion AS Descripcion
                    FROM Instruccion_Pendiente ip WITH (UPDLOCK, ROWLOCK)
                    INNER JOIN Instruccion i ON ip.id_instruccion = i.id_instruccion
                    WHERE ip.procesada = 0
                      AND i.activa = 1
                      AND (
                          ip.fecha_procesamiento IS NULL
                          OR ip.fecha_procesamiento < DATEADD(SECOND, -30, GETDATE())
                      )
                    ORDER BY ip.fecha_creacion ASC";

                var pendiente = connection.QueryFirstOrDefault<dynamic>(sql, transaction: transaction);
                if (pendiente == null)
                {
                    transaction.Commit();
                    return Ok(Array.Empty<object>());
                }

                int idPendiente = Convert.ToInt32(pendiente.Id);
                connection.Execute(
                    "UPDATE Instruccion_Pendiente SET fecha_procesamiento = GETDATE() WHERE id_pendiente = @id AND procesada = 0",
                    new { id = idPendiente },
                    transaction);

                transaction.Commit();
                return Ok(new[] { (object)pendiente });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        // POST /api/instrucciones - Crear instruccion pendiente (desde frontend)
        // Solo se envia el nombre: si no existe en el catalogo se crea automaticamente.
        [HttpPost]
        public IActionResult CrearInstruccion([FromBody] CrearInstruccionRequest request)
        {
            try
            {
                if (string.IsNullOrWhiteSpace(request.Nombre))
                    return BadRequest(new { error = "El nombre de la instruccion es obligatorio" });

                var resultado = _instrucciones.CrearPendiente(request.Nombre);

                return Ok(new
                {
                    id = resultado.Id,
                    mensaje = resultado.YaEstabaPendiente
                        ? "La instruccion ya estaba pendiente; no se duplico"
                        : "Instruccion creada",
                    nombre = request.Nombre,
                    yaEstabaPendiente = resultado.YaEstabaPendiente
                });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        // POST /api/instrucciones/respuesta - Respuesta del ESP32
        [HttpPost("respuesta")]
        public IActionResult RegistrarRespuesta([FromBody] RespuestaRequest request)
        {
            try
            {
                using var connection = _dbContext.CreateConnection();

                // Marcar como procesada
                var updateSql = @"
                    UPDATE Instruccion_Pendiente
                    SET procesada = 1, fecha_procesamiento = GETDATE()
                    WHERE id_pendiente = @id_pendiente";

                connection.Execute(updateSql, new { id_pendiente = request.IdPendiente });

                // Obtener nombre de instruccion para el log
                var nombreSql = @"
                    SELECT i.nombre
                    FROM Instruccion_Pendiente ip
                    INNER JOIN Instruccion i ON ip.id_instruccion = i.id_instruccion
                    WHERE ip.id_pendiente = @id";
                var nombre = connection.QueryFirstOrDefault<string>(nombreSql, new { id = request.IdPendiente });

                // Registrar en log
                var logSql = @"
                    INSERT INTO Log_Instruccion (nombre_instruccion, resultado, angulo_resultado, ip_dispositivo, tipo_ejecucion)
                    VALUES (@nombre, @resultado, @angulo, @ip, 'REMOTA')";

                connection.Execute(logSql, new {
                    nombre = nombre ?? "DESCONOCIDO",
                    resultado = request.Resultado,
                    angulo = request.Angulo,
                    ip = request.IpDispositivo
                });

                return Ok(new { mensaje = "Respuesta registrada" });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        // POST /api/instrucciones/local - Ejecucion local (desde boton)
        [HttpPost("local")]
        public IActionResult RegistrarLocal([FromBody] LocalRequest request)
        {
            try
            {
                using var connection = _dbContext.CreateConnection();

                var logSql = @"
                    INSERT INTO Log_Instruccion (nombre_instruccion, resultado, angulo_resultado, ip_dispositivo, tipo_ejecucion)
                    VALUES (@nombre, @resultado, @angulo, @ip, 'LOCAL')";

                connection.Execute(logSql, new {
                    nombre = request.Nombre,
                    resultado = request.Resultado,
                    angulo = request.Angulo,
                    ip = request.IpDispositivo
                });

                return Ok(new { mensaje = "Ejecucion local registrada" });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        // GET /api/instrucciones/log - Historial
        [HttpGet("log")]
        public IActionResult ObtenerLog([FromQuery] int limite = 20)
        {
            try
            {
                using var connection = _dbContext.CreateConnection();
                var log = connection.Query<dynamic>(
                    "SELECT TOP (@limite) id_log AS Id, nombre_instruccion AS Instruccion, resultado AS Resultado, angulo_resultado AS Angulo, tipo_ejecucion AS Tipo, fecha_ejecucion AS Fecha FROM Log_Instruccion ORDER BY fecha_ejecucion DESC",
                    new { limite }
                );
                return Ok(log);
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }
    }

    public class CrearInstruccionRequest
    {
        public string Nombre { get; set; } = string.Empty;
    }

    public class RespuestaRequest
    {
        public int IdPendiente { get; set; }
        public string Resultado { get; set; } = string.Empty;
        public int? Angulo { get; set; }
        public string? IpDispositivo { get; set; }
    }

    public class LocalRequest
    {
        public string Nombre { get; set; } = string.Empty;
        public string Resultado { get; set; } = string.Empty;
        public int? Angulo { get; set; }
        public string? IpDispositivo { get; set; }
    }
}
