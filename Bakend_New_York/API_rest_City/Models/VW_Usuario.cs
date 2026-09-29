namespace API_rest_City.Models
{
    public class VW_Usuario
    {
        public int ID_USUARIO { get; set; }
        public string NOMBRE_USUARIO { get; set; } = string.Empty;
        public string APELLIDOS_USUARIO { get; set; } = string.Empty;
        public string CORREO { get; set; } = string.Empty;
        public string CONTRASENA { get; set; } = string.Empty;
        public int ID_ROL { get; set; }
        public string NOMBRE_ROL { get; set; } = string.Empty;
        public bool ACTIVO { get; set; }
    }
}
