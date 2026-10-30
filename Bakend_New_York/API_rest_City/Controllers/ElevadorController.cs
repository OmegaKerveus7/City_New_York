using Microsoft.AspNetCore.Mvc;
using Dapper;
using API_rest_City.BD;
using API_rest_City.Services;

namespace API_rest_City.Controllers
{
    [ApiController]
    [Route("api/[controller]")]
    public class ElevadorController : ControllerBase
    {
        private const int NivelMaximo = 5;
        private const int NivelMinimo = 0;

        private readonly DBContext _dbContext;
        private readonly InstruccionService _instrucciones;

        public ElevadorController(DBContext dbContext, InstruccionService instrucciones)
        {
            _dbContext = dbContext;
            _instrucciones = instrucciones;
        }

        [HttpPost("subir")]
        public IActionResult Subir([FromBody] int? niveles = null)
        {
            try
            {
                int n = niveles ?? 1;
                if (n <= 0)
                    return BadRequest(new { error = "Los niveles deben ser mayores que cero" });

                string nombre = $"SUBIR_NIVEL {n}";
                var resultado = _instrucciones.CrearPendiente(nombre, $"Subir elevador {n} nivel(es)");

                return Ok(new
                {
                    mensaje = resultado.YaEstabaPendiente
                        ? $"{nombre} ya estaba en cola; no se duplico"
                        : $"{nombre} creado",
                    id = resultado.Id,
                    yaEstabaPendiente = resultado.YaEstabaPendiente,
                    niveles = n
                });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        [HttpPost("bajar")]
        public IActionResult Bajar([FromBody] int? niveles = null)
        {
            try
            {
                int n = niveles ?? 1;
                if (n <= 0)
                    return BadRequest(new { error = "Los niveles deben ser mayores que cero" });

                string nombre = $"BAJAR_NIVEL {n}";
                var resultado = _instrucciones.CrearPendiente(nombre, $"Bajar elevador {n} nivel(es)");

                return Ok(new
                {
                    mensaje = resultado.YaEstabaPendiente
                        ? $"{nombre} ya estaba en cola; no se duplico"
                        : $"{nombre} creado",
                    id = resultado.Id,
                    yaEstabaPendiente = resultado.YaEstabaPendiente,
                    niveles = n
                });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        [HttpPost("ir")]
        public IActionResult Ir([FromBody] NivelRequest request)
        {
            try
            {
                if (request == null)
                    return BadRequest(new { error = "Cuerpo de la peticion requerido" });

                int nivel = request.Nivel;
                if (nivel < NivelMinimo || nivel > NivelMaximo)
                    return BadRequest(new { error = $"Nivel fuera de rango ({NivelMinimo}-{NivelMaximo})" });

                string nombre = $"IR_A_NIVEL {nivel}";
                var resultado = _instrucciones.CrearPendiente(nombre, $"Ir al nivel {nivel} del elevador");

                return Ok(new
                {
                    mensaje = resultado.YaEstabaPendiente
                        ? $"{nombre} ya estaba en cola; no se duplico"
                        : $"{nombre} creado",
                    id = resultado.Id,
                    yaEstabaPendiente = resultado.YaEstabaPendiente,
                    nivel = nivel
                });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        [HttpPost("parar")]
        public IActionResult Parar()
        {
            try
            {
                var resultado = _instrucciones.CrearPendiente("ELEVADOR_PARAR", "Detener elevador");

                return Ok(new
                {
                    mensaje = resultado.YaEstabaPendiente
                        ? "ELEVADOR_PARAR ya estaba en cola; no se duplico"
                        : "ELEVADOR_PARAR creado",
                    id = resultado.Id,
                    yaEstabaPendiente = resultado.YaEstabaPendiente
                });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        [HttpPost("calibrar")]
        public IActionResult Calibrar()
        {
            try
            {
                var resultado = _instrucciones.CrearPendiente("CALIBRAR", "Calibrar elevador (bajar a piso 0)");

                return Ok(new
                {
                    mensaje = resultado.YaEstabaPendiente
                        ? "CALIBRAR ya estaba en cola; no se duplico"
                        : "CALIBRAR creado",
                    id = resultado.Id,
                    yaEstabaPendiente = resultado.YaEstabaPendiente
                });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        [HttpGet("estado")]
        public IActionResult ObtenerEstado()
        {
            try
            {
                using var connection = _dbContext.CreateConnection();

                var ultimo = connection.QueryFirstOrDefault<dynamic>(
                    @"SELECT TOP 1 nombre_instruccion AS instruccion,
                             COALESCE(nivel_resultado, angulo_resultado) AS nivel
                      FROM Log_Instruccion
                      WHERE nombre_instruccion LIKE 'SUBIR_NIVEL%'
                         OR nombre_instruccion LIKE 'BAJAR_NIVEL%'
                         OR nombre_instruccion LIKE 'IR_A_NIVEL%'
                         OR nombre_instruccion = 'ELEVADOR_STATUS'
                         OR nombre_instruccion = 'ELEVADOR_PARAR'
                         OR nombre_instruccion = 'CALIBRAR'
                      ORDER BY fecha_ejecucion DESC"
                );

                if (ultimo != null)
                {
                    return Ok(new
                    {
                        nivel = ultimo.nivel ?? 0,
                        megaOnline = true,
                        esclavoOnline = true,
                        ultimaInstruccion = (string)ultimo.instruccion
                    });
                }

                return Ok(new
                {
                    nivel = 0,
                    megaOnline = false,
                    esclavoOnline = false,
                    ultimaInstruccion = (string?)null
                });
            }
            catch (Exception ex)
            {
                return Ok(new
                {
                    nivel = 0,
                    megaOnline = false,
                    esclavoOnline = false,
                    ultimaInstruccion = (string?)null,
                    error = ex.Message
                });
            }
        }

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
                          COALESCE(nivel_resultado, angulo_resultado) AS Nivel,
                          tipo_ejecucion AS Tipo,
                          fecha_ejecucion AS Fecha
                      FROM Log_Instruccion
                      WHERE nombre_instruccion LIKE 'SUBIR_NIVEL%'
                         OR nombre_instruccion LIKE 'BAJAR_NIVEL%'
                         OR nombre_instruccion LIKE 'IR_A_NIVEL%'
                         OR nombre_instruccion = 'ELEVADOR_STATUS'
                         OR nombre_instruccion = 'ELEVADOR_PARAR'
                         OR nombre_instruccion = 'CALIBRAR'
                      ORDER BY fecha_ejecucion DESC",
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

    public class NivelRequest
    {
        public int Nivel { get; set; }
    }
}