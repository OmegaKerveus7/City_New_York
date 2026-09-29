using Dapper;
using API_rest_City.BD;
using API_rest_City.Models;
using Microsoft.Data.SqlClient;
using System.ComponentModel.DataAnnotations;

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

        public async Task<ResponseDto> Login(LoginRequest request)
        {
            ResponseDto respuesta = new ResponseDto();

            try
            {
                using var connection = _dbConnectionFactory!.CreateConnection();
                connection.Open();

                string sql = @"SELECT u.id_usuario AS ID_USUARIO, u.nombre_usuario AS NOMBRE_USUARIO, 
                               u.apellidos_usuario AS APELLIDOS_USUARIO, u.correo AS CORREO, 
                               u.contrasena AS CONTRASENA, r.id_rol AS ID_ROL, 
                               r.nombre_rol AS NOMBRE_ROL, u.activo AS ACTIVO
                               FROM Usuario u
                               INNER JOIN UsuarioRol ur ON u.id_usuario = ur.id_usuario
                               INNER JOIN RolUsuario r ON ur.id_rol_usuario = r.id_rol
                               WHERE u.correo = @Correo AND u.contrasena = @Contrasena AND u.activo = 1";

                var result = await connection.QueryAsync<VW_Usuario>(sql, new { request.Correo, request.Contrasena });
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

                using var transaction = connection.BeginTransaction();

                try
                {
                    string sqlUsuario = @"INSERT INTO Usuario (nombre_usuario, apellidos_usuario, correo, contrasena)
                                         VALUES (@NombreUsuario, @ApellidosUsuario, @Correo, @Contrasena);
                                         SELECT SCOPE_IDENTITY();";

                    int idUsuario = await connection.ExecuteScalarAsync<int>(sqlUsuario, new
                    {
                        request.NombreUsuario,
                        request.ApellidosUsuario,
                        request.Correo,
                        request.Contrasena
                    }, transaction);

                    string sqlRol = @"INSERT INTO UsuarioRol (id_usuario, id_rol_usuario)
                                      VALUES (@IdUsuario, @IdRol)";
                    await connection.ExecuteAsync(sqlRol, new { IdUsuario = idUsuario, request.IdRol }, transaction);

                    transaction.Commit();

                    respuesta.codigo = 200;
                    respuesta.data = idUsuario;
                    respuesta.mensaje = "Usuario creado exitosamente";
                }
                catch (Exception ex)
                {
                    transaction.Rollback();
                    throw new Exception(ex.Message);
                }
            }
            catch (Exception ex)
            {
                respuesta = new ResponseDto { codigo = 500, mensaje = "CrearUsuario(): " + ex.Message, data = null };
            }
            return respuesta;
        }
    }
}
