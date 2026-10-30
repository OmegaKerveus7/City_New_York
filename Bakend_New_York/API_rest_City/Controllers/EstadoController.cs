using Microsoft.AspNetCore.Mvc;
using API_rest_City.Services;

namespace API_rest_City.Controllers
{
    /// <summary>
    /// Estado en vivo de los componentes de la maqueta.
    /// El ESP32 reporta el estado por POST y el dashboard lo consulta por GET.
    /// </summary>
    [ApiController]
    [Route("api/estado")]
    public class EstadoController : ControllerBase
    {
        private readonly EstadoService _estado;

        public EstadoController(EstadoService estado)
        {
            _estado = estado;
        }

        /// <summary>
        /// POST /api/estado - Telemetria enviada por el ESP32.
        /// Body: { "ipDispositivo": "192.168.1.10", "estado": { "PUENTE.ANGULO": "115", ... } }
        /// </summary>
        [HttpPost]
        public IActionResult Registrar([FromBody] EstadoRequest request)
        {
            try
            {
                if (request?.Estado == null || request.Estado.Count == 0)
                    return BadRequest(new { error = "El estado es obligatorio" });

                int actualizados = _estado.Actualizar(request.Estado, request.IpDispositivo);

                return Ok(new
                {
                    mensaje = "Estado actualizado",
                    componentes = actualizados
                });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        /// <summary>
        /// GET /api/estado/todos - Estado de todos los componentes (dashboard).
        /// </summary>
        [HttpGet("todos")]
        public IActionResult ObtenerTodos()
        {
            try
            {
                var componentes = _estado.ObtenerTodos();

                var ultimaActualizacion = componentes.Count > 0
                    ? componentes.Max(c => c.FechaActualizacion)
                    : (DateTime?)null;

                return Ok(new
                {
                    componentes,
                    total = componentes.Count,
                    ultimaActualizacion
                });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }

        /// <summary>
        /// GET /api/estado/{clave} - Estado de un componente concreto.
        /// Ejemplo: /api/estado/FUENTE.BOMBA
        /// </summary>
        [HttpGet("{clave}")]
        public IActionResult Obtener(string clave)
        {
            try
            {
                var componente = _estado.Obtener(clave);

                if (componente == null)
                    return NotFound(new { error = $"No hay estado para '{clave}'" });

                return Ok(componente);
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }
    }

    public class EstadoRequest
    {
        public string? IpDispositivo { get; set; }
        public Dictionary<string, string>? Estado { get; set; }
    }
}
