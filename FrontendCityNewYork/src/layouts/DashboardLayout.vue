<script setup lang="ts">
import { ref } from 'vue'
import { useAuthStore } from '@/stores'
import { useRouter } from 'vue-router'

const authStore = useAuthStore()
const router = useRouter()
const sidebarOpen = ref(true)
const expandedMenus = ref<string[]>(['punto-entrada'])

const menuItems = [
  {
    id: 'control-ciudad',
    label: 'Control de Ciudad',
    icon: 'city',
    route: '/',
    badge: null
  },
  {
    id: 'central-park',
    label: 'Control de Central Park',
    icon: 'park',
    route: '/central-park',
    badge: null
  },
  {
    id: 'punto-entrada',
    label: 'Punto de Entrada',
    icon: 'entry',
    badge: null,
    children: [
      {
        id: 'puente-elevadizo',
        label: 'Puente Elevadizo',
        icon: 'bridge',
        route: '/puente-elevadizo'
      }
    ]
  }
]

const handleLogout = () => {
  authStore.logout()
  router.push('/login')
}

const toggleSidebar = () => {
  sidebarOpen.value = !sidebarOpen.value
}

const navigateTo = (route: string) => {
  router.push(route)
}

const isActive = (route: string) => {
  return router.currentRoute.value.path === route
}

const toggleMenu = (menuId: string) => {
  if (expandedMenus.value.includes(menuId)) {
    expandedMenus.value = expandedMenus.value.filter(id => id !== menuId)
  } else {
    expandedMenus.value.push(menuId)
  }
}
</script>

<template>
  <div class="dashboard-layout" :class="{ 'sidebar-collapsed': !sidebarOpen }">
    <aside class="sidebar">
      <div class="sidebar__header">
        <div class="sidebar__brand">
          <img src="/logo_new_York.webp" alt="NYC" class="sidebar__logo" />
          <span v-if="sidebarOpen" class="sidebar__title">City of NY</span>
        </div>
        <button class="sidebar__toggle" @click="toggleSidebar">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <line x1="3" y1="12" x2="21" y2="12" />
            <line x1="3" y1="6" x2="21" y2="6" />
            <line x1="3" y1="18" x2="21" y2="18" />
          </svg>
        </button>
      </div>

      <nav class="sidebar__nav">
        <template v-for="item in menuItems" :key="item.id">
          <div v-if="item.children" class="nav-group">
            <button
              class="nav-item nav-item--group"
              :class="{ active: isActive(item.route) || item.children.some(c => isActive(c.route)) }"
              @click="toggleMenu(item.id)"
            >
              <span class="nav-icon">
                <svg v-if="item.icon === 'entry'" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                  <path d="M9 21H5a2 2 0 01-2-2V5a2 2 0 012-2h4M16 17l5-5-5-5M21 12H9" />
                </svg>
              </span>
              <span v-if="sidebarOpen" class="nav-label">{{ item.label }}</span>
              <span v-if="sidebarOpen" class="nav-arrow" :class="{ expanded: expandedMenus.includes(item.id) }">
                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                  <polyline points="6 9 12 15 18 9" />
                </svg>
              </span>
            </button>
            <div v-if="expandedMenus.includes(item.id) && sidebarOpen" class="nav-children">
              <button
                v-for="child in item.children"
                :key="child.id"
                class="nav-item nav-item--child"
                :class="{ active: isActive(child.route) }"
                @click="navigateTo(child.route)"
              >
                <span class="nav-icon">
                  <svg v-if="child.icon === 'bridge'" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                    <path d="M4 15s1-1 4-1 5 2 8 2 4-1 4-1V3s-1 1-4 1-5-2-8-2-4 1-4 1z" />
                    <line x1="4" y1="22" x2="4" y2="15" />
                  </svg>
                </span>
                <span class="nav-label">{{ child.label }}</span>
              </button>
            </div>
          </div>
          <button
            v-else
            class="nav-item"
            :class="{ active: isActive(item.route) }"
            @click="navigateTo(item.route)"
          >
            <span class="nav-icon">
              <svg v-if="item.icon === 'city'" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                <rect x="3" y="3" width="7" height="7" />
                <rect x="14" y="3" width="7" height="7" />
                <rect x="14" y="14" width="7" height="7" />
                <rect x="3" y="14" width="7" height="7" />
              </svg>
              <svg v-if="item.icon === 'park'" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                <path d="M12 3v18M5 10l7-7 7 7M5 21h14" />
              </svg>
            </span>
            <span v-if="sidebarOpen" class="nav-label">{{ item.label }}</span>
          </button>
        </template>
      </nav>

      <div class="sidebar__footer">
        <div class="user-card" v-if="authStore.usuario">
          <div class="user-avatar">
            {{ (authStore.usuario.NombreUsuario || '').charAt(0) || '' }}{{ (authStore.usuario.ApellidosUsuario || '').charAt(0) || '' }}
          </div>
          <div v-if="sidebarOpen" class="user-info">
            <span class="user-name">{{ authStore.userName || 'Usuario' }}</span>
            <span class="user-role">{{ authStore.usuario?.Rol || '' }}</span>
          </div>
        </div>
        <button class="logout-btn" @click="handleLogout">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <path d="M9 21H5a2 2 0 01-2-2V5a2 2 0 012-2h4" />
            <polyline points="16 17 21 12 16 7" />
            <line x1="21" y1="12" x2="9" y2="12" />
          </svg>
          <span v-if="sidebarOpen">Cerrar Sesión</span>
        </button>
      </div>
    </aside>

    <main class="main-content">
      <header class="topbar">
        <div class="topbar__left">
          <h1 class="page-title">{{ router.currentRoute.value.meta?.title || 'Panel de Control' }}</h1>
          <p class="page-subtitle">Ciudad de New York - Sistema de Gestión Municipal</p>
        </div>
        <div class="topbar__right">
          <div class="datetime">
            <span class="date">{{ new Date().toLocaleDateString('es-ES', { weekday: 'long', year: 'numeric', month: 'long', day: 'numeric' }) }}</span>
          </div>
        </div>
      </header>

      <div class="page-content">
        <router-view />
      </div>
    </main>
  </div>
</template>

<style scoped>
.dashboard-layout {
  display: flex;
  min-height: 100vh;
  background: var(--nyc-gray-100);
}

.sidebar {
  width: 260px;
  background: var(--nyc-navy);
  display: flex;
  flex-direction: column;
  transition: width 0.3s ease;
  position: fixed;
  height: 100vh;
  z-index: 100;
  overflow-y: auto;
  overflow-x: hidden;
}

.sidebar-collapsed .sidebar {
  width: 68px;
}

.sidebar__header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 1.25rem 1rem;
  border-bottom: 1px solid rgba(255,255,255,0.1);
  min-height: 72px;
}

.sidebar__brand {
  display: flex;
  align-items: center;
  gap: 0.75rem;
  overflow: hidden;
}

.sidebar__logo {
  width: 36px;
  height: 36px;
  border-radius: var(--radius-sm);
  object-fit: cover;
  flex-shrink: 0;
}

.sidebar__title {
  color: white;
  font-weight: 700;
  font-size: 1rem;
  white-space: nowrap;
}

.sidebar__toggle {
  background: rgba(255,255,255,0.1);
  border: none;
  color: white;
  width: 32px;
  height: 32px;
  border-radius: var(--radius-sm);
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  transition: background 0.2s;
  flex-shrink: 0;
}

.sidebar__toggle:hover {
  background: rgba(255,255,255,0.2);
}

.sidebar__toggle svg {
  width: 18px;
  height: 18px;
}

.sidebar__nav {
  flex: 1;
  padding: 1rem 0.75rem;
  display: flex;
  flex-direction: column;
  gap: 0.25rem;
}

.nav-group {
  display: flex;
  flex-direction: column;
}

.nav-item {
  display: flex;
  align-items: center;
  gap: 0.75rem;
  padding: 0.7rem 0.85rem;
  color: var(--nyc-gray-300);
  text-decoration: none;
  border-radius: var(--radius-md);
  font-size: 0.9rem;
  transition: all 0.2s;
  white-space: nowrap;
  background: none;
  border: none;
  width: 100%;
  text-align: left;
  cursor: pointer;
  font-family: inherit;
}

.nav-item:hover {
  background: rgba(255,255,255,0.1);
  color: white;
}

.nav-item.active {
  background: var(--nyc-taxi);
  color: var(--nyc-navy);
  font-weight: 600;
}

.nav-item--group {
  color: var(--nyc-gray-200);
}

.nav-item--group.active {
  color: white;
  background: rgba(255,255,255,0.1);
}

.nav-icon {
  width: 20px;
  height: 20px;
  flex-shrink: 0;
  display: flex;
  align-items: center;
  justify-content: center;
}

.nav-icon svg {
  width: 100%;
  height: 100%;
}

.nav-label {
  flex: 1;
}

.nav-arrow {
  width: 16px;
  height: 16px;
  transition: transform 0.2s;
}

.nav-arrow.expanded {
  transform: rotate(180deg);
}

.nav-arrow svg {
  width: 100%;
  height: 100%;
}

.nav-children {
  padding-left: 2.5rem;
  display: flex;
  flex-direction: column;
  gap: 0.15rem;
  margin-top: 0.15rem;
}

.nav-item--child {
  font-size: 0.85rem;
  padding: 0.55rem 0.75rem;
}

.nav-item--child.active {
  background: var(--nyc-taxi);
  color: var(--nyc-navy);
}

.sidebar__footer {
  padding: 1rem 0.75rem;
  border-top: 1px solid rgba(255,255,255,0.1);
  display: flex;
  flex-direction: column;
  gap: 0.75rem;
}

.user-card {
  display: flex;
  align-items: center;
  gap: 0.75rem;
}

.user-avatar {
  width: 36px;
  height: 36px;
  background: var(--nyc-taxi);
  color: var(--nyc-navy);
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  font-weight: 700;
  font-size: 0.8rem;
  flex-shrink: 0;
}

.user-info {
  display: flex;
  flex-direction: column;
  overflow: hidden;
}

.user-name {
  color: white;
  font-weight: 600;
  font-size: 0.85rem;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.user-role {
  color: var(--nyc-gray-300);
  font-size: 0.75rem;
}

.logout-btn {
  display: flex;
  align-items: center;
  gap: 0.6rem;
  padding: 0.6rem 0.85rem;
  background: rgba(229, 62, 62, 0.2);
  border: none;
  color: #FC8181;
  border-radius: var(--radius-md);
  cursor: pointer;
  font-size: 0.85rem;
  transition: all 0.2s;
  white-space: nowrap;
  font-family: inherit;
}

.logout-btn:hover {
  background: rgba(229, 62, 62, 0.4);
  color: #FEB2B2;
}

.logout-btn svg {
  width: 18px;
  height: 18px;
  flex-shrink: 0;
}

.main-content {
  flex: 1;
  margin-left: 260px;
  transition: margin-left 0.3s ease;
  min-height: 100vh;
  display: flex;
  flex-direction: column;
}

.sidebar-collapsed .main-content {
  margin-left: 68px;
}

.topbar {
  background: white;
  padding: 1.25rem 2rem;
  display: flex;
  justify-content: space-between;
  align-items: center;
  border-bottom: 1px solid var(--nyc-gray-200);
  box-shadow: var(--shadow-sm);
  position: sticky;
  top: 0;
  z-index: 50;
}

.page-title {
  font-size: 1.4rem;
  font-weight: 700;
  color: var(--nyc-navy);
  margin-bottom: 0.15rem;
}

.page-subtitle {
  font-size: 0.8rem;
  color: var(--nyc-steel-light);
}

.datetime .date {
  font-size: 0.85rem;
  color: var(--nyc-steel);
}

.page-content {
  padding: 2rem;
  flex: 1;
}
</style>
