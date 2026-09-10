import vue from '@vitejs/plugin-vue'
import { defineConfig } from 'vite'

export default defineConfig({
  plugins: [vue()],
  server: {
    proxy: {
      "/api": {
        target: "http://127.0.0.1:8080",
        changeOrigin: true,
        rewrite: (p) => {
          const r = p.replace(/^\/api/, "")
          return r.startsWith("/") ? r : "/" + r
        },
      },
    },
  },
})