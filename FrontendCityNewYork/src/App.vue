<script setup lang="ts">
import { computed } from 'vue'
import { useRoute } from 'vue-router'
import DashboardLayout from '@/layouts/DashboardLayout.vue'

const route = useRoute()
const isAuthPage = computed(() => route.meta.requiresGuest === true)
</script>

<template>
  <router-view v-if="isAuthPage" />
  <DashboardLayout v-else />
</template>

<style>
#app {
  width: 100%;
  margin: 0;
  min-height: 100vh;
}
</style>
