using Microsoft.Data.SqlClient;

namespace API_rest_City.BD
{
    public class DBContext
    {
        private readonly string _connectionString;

        public DBContext(IConfiguration configuration)
        {
            _connectionString = configuration.GetConnectionString("Ciudad_New_York")
                ?? throw new InvalidOperationException("Connection string 'Ciudad_New_York' not found.");
        }

        public SqlConnection CreateConnection()
        {
            return new SqlConnection(_connectionString);
        }
    }
}
