using Dapper;
using API_rest_City.BD;
using API_rest_City.Models;

namespace API_rest_City.Services
{
    /// <summary>
    /// Guarda el estado actual que reporta el ESP32 (telemetría).
    /// Una fila por componente, con clave "MODULO.CAMPO" (PUENTE.ANGULO, FUENTE.BOMBA...).
    /// </summary>
    public class EstadoService
    {
        private readonly DBContext _dbContext;

        public EstadoService(DBContext dbContext)
        {
            _dbContext = dbContext;
        }

        /// <summary>
        /// Inserta o actualiza el estado de los componentes reportados.
        /// </summary>
        public int Actualizar(Dictionary<string, string>? estado, string? ipDispositivo = null)
        {
            if (estado == null || estado.Count == 0)
                return 0;

            using var connection = _dbContext.CreateConnection();

            int actualizados = 0;

            foreach (var par in estado)
            {
                string clave = (par.Key ?? string.Empty).Trim();
                string valor = (par.Value ?? string.Empty).Trim();

                if (clave.Length == 0)
                    continue;

                string modulo = ModuloDe(clave);
                int? valorNumero = int.TryParse(valor, out int numero) ? numero : (int?)null;

                int filas = connection.Execute(
                    @"UPDATE Estado_Componente
                         SET modulo = @modulo,
                             valor = @valor,
                             valor_numero = @valorNumero,
                             texto = @texto,
                             ip_dispositivo = @ip,
                             fecha_actualizacion = GETDATE()
                       WHERE clave = @clave",
                    new { clave, modulo, valor, valorNumero, texto = valor, ip = ipDispositivo });

                if (filas == 0)
                {
                    connection.Execute(
                        @"INSERT INTO Estado_Componente (clave, modulo, valor, valor_numero, texto, ip_dispositivo)
                          VALUES (@clave, @modulo, @valor, @valorNumero, @texto, @ip)",
                        new { clave, modulo, valor, valorNumero, texto = valor, ip = ipDispositivo });
                }

                actualizados++;
            }

            return actualizados;
        }

        public List<EstadoComponente> ObtenerTodos()
        {
            using var connection = _dbContext.CreateConnection();
            return connection.Query<EstadoComponente>(
                @"SELECT clave AS Clave,
                         modulo AS Modulo,
                         valor AS Valor,
                         valor_numero AS ValorNumero,
                         texto AS Texto,
                         ip_dispositivo AS IpDispositivo,
                         fecha_actualizacion AS FechaActualizacion
                    FROM Estado_Componente
                   ORDER BY modulo, clave").ToList();
        }

        public List<EstadoComponente> ObtenerPorModulo(string modulo)
        {
            using var connection = _dbContext.CreateConnection();
            return connection.Query<EstadoComponente>(
                @"SELECT clave AS Clave,
                         modulo AS Modulo,
                         valor AS Valor,
                         valor_numero AS ValorNumero,
                         texto AS Texto,
                         ip_dispositivo AS IpDispositivo,
                         fecha_actualizacion AS FechaActualizacion
                    FROM Estado_Componente
                   WHERE modulo = @modulo
                   ORDER BY clave",
                new { modulo }).ToList();
        }

        public EstadoComponente? Obtener(string clave)
        {
            using var connection = _dbContext.CreateConnection();
            return connection.QueryFirstOrDefault<EstadoComponente>(
                @"SELECT clave AS Clave,
                         modulo AS Modulo,
                         valor AS Valor,
                         valor_numero AS ValorNumero,
                         texto AS Texto,
                         ip_dispositivo AS IpDispositivo,
                         fecha_actualizacion AS FechaActualizacion
                    FROM Estado_Componente
                   WHERE clave = @clave",
                new { clave });
        }

        /// <summary>
        /// El modulo es el prefijo de la clave: "PUENTE.ANGULO" -> "PUENTE".
        /// </summary>
        private static string ModuloDe(string clave)
        {
            int punto = clave.IndexOf('.');
            return punto > 0 ? clave.Substring(0, punto).ToUpperInvariant() : clave.ToUpperInvariant();
        }
    }
}
