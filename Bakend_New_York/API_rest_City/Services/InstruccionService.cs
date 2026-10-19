using Dapper;
using API_rest_City.BD;
using System.Data;

namespace API_rest_City.Services
{
    public class InstruccionService
    {
        private readonly DBContext _dbContext;

        public InstruccionService(DBContext dbContext)
        {
            _dbContext = dbContext;
        }

        /// <summary>
        /// Funcion unica para encolar comandos: si la instruccion no existe la crea
        /// y luego la agrega a la cola de pendientes. Solo se pasa el nombre.
        /// </summary>
        public (int Id, bool YaEstabaPendiente) CrearPendiente(string nombre, string? descripcion = null)
        {
            if (string.IsNullOrWhiteSpace(nombre))
                throw new ArgumentException("El nombre de la instruccion es obligatorio", nameof(nombre));

            using var connection = _dbContext.CreateConnection();
            connection.Open();
            using var transaction = connection.BeginTransaction(IsolationLevel.Serializable);

            var idInstruccion = connection.QueryFirstOrDefault<int?>(
                "SELECT id_instruccion FROM Instruccion WITH (UPDLOCK, HOLDLOCK) WHERE nombre = @nombre",
                new { nombre },
                transaction);

            if (!idInstruccion.HasValue)
            {
                connection.Execute(
                    "INSERT INTO Instruccion (nombre, descripcion, activa) VALUES (@nombre, @descripcion, 1)",
                    new { nombre, descripcion = descripcion ?? nombre },
                    transaction);

                idInstruccion = connection.QueryFirstOrDefault<int?>(
                    "SELECT id_instruccion FROM Instruccion WHERE nombre = @nombre",
                    new { nombre },
                    transaction);
            }

            if (!idInstruccion.HasValue)
                throw new InvalidOperationException($"No se pudo crear la instruccion '{nombre}'");

            bool esMovimientoLargo =
                nombre.StartsWith("TELEFERICO_AVANZAR", StringComparison.OrdinalIgnoreCase) ||
                nombre.StartsWith("TELEFERICO_REGRESAR", StringComparison.OrdinalIgnoreCase) ||
                nombre.StartsWith("SUBIR_NIVEL", StringComparison.OrdinalIgnoreCase) ||
                nombre.StartsWith("BAJAR_NIVEL", StringComparison.OrdinalIgnoreCase) ||
                nombre.StartsWith("IR_A_NIVEL", StringComparison.OrdinalIgnoreCase) ||
                nombre.StartsWith("CALIBRAR", StringComparison.OrdinalIgnoreCase);

            var pendienteExistente = connection.QueryFirstOrDefault<int?>(
                @"SELECT TOP 1 id_pendiente
                  FROM Instruccion_Pendiente WITH (UPDLOCK, HOLDLOCK)
                  WHERE id_instruccion = @idInstruccion
                    AND (
                        procesada = 0
                        OR (@esMovimientoLargo = 1
                            AND fecha_procesamiento >= DATEADD(SECOND, -15, GETDATE()))
                    )
                  ORDER BY id_pendiente DESC",
                new { idInstruccion, esMovimientoLargo },
                transaction);

            if (pendienteExistente.HasValue)
            {
                transaction.Commit();
                return (pendienteExistente.Value, true);
            }

            int idPendiente = connection.QuerySingle<int>(
                @"INSERT INTO Instruccion_Pendiente (id_instruccion)
                  VALUES (@idInstruccion);
                  SELECT CAST(SCOPE_IDENTITY() AS INT);",
                new { idInstruccion },
                transaction);

            transaction.Commit();
            return (idPendiente, false);
        }
    }
}
