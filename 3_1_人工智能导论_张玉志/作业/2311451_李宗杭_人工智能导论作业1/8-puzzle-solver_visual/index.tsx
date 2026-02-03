/**
 * index.tsx - 应用程序入口文件
 * 
 * 这是整个 React 应用的入口点
 * 负责：
 * 1. 获取 HTML 中的根 DOM 元素
 * 2. 创建 React 18 的并发模式根
 * 3. 在严格模式下渲染主应用组件
 */

import React from 'react';
import ReactDOM from 'react-dom/client';
import App from './App';

// 获取 HTML 文档中的根挂载点元素
const rootElement = document.getElementById('root');
if (!rootElement) {
  // 如果找不到根元素，抛出错误
  throw new Error("Could not find root element to mount to");
}

// 创建 React 18+ 的并发模式根
const root = ReactDOM.createRoot(rootElement);

// 在严格模式下渲染主应用组件
// StrictMode 会进行额外的检查和警告，帮助发现潜在问题
root.render(
  <React.StrictMode>
    <App />
  </React.StrictMode>
);