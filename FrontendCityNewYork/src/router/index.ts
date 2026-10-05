import { createRouter, createWebHistory } from 'vue-router'

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

  if (to.meta.requiresAuth) {
    if (!localStorage.getItem('token')) {
      next('/login')
    } else {
      next()
    }
  } else if (to.meta.requiresGuest) {
    if (localStorage.getItem('token')) {
      next('/')
    } else {
      next()
    }
  } else {
    next()
  }
})

export default router
