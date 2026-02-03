/**
 * types.ts - 8-Puzzle AI 求解器类型定义文件
 * 
 * 本文件定义了整个应用程序中使用的所有 TypeScript 类型、接口和枚举
 * 包括：棋盘表示、算法类型、搜索统计、求解结果等
 */

// 棋盘类型：二维数组表示 3x3 的拼图，0 代表空格
export type Board = number[][];

// 位置类型：表示棋盘上的坐标（行、列）
export type Position = { row: number; col: number };

/**
 * 算法类型枚举
 * 定义了所有支持的搜索算法类型
 */
export enum AlgorithmType {
  BFS = 'BFS',                          // 广度优先搜索
  DFS = 'DFS',                          // 深度优先搜索
  A_STAR_MANHATTAN = 'A* (Manhattan)',  // A* 算法（曼哈顿距离启发式）
  A_STAR_EUCLIDEAN = 'A* (Euclidean)',  // A* 算法（欧几里得距离启发式）
  A_STAR_MISPLACED = 'A* (Misplaced)',  // A* 算法（错位瓦片数启发式）
  RANDOM = 'Random',                     // 随机搜索算法
}

// 瓦片数字显示主题
export enum TileTheme {
  ARABIC = 'arabic',    // 阿拉伯数字
  CHINESE = 'chinese',  // 中文数字
}

/**
 * 搜索统计接口
 * 记录算法执行过程中的各项性能指标
 */
export interface SearchStats {
  nodesExplored: number;    // 已探索的节点数量
  nodesGenerated: number;   // 已生成的节点总数
  timeElapsed: number;      // 已消耗的时间（毫秒）
  memoryUsed: number;       // 估算的内存使用量（MB）
  pathLength: number;       // 解决方案的路径长度（步数）
  success: boolean;         // 是否成功找到解决方案
  maxDepthReached: number;  // 搜索达到的最大深度
}

/**
 * 求解器结果接口
 * 包含完整的求解结果和统计信息
 */
export interface SolverResult {
  path: Board[];                  // 解决方案的步骤序列（从初始状态到目标状态）
  fullSearchHistory?: Board[];    // 完整的搜索历史记录（用于可视化整个搜索过程）
  stats: SearchStats;             // 搜索统计信息
}

/**
 * 拼图状态接口
 * 表示搜索树中的一个节点，包含状态和路径信息
 */
export interface PuzzleState {
  board: Board;                   // 当前棋盘状态
  parent: PuzzleState | null;     // 父节点（用于回溯路径）
  move: string;                   // 到达此状态的移动方向（'Up'、'Down'、'Left'、'Right'）
  depth: number;                  // 当前状态的深度（距离初始状态的步数）
  zeroPos: Position;              // 空格（0）的位置，用于快速生成邻居
  
  // A* 算法的代价函数值
  g: number;                      // 从起始节点到当前节点的实际代价
  h: number;                      // 从当前节点到目标节点的启发式估计代价
  f: number;                      // 总估计代价 f = g + h
}

/**
 * 进度回调函数类型
 * 用于在搜索过程中向 UI 报告进度
 * 
 * @param stats - 当前的搜索统计信息
 * @returns Promise<boolean> - 返回 false 表示应该停止搜索
 */
export type ProgressCallback = (stats: SearchStats) => Promise<boolean>;