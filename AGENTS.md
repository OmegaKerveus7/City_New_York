# City_New_York

## Project Structure
- `Bakend_New_York/API_rest_City/` — ASP.NET Core 8 minimal API project
- `BD_New_York/Conetnedor/docker-compose.yml` — SQL Server 2022 container

## Run the API
```bash
cd Bakend_New_York/API_rest_City
dotnet run
```
- HTTP: http://localhost:5238
- HTTPS: https://localhost:7137
- Default endpoint: `/weatherforecast`

## Database
SQL Server via Docker (port 1444 host → 1433 container):
```bash
cd BD_New_York/Conetnedor
docker compose up -d
```
- SA password: `Arquitectura2_New_york` (dev only)

## Build
```bash
dotnet build Bakend_New_York/API_rest_City/API_rest_City.csproj
```
