namespace API_rest_City.Models
{
    public class CrearUsuarioRequest
    {
        public string NombreUsuario { get; set; } = string.Empty;
        public string ApellidosUsuario { get; set; } = string.Empty;
        public string Correo { get; set; } = string.Empty;
        public string Contrasena { get; set; } = string.Empty;
        public int IdRol { get; set; }
    }
}
