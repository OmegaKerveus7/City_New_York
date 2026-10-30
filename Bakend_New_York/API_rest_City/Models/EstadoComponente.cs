namespace API_rest_City.Models
{
    /// <summary>
    /// Estado actual de un componente de la maqueta (puente, teleférico,
    /// elevador, fuente, semáforos). Lo reporta el ESP32 y lo consume el dashboard.
    /// </summary>
    public class EstadoComponente
    {
        public string Clave { get; set; } = string.Empty;
        public string Modulo { get; set; } = string.Empty;
        public string? Valor { get; set; }
        public int? ValorNumero { get; set; }
        public string? Texto { get; set; }
        public string? IpDispositivo { get; set; }
        public DateTime FechaActualizacion { get; set; }
    }
}
