using Microsoft.AspNetCore.Mvc;
using Dapper;
using API_rest_City.BD;

namespace API_rest_City.Controllers
{
    [ApiController]
    [Route("api/[controller]")]
    public class PuenteController : ControllerBase
    {
        private readonly DBContext _dbContext;
        private const string esp32BaseUrl = "https://warless-predestinately-bethann.ngrok-free.dev";

        public PuenteController(DBContext dbContext)
        {
            _dbContext = dbContext;
        }

        [HttpGet("estado")]
        public IActionResult ObtenerEstado()
        {
            try
            {
                using var connection = _dbContext.CreateConnection();
                var ultimoLog = connection.QueryFirstOrDefault<dynamic>(
                    @"SELECT TOP 1 
                        CASE WHEN resultado = 'OK' THEN 'ABIERTO' ELSE 'CERRADO' END AS estado,
                        angulo_resultado AS angulo
                      FROM Log_Instruccion 
                      ORDER BY fecha_ejecucion DESC"
                );

                if (ultimoLog != null)
                {
                    return Ok(new
                    {
                        estado = ultimoLog.estado,
                        angulo = ultimoLog.angulo ?? 0,
                        megaOnline = true,
                        esclavoOnline = true
                    });
                }

                return Ok(new
                {
                    estado = "CERRADO",
                    angulo = 180,
                    megaOnline = false,
                    esclavoOnline = false
                });
            }
            catch (Exception ex)
            {
                return Ok(new
                {
                    estado = "CERRADO",
                    angulo = 180,
                    megaOnline = false,
                    esclavoOnline = false
                });
            }
        }

        [HttpPost("abrir")]
        public IActionResult AbrirPuente()
        {
            try
            {
                using var connection = _dbContext.CreateConnection();

                var sql = @"
                    INSERT INTO Instruccion_Pendiente (id_instruccion)
                    SELECT id_instruccion FROM Instruccion WHERE nombre = 'SERVO_SUBIR' AND activa = 1;
                    SELECT SCOPE_IDENTITY() AS Id;";

                var result = connection.QueryFirstOrDefault<dynamic>(sql);

                if (result == null || result.Id == null)
                {
                    return BadRequest(new { error = "Instruccion no encontrada" });
                }

                return Ok(new
                {
                    mensaje = "Instruccion SERVO_SUBIR creada",
                    id = result.Id
                });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        [HttpPost("cerrar")]
        public IActionResult CerrarPuente()
        {
            try
            {
                using var connection = _dbContext.CreateConnection();

                var sql = @"
                    INSERT INTO Instruccion_Pendiente (id_instruccion)
                    SELECT id_instruccion FROM Instruccion WHERE nombre = 'SERVO_BAJAR' AND activa = 1;
                    SELECT SCOPE_IDENTITY() AS Id;";

                var result = connection.QueryFirstOrDefault<dynamic>(sql);

                if (result == null || result.Id == null)
                {
                    return BadRequest(new { error = "Instruccion no encontrada" });
                }

                return Ok(new
                {
                    mensaje = "Instruccion SERVO_BAJAR creada",
                    id = result.Id
                });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

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
}
