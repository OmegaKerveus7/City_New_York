import { createRouter, createWebHistory } from 'vue-router'
import { useAuthStore } from '@/stores'

const router = createRouter({
  history: createWebHistory(),
  routes: [
    {
      path: '/login',
      name: 'login',
      component: () => import('@/pages/LoginPage.vue'),
      meta: { requiresGuest: true }
    },
    {
      path: '/register',
      name: 'register',
      component: () => import('@/pages/RegisterPage.vue'),
      meta: { requiresGuest: true }
    },
    {
      path: '/',
      name: 'dashboard',
      component: () => import('@/pages/DashboardPage.vue'),
      meta: { requiresAuth: true, title: 'Control de Ciudad' }
    },
    {
      path: '/central-park',
      name: 'central-park',
      component: () => import('@/pages/CentralParkPage.vue'),
      meta: { requiresAuth: true, title: 'Control de Central Park' }
    },
    {
      path: '/puente-elevadizo',
      name: 'puente-elevadizo',
      component: () => import('@/pages/PuenteElevadizoPage.vue'),
      meta: { requiresAuth: true, title: 'Puente Elevadizo' }
    },
    {
      path: '/teleferico',
      name: 'teleferico',
      component: () => import('@/pages/TelefericoPage.vue'),
      meta: { requiresAuth: true, title: 'Teleferico' }
    },
    {
      path: '/empire-state/elevador',
      name: 'empire-state-elevador',
      component: () => import('@/pages/ElevadorPage.vue'),
      meta: { requiresAuth: true, title: 'Empire State - Elevador' }
    },
    {
      path: '/:pathMatch(.*)*',
      redirect: '/'
    }
  ]
})

let isInitialized = false

router.beforeEach((to, _from, next) => {
  if (!isInitialized) {
    isInitialized = true
  }

  const authStore = useAuthStore()

  if (to.meta.requiresAuth && !authStore.isAuthenticated) {
    next({ name: 'login' })
  } else if (to.meta.requiresGuest && authStore.isAuthenticated) {
    next({ name: 'dashboard' })
  } else {
    next()
  }
})

export default router
