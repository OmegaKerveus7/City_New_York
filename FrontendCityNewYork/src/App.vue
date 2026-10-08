<script setup lang="ts">
import DashboardLayout from '@/layouts/DashboardLayout.vue'
</script>

<template>
  <DashboardLayout>
    <router-view />
  </DashboardLayout>
</template>

<style>
#app {
  width: 100%;
  margin: 0;
  min-height: 100vh;
}
</style>
