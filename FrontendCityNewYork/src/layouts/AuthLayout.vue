<script setup lang="ts">
import { useAuthStore } from '@/stores'
import { useRouter } from 'vue-router'

const authStore = useAuthStore()
const router = useRouter()

const handleLogout = () => {
  authStore.logout()
  router.push('/login')
}
</script>

<template>
  <div class="auth-layout">
    <header class="auth-header">
      <nav>
        <div class="user-info" v-if="authStore.isAuthenticated">
          <span>{{ authStore.userName }}</span>
          <button @click="handleLogout">Cerrar Sesión</button>
        </div>
      </nav>
    </header>
    <main class="auth-main">
      <slot />
    </main>
  </div>
</template>

<style scoped>
.auth-layout {
  min-height: 100vh;
  display: flex;
  flex-direction: column;
}

.auth-header {
  background: #1a1a2e;
  color: white;
  padding: 1rem 2rem;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
}

.auth-header nav {
  display: flex;
  justify-content: flex-end;
  align-items: center;
}

.user-info {
  display: flex;
  gap: 1rem;
  align-items: center;
}

.user-info button {
  background: #e94560;
  color: white;
  border: none;
  padding: 0.5rem 1rem;
  border-radius: 4px;
  cursor: pointer;
  transition: background 0.3s;
}

.user-info button:hover {
  background: #d63447;
}

.auth-main {
  flex: 1;
  padding: 2rem;
  background: #f5f5f5;
}
</style>
