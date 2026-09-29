using System.IdentityModel.Tokens.Jwt;
using System.Security.Claims;
using System.Text;
using API_rest_City.Models;
using Microsoft.IdentityModel.Tokens;

namespace API_rest_City.Services
{
    public class JwtService
    {
        private readonly IConfiguration _configuration;

        public JwtService(IConfiguration configuration)
        {
            _configuration = configuration;
        }

        public LoginResponse GenerarToken(Usuario usuario)
        {
            var key = _configuration["Jwt:Key"]!;
            var issuer = _configuration["Jwt:Issuer"];
            var audience = _configuration["Jwt:Audience"];
            var expirationMinutes = int.Parse(
                _configuration["Jwt:ExpirationMinutes"] ?? "60");

            var expiracion = DateTime.UtcNow.AddMinutes(expirationMinutes);

            var claims = new List<Claim>
            {
                new(JwtRegisteredClaimNames.Sub, usuario.Id.ToString()),
                new(JwtRegisteredClaimNames.Email, usuario.Correo),
                new(ClaimTypes.Name, usuario.NombreUsuario),
                new(ClaimTypes.Role, usuario.Rol)
            };

            var securityKey = new SymmetricSecurityKey(Encoding.UTF8.GetBytes(key));

            var credentials = new SigningCredentials(securityKey, SecurityAlgorithms.HmacSha256);

            var token = new JwtSecurityToken(
                issuer: issuer,
                audience: audience,
                claims: claims,
                expires: expiracion,
                signingCredentials: credentials);

            return new LoginResponse
            {
                Token = new JwtSecurityTokenHandler().WriteToken(token),
                Expira = expiracion
            };
        }
    }
}
