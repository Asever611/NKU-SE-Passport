<div align="center">
<img width="1200" height="475" alt="GHBanner" src="https://github.com/user-attachments/assets/0aa67016-6eaf-458a-adb2-6e31a0763ed6" />
</div>

# 8-Puzzle AI 求解器

这是一个使用 React 和 TypeScript 构建的交互式 8-拼图求解器应用程序，可视化展示多种人工智能搜索算法如何解决经典的滑块拼图问题。

## 项目简介

8-拼图（8-Puzzle）是经典的人工智能问题之一。在一个 3×3 的网格中，有 8 个数字块（标记为 1-8）和一个空格，玩家可以通过将相邻的数字块滑入空格来重新排列它们。目标是从随机的初始状态到达目标状态（1-2-3 / 4-5-6 / 7-8-空）。

本项目实现了以下搜索算法：
- **BFS（广度优先搜索）** - 保证找到最优解
- **DFS（深度优先搜索）** - 使用深度限制防止无限循环
- **A\* (Manhattan 距离)** - 使用曼哈顿距离启发式的最优搜索
- **A\* (Euclidean 距离)** - 使用欧几里得距离启发式
- **A\* (Misplaced 瓦片)** - 使用错位瓦片数量启发式
- **Random（随机搜索）** - 随机移动探索，用于对比实验

## 主要功能

- 🎯 多种 AI 搜索算法选择
- 🎬 实时动画展示解决过程
- 📊 详细的性能统计（时间、内存、节点数等）
- 🔀 随机生成可解的初始状态
- ⚙️ 可调节的动画速度和算法参数
- 👀 可选的完整搜索过程可视化

## 项目结构

```
8-puzzle-ai-solver_v2/
├── index.html              # HTML 入口文件
├── index.tsx               # React 应用入口，渲染根组件
├── App.tsx                 # 主应用组件，包含核心逻辑和状态管理
├── types.ts                # TypeScript 类型定义
├── vite.config.ts          # Vite 构建工具配置
├── package.json            # 项目依赖和脚本配置
├── tsconfig.json           # TypeScript 编译器配置
├── metadata.json           # 项目元数据
├── README.md               # 项目说明文档
├── components/             # React 组件目录
│   ├── Controls.tsx        # 控制面板组件（算法选择、参数设置等）
│   ├── PuzzleBoard.tsx     # 拼图棋盘组件（可视化显示）
│   └── StatsPanel.tsx      # 统计面板组件（显示性能数据）
└── services/               # 核心服务目录
    ├── solver.ts           # 求解器核心实现（所有搜索算法）
    └── priorityQueue.ts    # 优先队列数据结构（用于A*算法）
```

## 文件说明

### 根目录文件

- **index.html** - 应用的 HTML 模板，包含 root 挂载点
- **index.tsx** - React 应用的入口文件，创建 React root 并渲染 App 组件
- **App.tsx** - 主应用组件，负责：
  - 管理拼图状态和算法选择
  - 处理求解和动画逻辑
  - 协调各子组件交互
- **types.ts** - 定义所有 TypeScript 类型和接口：
  - 棋盘类型、位置类型
  - 算法枚举
  - 搜索统计和结果接口
  - 拼图状态结构
- **vite.config.ts** - Vite 开发服务器和构建配置
- **package.json** - NPM 包管理配置，定义依赖和脚本
- **tsconfig.json** - TypeScript 编译器配置
- **README.md** - 本文档

### components/ 组件目录

- **Controls.tsx** - 控制面板组件
  - 算法选择下拉菜单
  - DFS 最大深度设置
  - Random 搜索最大迭代次数设置
  - 动画速度滑块
  - "显示完整搜索过程"开关
  - 随机生成和开始/停止按钮

- **PuzzleBoard.tsx** - 拼图棋盘视觉组件
  - 3×3 网格显示
  - 数字方块动画过渡
  - 正确/错误位置的颜色编码（绿色/黄色）
  - 响应式布局

- **StatsPanel.tsx** - 统计面板组件
  - 显示实时搜索统计
  - 时间消耗、内存使用
  - 节点探索数、节点生成数
  - 解决方案路径长度
  - 成功/失败状态指示

### services/ 服务目录

- **solver.ts** - 核心求解器实现
  - 实现所有搜索算法（BFS, DFS, A*, Random）
  - 拼图可解性检查（逆序数判断）
  - 随机棋盘生成
  - 启发式函数（Manhattan、Euclidean、Misplaced）
  - 进度回调支持，允许 UI 实时更新

- **priorityQueue.ts** - 优先队列数据结构
  - 用于 A* 算法的开放列表
  - 基于优先级排序
  - 支持 FIFO tie-breaking（同优先级先进先出）

## 本地运行

**前置要求：** Node.js (推荐 v16+)

1. **安装依赖：**
   ```bash
   npm install
   ```

2. **启动开发服务器：**
   ```bash
   npm run dev
   ```

3. **访问应用：**
   打开浏览器访问 `http://localhost:3000`

4. **构建生产版本：**
   ```bash
   npm run build
   ```

## 使用说明

1. **选择算法** - 从下拉菜单中选择要使用的搜索算法
2. **调整参数** - 根据算法类型调整相关参数（如 DFS 的最大深度）
3. **点击"Randomize"** - 生成一个新的随机可解初始状态
4. **点击"Start Solving"** - 开始求解
5. **观察过程** - 实时查看统计数据和求解动画
6. **调整速度** - 使用速度滑块控制动画播放速度
7. **显示搜索过程** - 启用"显示完整搜索过程"可看到算法探索的所有状态（注意：可能很慢）

## 技术栈

- **前端框架：** React 19.2
- **语言：** TypeScript 5.8
- **构建工具：** Vite 6.2
- **UI图标：** Lucide React
- **样式：** Tailwind CSS（通过内联类）

## 算法复杂度对比

| 算法 | 时间复杂度 | 空间复杂度 | 最优性 | 完备性 |
|------|-----------|-----------|--------|--------|
| BFS | O(b^d) | O(b^d) | ✅ | ✅ |
| DFS | O(b^m) | O(bm) | ❌ | ✅ (有深度限制) |
| A* | O(b^d) | O(b^d) | ✅ (使用可采纳启发式) | ✅ |
| Random | O(∞) | O(1) | ❌ | ❌ |

其中：
- b = 分支因子（平均约为 2-3）
- d = 解的深度
- m = 最大深度

## 项目特点

- ✅ 异步处理，UI 不会冻结
- ✅ 通过定期 yield 实现进度回调
- ✅ 内存优化的状态存储
- ✅ 可中断的搜索过程
- ✅ 响应式设计，支持移动端
- ✅ 详细的性能监控

## 开发者信息

本项目使用 React 18+ 的并发特性和异步模式优化，通过 Async/Await yielding 模式避免阻塞主线程。

查看在线演示：https://ai.studio/apps/drive/1zDPsWXnyI9Wn6HRs8ndExAQeOHeSNsRa

## 许可证

MIT License
