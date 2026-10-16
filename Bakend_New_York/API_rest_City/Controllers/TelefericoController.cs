using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.ModelBinding;
using Dapper;
using API_rest_City.BD;
using API_rest_City.Services;

namespace API_rest_City.Controllers
{
    [ApiController]
    [Route("api/[controller]")]
    public class TelefericoController : ControllerBase
    {
        private const int PasosMovimientoPredeterminados = 3000;
        private readonly DBContext _dbContext;
        private readonly InstruccionService _instrucciones;

        public TelefericoController(DBContext dbContext, InstruccionService instrucciones)
        {
            _dbContext = dbContext;
            _instrucciones = instrucciones;
        }

        [HttpPost("avanzar")]
        public IActionResult Avanzar([FromBody(EmptyBodyBehavior = EmptyBodyBehavior.Allow)] int? pasos = null)
        {
            try
            {
                int pasosMovimiento = pasos ?? PasosMovimientoPredeterminados;
                if (pasosMovimiento <= 0)
                    return BadRequest(new { error = "Los pasos deben ser mayores que cero" });

                string nombreInstruccion = $"TELEFERICO_AVANZAR {pasosMovimiento}";

                var resultado = _instrucciones.CrearPendiente(nombreInstruccion, $"Avanzar teleferico {pasosMovimiento} pasos");

                return Ok(new
                {
                    mensaje = resultado.YaEstabaPendiente
                        ? $"{nombreInstruccion} ya estaba en cola; no se duplico"
                        : $"{nombreInstruccion} creado",
                    id = resultado.Id,
                    yaEstabaPendiente = resultado.YaEstabaPendiente,
                    pasos = pasosMovimiento
                });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        [HttpPost("regresar")]
        public IActionResult Regresar([FromBody(EmptyBodyBehavior = EmptyBodyBehavior.Allow)] int? pasos = null)
        {
            try
            {
                int pasosMovimiento = pasos ?? PasosMovimientoPredeterminados;
                if (pasosMovimiento <= 0)
                    return BadRequest(new { error = "Los pasos deben ser mayores que cero" });

                string nombreInstruccion = $"TELEFERICO_REGRESAR {pasosMovimiento}";

                var resultado = _instrucciones.CrearPendiente(nombreInstruccion, $"Regresar teleferico {pasosMovimiento} pasos");

                return Ok(new
                {
                    mensaje = resultado.YaEstabaPendiente
                        ? $"{nombreInstruccion} ya estaba en cola; no se duplico"
                        : $"{nombreInstruccion} creado",
                    id = resultado.Id,
                    yaEstabaPendiente = resultado.YaEstabaPendiente,
                    pasos = pasosMovimiento
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
                var resultado = _instrucciones.CrearPendiente("TELEFERICO_PARAR", "Detener teleferico");

                return Ok(new
                {
                    mensaje = resultado.YaEstabaPendiente
                        ? "TELEFERICO_PARAR ya estaba en cola; no se duplico"
                        : "TELEFERICO_PARAR creado",
                    id = resultado.Id,
                    yaEstabaPendiente = resultado.YaEstabaPendiente
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
                    @"SELECT TOP (@limite) 
                        id_log AS Id, 
                        nombre_instruccion AS Instruccion, 
                        resultado AS Resultado, 
                        angulo_resultado AS Pasos, 
                        tipo_ejecucion AS Tipo, 
                        fecha_ejecucion AS Fecha 
                      FROM Log_Instruccion 
                      WHERE nombre_instruccion LIKE 'TELEFERICO%'
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
}
