using API_rest_City.BD;
using API_rest_City.DTO;
using API_rest_City.Models;
using API_rest_City.Services;
using Microsoft.AspNetCore.Mvc;

namespace API_rest_City.Controllers
{
    [ApiController]
    [Route("api/[controller]")]
    public class AuthController : ControllerBase
    {
        private readonly JwtService _jwtService;
        private readonly DBContext _dbConnectionFactory;

        public AuthController(DBContext dbConnectionFactory, JwtService jwtService)
        {
            _dbConnectionFactory = dbConnectionFactory;
            _jwtService = jwtService;
        }

        [HttpPost("login")]
        public async Task<IActionResult> Login([FromBody] LoginRequest request)
        {
            ResponseDto respuesta = new ResponseDto();

            try
            {
                if (!ModelState.IsValid)
                {
                    var mensaje = ModelState.Values
                        .SelectMany(v => v.Errors)
                        .Select(e => e.ErrorMessage);

                    var mensajeError = string.Join(" | ", mensaje);
                    throw new Exception(mensajeError);
                }

                AuthDTO authDTO = new AuthDTO();
                authDTO.SetConexion(_dbConnectionFactory);

                respuesta = await authDTO.Login(request);

                if (respuesta.codigo != 200)
                {
                    return Unauthorized(new { mensaje = respuesta.mensaje });
                }

                VW_Usuario vw_usuario = (VW_Usuario)respuesta.data!;

                if (vw_usuario == null)
                {
                    return Unauthorized(new { mensaje = "Usuario o contraseña incorrectos" });
                }

                Usuario usuario = new Usuario
                {
                    Id = vw_usuario.ID_USUARIO,
                    NombreUsuario = vw_usuario.NOMBRE_USUARIO,
                    ApellidosUsuario = vw_usuario.APELLIDOS_USUARIO,
                    Correo = vw_usuario.CORREO,
                    Rol = vw_usuario.NOMBRE_ROL
                };

                return Ok(_jwtService.GenerarToken(usuario));
            }
            catch (Exception ex)
            {
                respuesta = new ResponseDto { codigo = 500, data = null, mensaje = "Login(): " + ex.Message };
                return StatusCode(500, respuesta);
            }
        }

        [HttpPost("registrar")]
        public async Task<IActionResult> CrearUsuario([FromBody] CrearUsuarioRequest request)
        {
            ResponseDto respuesta = new ResponseDto();

            try
            {
                if (!ModelState.IsValid)
                {
                    var mensaje = ModelState.Values
                        .SelectMany(v => v.Errors)
                        .Select(e => e.ErrorMessage);

                    var mensajeError = string.Join(" | ", mensaje);
                    throw new Exception(mensajeError);
                }

                AuthDTO authDTO = new AuthDTO();
                authDTO.SetConexion(_dbConnectionFactory);

                respuesta = await authDTO.CrearUsuario(request);

                if (respuesta.codigo != 200)
                {
                    return BadRequest(respuesta);
                }

                return Ok(respuesta);
            }
            catch (Exception ex)
            {
                respuesta = new ResponseDto { codigo = 500, data = null, mensaje = "CrearUsuario(): " + ex.Message };
                return StatusCode(500, respuesta);
            }
        }
    }
}
