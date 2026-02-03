/**
 * vite.config.ts - Vite 构建工具配置文件
 * 
 * 配置 Vite 开发服务器和构建选项
 * 包括：
 * - React 插件支持
 * - 开发服务器端口和主机配置
 * - 环境变量注入（Gemini API Key）
 * - 路径别名设置
 */

import path from 'path';
import { defineConfig, loadEnv } from 'vite';
import react from '@vitejs/plugin-react';

export default defineConfig(({ mode }) => {
    // 加载环境变量
    const env = loadEnv(mode, '.', '');
    
    return {
      // 开发服务器配置
      server: {
        port: 3000,           // 端口号
        host: '0.0.0.0',      // 监听所有网络接口
        allowedHosts: [
          'localhost',
          '127.0.0.1',
          'frp-six.com',
          '10.136.67.69'
        ]
      },
      
      // 插件配置
      plugins: [react()],     // 启用 React 支持
      
      // 定义全局常量（将环境变量注入到应用）
      define: {
        'process.env.API_KEY': JSON.stringify(env.GEMINI_API_KEY),
        'process.env.GEMINI_API_KEY': JSON.stringify(env.GEMINI_API_KEY)
      },
      
      // 解析配置
      resolve: {
        alias: {
          '@': path.resolve(__dirname, '.'),  // @ 别名指向根目录
        }
      }
    };
});
