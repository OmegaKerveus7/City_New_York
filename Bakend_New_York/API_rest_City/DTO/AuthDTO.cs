using Dapper;
using API_rest_City.BD;
using API_rest_City.Models;
using Microsoft.Data.SqlClient;
using System.ComponentModel.DataAnnotations;
using System.Data;
using System.Security.Cryptography;
using System.Text;

namespace API_rest_City.DTO
{
    public class AuthDTO
    {
        public string? Correo { get; set; }
        public string? Contrasena { get; set; }
        public string? NombreUsuario { get; set; }
        public string? ApellidosUsuario { get; set; }
        public int? IdRol { get; set; }

        private DBContext? _dbConnectionFactory;

        public void SetConexion(DBContext dbConexion)
        {
            _dbConnectionFactory = dbConexion;
        }

        private string HashPassword(string password)
        {
            using var sha256 = SHA256.Create();
            var hashedBytes = sha256.ComputeHash(Encoding.UTF8.GetBytes(password));
            return BitConverter.ToString(hashedBytes).Replace("-", "").ToLower();
        }

        public async Task<ResponseDto> Login(LoginRequest request)
        {
            ResponseDto respuesta = new ResponseDto();

            try
            {
                using var connection = _dbConnectionFactory!.CreateConnection();
                connection.Open();

                var hashedPassword = HashPassword(request.Contrasena);

                var parameters = new DynamicParameters();
                parameters.Add("@Correo", request.Correo);
                parameters.Add("@Contrasena", hashedPassword);

                var result = await connection.QueryAsync<VW_Usuario>(
                    "SP_Login",
                    parameters,
                    commandType: CommandType.StoredProcedure
                );

                VW_Usuario? vw_usuario = result.FirstOrDefault();

                if (vw_usuario == null)
                {
                    return new ResponseDto { codigo = 401, mensaje = "Credenciales inválidas", data = null };
                }

                respuesta.codigo = 200;
                respuesta.data = vw_usuario;
                respuesta.mensaje = "Login exitoso";
            }
            catch (Exception ex)
            {
                respuesta = new ResponseDto { codigo = 500, mensaje = "Login(): " + ex.Message, data = null };
            }
            return respuesta;
        }

        public async Task<ResponseDto> CrearUsuario(CrearUsuarioRequest request)
        {
            ResponseDto respuesta = new ResponseDto();

            try
            {
                using var connection = _dbConnectionFactory!.CreateConnection();
                connection.Open();

                var hashedPassword = HashPassword(request.Contrasena);

                var parameters = new DynamicParameters();
                parameters.Add("@NombreUsuario", request.NombreUsuario);
                parameters.Add("@ApellidosUsuario", request.ApellidosUsuario);
                parameters.Add("@Correo", request.Correo);
                parameters.Add("@Contrasena", hashedPassword);
                parameters.Add("@IdRol", request.IdRol);

                var result = await connection.QueryFirstOrDefaultAsync<dynamic>(
                    "SP_CrearUsuario",
                    parameters,
                    commandType: CommandType.StoredProcedure
                );

                if (result != null)
                {
                    int idUsuario = (int)result.IdUsuario;
                    string mensaje = (string)result.Mensaje;

                    if (idUsuario == -1)
                    {
                        return new ResponseDto { codigo = 400, mensaje = mensaje, data = null };
                    }

                    respuesta.codigo = 200;
                    respuesta.data = idUsuario;
                    respuesta.mensaje = mensaje;
                }
                else
                {
                    respuesta.codigo = 500;
                    respuesta.mensaje = "Error al crear usuario";
                }
            }
            catch (Exception ex)
            {
                respuesta = new ResponseDto { codigo = 500, mensaje = "CrearUsuario(): " + ex.Message, data = null };
            }
            return respuesta;
        }

        public async Task<ResponseDto> ObtenerRoles()
        {
            ResponseDto respuesta = new ResponseDto();

            try
            {
                using var connection = _dbConnectionFactory!.CreateConnection();
                connection.Open();

                var result = await connection.QueryAsync<Rol>(
                    "SP_ObtenerRoles",
                    commandType: CommandType.StoredProcedure
                );

                respuesta.codigo = 200;
                respuesta.data = result.ToList();
                respuesta.mensaje = "Roles obtenidos exitosamente";
            }
            catch (Exception ex)
            {
                respuesta = new ResponseDto { codigo = 500, mensaje = "ObtenerRoles(): " + ex.Message, data = null };
            }
            return respuesta;
        }
    }
}
