using Dapper;
using API_rest_City.BD;
using Microsoft.AspNetCore.Mvc;
using System.Security.Cryptography;
using System.Text;

namespace API_rest_City.Controllers
{
    [ApiController]
    [Route("api/[controller]")]
    public class TestController : ControllerBase
    {
        private readonly DBContext _dbContext;

        public TestController(DBContext dbContext)
        {
            _dbContext = dbContext;
        }

        private string HashPassword(string password)
        {
            using var sha256 = SHA256.Create();
            var hashedBytes = sha256.ComputeHash(Encoding.UTF8.GetBytes(password));
            return BitConverter.ToString(hashedBytes).Replace("-", "").ToLower();
        }

        [HttpGet("conexion")]
        public IActionResult TestConexion()
        {
            try
            {
                using var connection = _dbContext.CreateConnection();
                connection.Open();
                return Ok(new
                {
                    codigo = 200,
                    mensaje = "Conexión a base de datos exitosa",
                    estado = true
                });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new
                {
                    codigo = 500,
                    mensaje = "Error de conexión: " + ex.Message,
                    estado = false
                });
            }
        }

        [HttpPost("crear-usuario")]
        public IActionResult TestCrearUsuario([FromBody] TestCrearUsuarioRequest request)
        {
            try
            {
                using var connection = _dbContext.CreateConnection();
                connection.Open();

                var hashedPassword = HashPassword(request.Contrasena);

                var parameters = new Dapper.DynamicParameters();
                parameters.Add("@NombreUsuario", request.Nombre);
                parameters.Add("@ApellidosUsuario", request.Apellidos);
                parameters.Add("@Correo", request.Correo);
                parameters.Add("@Contrasena", hashedPassword);
                parameters.Add("@IdRol", request.IdRol);

                var result = connection.QueryFirstOrDefault<dynamic>(
                    "SP_CrearUsuario",
                    parameters,
                    commandType: System.Data.CommandType.StoredProcedure
                );

                if (result != null)
                {
                    int idUsuario = (int)result.IdUsuario;
                    string mensaje = (string)result.Mensaje;

                    if (idUsuario == -1)
                    {
                        return BadRequest(new
                        {
                            codigo = 400,
                            mensaje = mensaje,
                            idUsuario = -1
                        });
                    }

                    return Ok(new
                    {
                        codigo = 200,
                        mensaje = mensaje,
                        idUsuario = idUsuario
                    });
                }

                return StatusCode(500, new
                {
                    codigo = 500,
                    mensaje = "Error al crear usuario"
                });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new
                {
                    codigo = 500,
                    mensaje = "Error: " + ex.Message
                });
            }
        }
    }

    public class TestCrearUsuarioRequest
    {
        public string Nombre { get; set; } = string.Empty;
        public string Apellidos { get; set; } = string.Empty;
        public string Correo { get; set; } = string.Empty;
        public string Contrasena { get; set; } = string.Empty;
        public int IdRol { get; set; } = 2;
    }
}
