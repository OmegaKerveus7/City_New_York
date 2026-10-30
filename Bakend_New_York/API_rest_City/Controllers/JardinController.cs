using Microsoft.AspNetCore.Mvc;
using Dapper;
using API_rest_City.BD;
using API_rest_City.Services;

namespace API_rest_City.Controllers
{
    /// <summary>
    /// Control de la fuente y las luces del Central Park (esclavo I2C 0x0A).
    /// </summary>
    [ApiController]
    [Route("api/jardin")]
    public class JardinController : ControllerBase
    {
        private readonly DBContext _dbContext;
        private readonly InstruccionService _instrucciones;
        private readonly EstadoService _estado;

        public JardinController(DBContext dbContext, InstruccionService instrucciones, EstadoService estado)
        {
            _dbContext = dbContext;
            _instrucciones = instrucciones;
            _estado = estado;
        }

        // POST /api/jardin/fuente/encender
        [HttpPost("fuente/encender")]
        public IActionResult EncenderFuente()
        {
            try
            {
                var resultado = _instrucciones.CrearPendiente("FUENTE_ENCENDER", "Encender la fuente del Central Park");

                return Ok(new
                {
                    mensaje = resultado.YaEstabaPendiente
                        ? "FUENTE_ENCENDER ya estaba en cola; no se duplico"
                        : "FUENTE_ENCENDER creado",
                    id = resultado.Id,
                    yaEstabaPendiente = resultado.YaEstabaPendiente
                });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        // POST /api/jardin/fuente/apagar
        [HttpPost("fuente/apagar")]
        public IActionResult ApagarFuente()
        {
            try
            {
                var resultado = _instrucciones.CrearPendiente("FUENTE_APAGAR", "Apagar la fuente del Central Park");

                return Ok(new
                {
                    mensaje = resultado.YaEstabaPendiente
                        ? "FUENTE_APAGAR ya estaba en cola; no se duplico"
                        : "FUENTE_APAGAR creado",
                    id = resultado.Id,
                    yaEstabaPendiente = resultado.YaEstabaPendiente
                });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        // POST /api/jardin/fuente/pulso  body: 15  (segundos)
        [HttpPost("fuente/pulso")]
        public IActionResult PulsoFuente([FromBody] int? segundos = null)
        {
            try
            {
                int seg = segundos ?? 10;
                if (seg <= 0 || seg > 600)
                    return BadRequest(new { error = "Los segundos deben estar entre 1 y 600" });

                string nombre = $"FUENTE_PULSO {seg}";
                var resultado = _instrucciones.CrearPendiente(nombre, $"Fuente encendida {seg} segundos");

                return Ok(new
                {
                    mensaje = resultado.YaEstabaPendiente
                        ? $"{nombre} ya estaba en cola; no se duplico"
                        : $"{nombre} creado",
                    id = resultado.Id,
                    yaEstabaPendiente = resultado.YaEstabaPendiente,
                    segundos = seg
                });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        // POST /api/jardin/fuente/status
        [HttpPost("fuente/status")]
        public IActionResult StatusFuente()
        {
            try
            {
                var resultado = _instrucciones.CrearPendiente("FUENTE_STATUS", "Reportar estado de la fuente");

                return Ok(new
                {
                    mensaje = resultado.YaEstabaPendiente
                        ? "FUENTE_STATUS ya estaba en cola; no se duplico"
                        : "FUENTE_STATUS creado",
                    id = resultado.Id,
                    yaEstabaPendiente = resultado.YaEstabaPendiente
                });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        // POST /api/jardin/luces/encender
        [HttpPost("luces/encender")]
        public IActionResult EncenderLuces()
        {
            try
            {
                var resultado = _instrucciones.CrearPendiente("JARDIN_LUCES_ON", "Encender las luces del jardin");

                return Ok(new
                {
                    mensaje = resultado.YaEstabaPendiente
                        ? "JARDIN_LUCES_ON ya estaba en cola; no se duplico"
                        : "JARDIN_LUCES_ON creado",
                    id = resultado.Id,
                    yaEstabaPendiente = resultado.YaEstabaPendiente
                });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        // POST /api/jardin/luces/apagar
        [HttpPost("luces/apagar")]
        public IActionResult ApagarLuces()
        {
            try
            {
                var resultado = _instrucciones.CrearPendiente("JARDIN_LUCES_OFF", "Apagar las luces del jardin");

                return Ok(new
                {
                    mensaje = resultado.YaEstabaPendiente
                        ? "JARDIN_LUCES_OFF ya estaba en cola; no se duplico"
                        : "JARDIN_LUCES_OFF creado",
                    id = resultado.Id,
                    yaEstabaPendiente = resultado.YaEstabaPendiente
                });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        /// <summary>
        /// GET /api/jardin/estado - Estado en vivo de la fuente (telemetría).
        /// </summary>
        [HttpGet("estado")]
        public IActionResult ObtenerEstado()
        {
            try
            {
                var componentes = _estado.ObtenerPorModulo("FUENTE");

                return Ok(new
                {
                    componentes,
                    bomba = Valor(componentes, "FUENTE.BOMBA"),
                    agua = Valor(componentes, "FUENTE.AGUA"),
                    luces = Valor(componentes, "FUENTE.LUCES"),
                    total = componentes.Count
                });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        // GET /api/jardin/log
        [HttpGet("log")]
        public IActionResult ObtenerLog([FromQuery] int limite = 20)
        {
            try
            {
                using var connection = _dbContext.CreateConnection();
                var log = connection.Query<dynamic>(
                    @"SELECT TOP (@limite)
                          id_log AS Id,
                          nombre_instruccion AS Instruccion,
                          resultado AS Resultado,
                          angulo_resultado AS Valor,
                          nivel_resultado AS Nivel,
                          tipo_ejecucion AS Tipo,
                          fecha_ejecucion AS Fecha
                      FROM Log_Instruccion
                      WHERE nombre_instruccion LIKE 'FUENTE%'
                         OR nombre_instruccion LIKE 'JARDIN%'
                      ORDER BY fecha_ejecucion DESC",
                    new { limite });
                return Ok(log);
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        private static string? Valor(List<API_rest_City.Models.EstadoComponente> componentes, string clave)
        {
            return componentes.FirstOrDefault(c => c.Clave == clave)?.Valor;
        }
    }
}
