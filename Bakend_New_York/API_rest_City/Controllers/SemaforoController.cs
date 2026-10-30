using Microsoft.AspNetCore.Mvc;
using Dapper;
using API_rest_City.BD;
using API_rest_City.Services;

namespace API_rest_City.Controllers
{
    /// <summary>
    /// Control de los semáforos de la ciudad.
    /// Los semáforos los maneja directamente el Arduino Mega maestro
    /// (pines 22-24 para el carril N-S y 26-28 para el carril E-O).
    /// </summary>
    [ApiController]
    [Route("api/semaforo")]
    public class SemaforoController : ControllerBase
    {
        private static readonly string[] CarrilesValidos = { "NS", "EO" };
        private static readonly string[] ColoresValidos = { "ROJO", "AMARILLO", "VERDE" };

        private readonly DBContext _dbContext;
        private readonly InstruccionService _instrucciones;
        private readonly EstadoService _estado;

        public SemaforoController(DBContext dbContext, InstruccionService instrucciones, EstadoService estado)
        {
            _dbContext = dbContext;
            _instrucciones = instrucciones;
            _estado = estado;
        }

        /// <summary>
        /// POST /api/semaforo/cambiar  body: { "carril": "NS", "color": "VERDE" }
        /// </summary>
        [HttpPost("cambiar")]
        public IActionResult Cambiar([FromBody] SemaforoRequest request)
        {
            try
            {
                if (request == null)
                    return BadRequest(new { error = "Cuerpo de la peticion requerido" });

                string carril = (request.Carril ?? string.Empty).Trim().ToUpperInvariant();
                string color = (request.Color ?? string.Empty).Trim().ToUpperInvariant();

                if (!CarrilesValidos.Contains(carril))
                    return BadRequest(new { error = "Carril debe ser NS o EO" });

                if (!ColoresValidos.Contains(color))
                    return BadRequest(new { error = "Color debe ser ROJO, AMARILLO o VERDE" });

                string nombre = $"SEMAFORO_{carril}_{color}";
                var resultado = _instrucciones.CrearPendiente(nombre, $"Semaforo {carril} en {color}");

                return Ok(new
                {
                    mensaje = resultado.YaEstabaPendiente
                        ? $"{nombre} ya estaba en cola; no se duplico"
                        : $"{nombre} creado",
                    id = resultado.Id,
                    yaEstabaPendiente = resultado.YaEstabaPendiente,
                    carril,
                    color
                });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        // POST /api/semaforo/auto
        [HttpPost("auto")]
        public IActionResult ModoAutomatico()
        {
            try
            {
                var resultado = _instrucciones.CrearPendiente("SEMAFORO_AUTO", "Semaforos en modo automatico");

                return Ok(new
                {
                    mensaje = resultado.YaEstabaPendiente
                        ? "SEMAFORO_AUTO ya estaba en cola; no se duplico"
                        : "SEMAFORO_AUTO creado",
                    id = resultado.Id,
                    yaEstabaPendiente = resultado.YaEstabaPendiente
                });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        // POST /api/semaforo/parar
        [HttpPost("parar")]
        public IActionResult Parar()
        {
            try
            {
                var resultado = _instrucciones.CrearPendiente("SEMAFORO_PARAR", "Detener el ciclo automatico de los semaforos");

                return Ok(new
                {
                    mensaje = resultado.YaEstabaPendiente
                        ? "SEMAFORO_PARAR ya estaba en cola; no se duplico"
                        : "SEMAFORO_PARAR creado",
                    id = resultado.Id,
                    yaEstabaPendiente = resultado.YaEstabaPendiente
                });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        // POST /api/semaforo/status
        [HttpPost("status")]
        public IActionResult Status()
        {
            try
            {
                var resultado = _instrucciones.CrearPendiente("SEMAFORO_STATUS", "Reportar estado de los semaforos");

                return Ok(new
                {
                    mensaje = resultado.YaEstabaPendiente
                        ? "SEMAFORO_STATUS ya estaba en cola; no se duplico"
                        : "SEMAFORO_STATUS creado",
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
        /// GET /api/semaforo/estado - Estado en vivo de los semáforos (telemetría).
        /// </summary>
        [HttpGet("estado")]
        public IActionResult ObtenerEstado()
        {
            try
            {
                var componentes = _estado.ObtenerPorModulo("SEMAFORO");

                return Ok(new
                {
                    componentes,
                    ns = Valor(componentes, "SEMAFORO.NS"),
                    eo = Valor(componentes, "SEMAFORO.EO"),
                    modo = Valor(componentes, "SEMAFORO.MODO"),
                    total = componentes.Count
                });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        // GET /api/semaforo/log
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
                          tipo_ejecucion AS Tipo,
                          fecha_ejecucion AS Fecha
                      FROM Log_Instruccion
                      WHERE nombre_instruccion LIKE 'SEMAFORO%'
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

    public class SemaforoRequest
    {
        public string? Carril { get; set; }
        public string? Color { get; set; }
    }
}
