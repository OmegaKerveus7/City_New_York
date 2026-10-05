import axios from 'axios'

const http = axios.create({
  baseURL: 'http://localhost:5238/api',
  timeout: 10000,
  headers: {
    'Content-Type': 'application/json'
  }
})

http.interceptors.request.use(
  (config) => {
    const token = localStorage.getItem('token')
    if (token) {
      config.headers.Authorization = `Bearer ${token}`
    }
    return config
  },
  (error) => Promise.reject(error)
)

http.interceptors.response.use(
  (response) => response,
  (error) => {
    console.error('HTTP Error:', error.response?.status, error.response?.data)
    if (error.response?.status === 401) {
      const isOnLoginPage = window.location.pathname === '/login'
      const token = localStorage.getItem('token')

      if (token && !isOnLoginPage) {
        localStorage.removeItem('token')
        localStorage.removeItem('usuario')
        window.location.href = '/login'
      }
    }
    return Promise.reject(error)
  }
)

export default http
