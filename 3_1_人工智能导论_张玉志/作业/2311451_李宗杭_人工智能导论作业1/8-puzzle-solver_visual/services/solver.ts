/**
 * solver.ts - 8-Puzzle 求解器核心实现
 * 
 * 本文件包含所有搜索算法的实现：
 * - BFS（广度优先搜索）
 * - DFS（深度优先搜索，带深度限制）
 * - A*（A星算法，支持三种启发式函数）
 * - Random（随机搜索）
 * 
 * 同时提供：
 * - 拼图可解性检查（基于逆序数）
 * - 随机可解棋盘生成
 * - 启发式函数（Manhattan、Euclidean、Misplaced Tiles）
 * - 异步进度回调支持
 */

import { AlgorithmType, Board, ProgressCallback, PuzzleState, SearchStats, SolverResult, Position } from '../types';
import { PriorityQueue } from './priorityQueue';

// 棋盘大小（3x3）
const SIZE = 3;

// 目标状态：按顺序排列，0（空格）在右下角
const GOAL_STATE: Board = [
  [1, 2, 3],
  [4, 5, 6],
  [7, 8, 0],
];

/**
 * 检查棋盘是否达到目标状态
 * 
 * @param board - 要检查的棋盘
 * @returns 是否为目标状态
 */
const isGoal = (board: Board): boolean => {
  for (let i = 0; i < SIZE; i++) {
    for (let j = 0; j < SIZE; j++) {
      if (board[i][j] !== GOAL_STATE[i][j]) return false;
    }
  }
  return true;
};

/**
 * 查找空格（0）在棋盘上的位置
 * 
 * @param board - 要搜索的棋盘
 * @returns 空格的位置坐标
 */
const findZero = (board: Board): Position => {
  for (let i = 0; i < SIZE; i++) {
    for (let j = 0; j < SIZE; j++) {
      if (board[i][j] === 0) return { row: i, col: j };
    }
  }
  return { row: -1, col: -1 };
};

/**
 * 将棋盘序列化为字符串（用于 Set/Map 的键）
 * 格式："1,2,3;4,5,6;7,8,0"
 * 
 * @param board - 要序列化的棋盘
 * @returns 序列化后的字符串
 */
const serialize = (board: Board): string => {
  return board.map(row => row.join(',')).join(';');
};

/**
 * 深拷贝棋盘（避免引用问题）
 * 
 * @param board - 要拷贝的棋盘
 * @returns 新的棋盘副本
 */
const copyBoard = (board: Board): Board => {
  return board.map(row => [...row]);
};

/**
 * 检查棋盘是否可解
 * 
 * 对于 3x3 的 8-Puzzle，可解性可以通过计算逆序数来判断：
 * - 将棋盘扁平化为一维数组（忽略 0）
 * - 计算逆序对的数量（i < j 但 arr[i] > arr[j]）
 * - 如果逆序数为偶数，则可解；否则不可解
 * 
 * @param board - 要检查的棋盘
 * @returns 是否可解
 */
export const isSolvable = (board: Board): boolean => {
  const flat: number[] = [];
  for (let i = 0; i < SIZE; i++) {
    for (let j = 0; j < SIZE; j++) {
      if (board[i][j] !== 0) flat.push(board[i][j]);
    }
  }

  let inversions = 0;
  for (let i = 0; i < flat.length; i++) {
    for (let j = i + 1; j < flat.length; j++) {
      if (flat[i] > flat[j]) inversions++;
    }
  }

  // 对于 3x3（奇数大小），逆序数为偶数时可解
  return inversions % 2 === 0;
};

/**
 * 生成随机可解的棋盘
 * 
 * 使用 Fisher-Yates 随机打乱算法生成 0-8 的随机排列，
 * 然后检查是否可解且不是已解决状态，
 * 如果不满足条件则重新生成。
 * 
 * @returns 随机生成的可解棋盘
 */
export const generateRandomBoard = (): Board => {
  let board: Board;
  do {
    // 生成 0-8 的随机排列
    const nums = Array.from({ length: 9 }, (_, i) => i);
    for (let i = nums.length - 1; i > 0; i--) {
      const j = Math.floor(Math.random() * (i + 1));
      [nums[i], nums[j]] = [nums[j], nums[i]];
    }
    
    // 将一维数组转换为 3x3 棋盘
    board = [];
    for (let i = 0; i < SIZE; i++) {
      board.push(nums.slice(i * 3, (i + 1) * 3));
    }
  } while (!isSolvable(board) || isGoal(board)); // 确保可解且不是目标状态
  return board;
};

/**
 * 生成当前状态的所有后继状态（邻居）
 * 
 * 通过将空格向四个方向（上、下、左、右）移动来生成新状态。
 * 每个有效移动都会生成一个新的后继状态。
 * 
 * @param state - 当前拼图状态
 * @returns 后继状态数组
 */
const getNeighbors = (state: PuzzleState): PuzzleState[] => {
  const neighbors: PuzzleState[] = [];
  const { row, col } = state.zeroPos;
  
  // 四个可能的移动方向
  const directions = [
    { r: -1, c: 0, m: 'Up' },
    { r: 1, c: 0, m: 'Down' },
    { r: 0, c: -1, m: 'Left' },
    { r: 0, c: 1, m: 'Right' },
  ];

  for (const dir of directions) {
    const newRow = row + dir.r;
    const newCol = col + dir.c;

    // 检查是否越界
    if (newRow >= 0 && newRow < SIZE && newCol >= 0 && newCol < SIZE) {
      const newBoard = copyBoard(state.board);
      // 交换空格和目标位置的元素
      [newBoard[row][col], newBoard[newRow][newCol]] = [newBoard[newRow][newCol], newBoard[row][col]];

      neighbors.push({
        board: newBoard,
        parent: state,
        move: dir.m,
        depth: state.depth + 1,
        zeroPos: { row: newRow, col: newCol },
        g: 0, h: 0, f: 0 // 将由算法设置
      });
    }
  }
  return neighbors;
};

/**
 * 重构从初始状态到目标状态的路径
 * 
 * 通过回溯父节点指针来重建完整路径。
 * 
 * @param state - 目标状态（路径的终点）
 * @returns 棋盘状态序列（从初始到目标）
 */
const getPath = (state: PuzzleState): Board[] => {
  const path: Board[] = [];
  let current: PuzzleState | null = state;
  while (current) {
    path.push(current.board);
    current = current.parent;
  }
  return path.reverse();
};

/**
 * 获取数字在目标状态中的坐标
 * 
 * @param val - 瓦片上的数字（0-8）
 * @returns 目标位置坐标
 */
const getCoordinates = (val: number): Position => {
  // 目标状态映射：1:(0,0), 2:(0,1), 3:(0,2), 4:(1,0)... 8:(2,1), 0:(2,2)
  if (val === 0) return { row: 2, col: 2 };
  const idx = val - 1;
  return { row: Math.floor(idx / 3), col: idx % 3 };
};

/**
 * Manhattan 距离启发式函数
 * 
 * 计算每个瓦片到其目标位置的曼哈顿距离（水平+垂直距离）之和。
 * 这是 A* 算法最常用的启发式，也是 8-Puzzle 的可容许启发式。
 * 
 * @param board - 当前棋盘状态
 * @returns 启发式值（估计到目标的代价）
 */
const manhattan = (board: Board): number => {
  let dist = 0;
  for (let r = 0; r < SIZE; r++) {
    for (let c = 0; c < SIZE; c++) {
      const val = board[r][c];
      if (val !== 0) {
        const target = getCoordinates(val);
        dist += Math.abs(r - target.row) + Math.abs(c - target.col);
      }
    }
  }
  return dist;
};

/**
 * Euclidean 距离启发式函数
 * 
 * 计算每个瓦片到其目标位置的欧几里得距离（直线距离）之和。
 * 注意：此启发式不是可容许的，因为实际移动只能水平或垂直。
 * 
 * @param board - 当前棋盘状态
 * @returns 启发式值
 */
const euclidean = (board: Board): number => {
  let dist = 0;
  for (let r = 0; r < SIZE; r++) {
    for (let c = 0; c < SIZE; c++) {
      const val = board[r][c];
      if (val !== 0) {
        const target = getCoordinates(val);
        dist += Math.sqrt(Math.pow(r - target.row, 2) + Math.pow(c - target.col, 2));
      }
    }
  }
  return dist;
};

/**
 * Misplaced Tiles 启发式函数
 * 
 * 计算不在正确位置上的瓦片数量。
 * 这是一个可容许但不是很强的启发式。
 * 
 * @param board - 当前棋盘状态
 * @returns 启发式值（错位瓦片数）
 */
const misplaced = (board: Board): number => {
  let count = 0;
  for (let r = 0; r < SIZE; r++) {
    for (let c = 0; c < SIZE; c++) {
      const val = board[r][c];
      if (val !== 0 && val !== GOAL_STATE[r][c]) {
        count++;
      }
    }
  }
  return count;
};

/**
 * Solver 类 - 8-Puzzle 求解器
 * 
 * 封装了所有搜索算法的实现，并提供统一的接口。
 */
export class Solver {
  private initialBoard: Board;       // 初始棋盘状态
  private algorithm: AlgorithmType;  // 选定的算法
  private maxDepth: number;          // DFS 算法的最大深度
  private maxIterations: number;     // Random 算法的最大迭代次数
  private startTime: number = 0;     // 搜索开始时间

  /**
   * 构造函数
   * 
   * @param board - 初始棋盘状态
   * @param algo - 选定的搜索算法
   * @param maxDepth - DFS 的最大深度（默认 50）
   * @param maxIterations - Random 的最大迭代次数（默认 100000）
   */
  constructor(board: Board, algo: AlgorithmType, maxDepth = 50, maxIterations = 100000) {
    this.initialBoard = board;
    this.algorithm = algo;
    this.maxDepth = maxDepth;
    this.maxIterations = maxIterations;
  }

  /**
   * 执行搜索算法
   * 
   * 根据选定的算法类型调用相应的实现方法。
   * 
   * @param onProgress - 进度回调函数，用于实时更新 UI
   * @returns 求解结果，包含路径、统计信息等
   */
  async solve(onProgress: ProgressCallback): Promise<SolverResult> {
    this.startTime = performance.now();
    const initialState: PuzzleState = {
      board: this.initialBoard,
      parent: null,
      move: 'Start',
      depth: 0,
      zeroPos: findZero(this.initialBoard),
      g: 0, h: 0, f: 0
    };

    // 根据算法类型调用相应的方法
    switch (this.algorithm) {
      case AlgorithmType.BFS:
        return this.bfs(initialState, onProgress);
      case AlgorithmType.DFS:
        return this.dfs(initialState, onProgress);
      case AlgorithmType.RANDOM:
        return this.randomSearch(initialState, onProgress);
      case AlgorithmType.A_STAR_MANHATTAN:
      case AlgorithmType.A_STAR_EUCLIDEAN:
      case AlgorithmType.A_STAR_MISPLACED:
        return this.aStar(initialState, onProgress);
      default:
        throw new Error("Unknown Algorithm");
    }
  }

  /**
   * BFS（广度优先搜索）算法实现
   * 
   * 使用队列（FIFO）系统地探索所有可达状态。
   * 特点：
   * - 保证找到最短路径（最优解）
   * - 空间复杂度较高，需要存储所有访问过的状态
   * - 适合解决方案较浅的问题
   * 
   * @param start - 初始状态
   * @param onProgress - 进度回调函数
   * @returns 求解结果
   */
  private async bfs(start: PuzzleState, onProgress: ProgressCallback): Promise<SolverResult> {
    const queue: PuzzleState[] = [start];
    const visited = new Set<string>();
    visited.add(serialize(start.board));
    
    const stats: SearchStats = {
      nodesExplored: 0,
      nodesGenerated: 1,
      timeElapsed: 0,
      memoryUsed: 0,
      pathLength: 0,
      success: false,
      maxDepthReached: 0
    };

    // 完整搜索历史（注意：BFS/DFS 的完整历史可能非常大，可能导致浏览器内存崩溃）
    // 通常只返回路径，或采样的历史记录
    const fullHistory: Board[] = [];

    while (queue.length > 0) {
      const current = queue.shift()!;
      stats.nodesExplored++;
      if (current.depth > stats.maxDepthReached) stats.maxDepthReached = current.depth;

      // 定期让出执行权给 UI 循环（每 200 个节点）
      if (stats.nodesExplored % 200 === 0) {
        stats.timeElapsed = performance.now() - this.startTime;
        stats.memoryUsed = (visited.size * 100) / 1024 / 1024; // 粗略估计：每个节点 100 字节
        const shouldContinue = await onProgress({ ...stats });
        if (!shouldContinue) return { path: [], stats, fullSearchHistory: [] };
      }

      // 检查是否达到目标状态
      if (isGoal(current.board)) {
        stats.success = true;
        stats.timeElapsed = performance.now() - this.startTime;
        stats.pathLength = current.depth;
        return { path: getPath(current), stats };
      }

      // 生成并处理所有邻居状态
      const neighbors = getNeighbors(current);
      for (const neighbor of neighbors) {
        const s = serialize(neighbor.board);
        if (!visited.has(s)) {
          visited.add(s);
          queue.push(neighbor);
          stats.nodesGenerated++;
        }
      }
    }
    
    // 未找到解决方案
    return { path: [], stats };
  }

  /**
   * DFS（深度优先搜索）算法实现
   * 
   * 使用栈（LIFO）深入探索每个分支直到达到深度限制或找到解。
   * 特点：
   * - 使用深度限制避免无限循环
   * - 空间复杂度较低（只需存储当前路径）
   * - 不保证找到最优解
   * - 对于深层解决方案可能效率低下
   * 
   * @param start - 初始状态
   * @param onProgress - 进度回调函数
   * @returns 求解结果
   */
  private async dfs(start: PuzzleState, onProgress: ProgressCallback): Promise<SolverResult> {
    const stack: PuzzleState[] = [start];
    // 保留访问集合用于循环检测
    // 注意：8-puzzle 的标准 DFS 需要访问集合来避免无限循环，因为图是循环的
    const visited = new Set<string>();
    visited.add(serialize(start.board));

    const stats: SearchStats = {
      nodesExplored: 0,
      nodesGenerated: 1,
      timeElapsed: 0,
      memoryUsed: 0,
      pathLength: 0,
      success: false,
      maxDepthReached: 0
    };

    while (stack.length > 0) {
      const current = stack.pop()!;
      stats.nodesExplored++;
      if (current.depth > stats.maxDepthReached) stats.maxDepthReached = current.depth;

      // 定期让出执行权给 UI
      if (stats.nodesExplored % 200 === 0) {
        stats.timeElapsed = performance.now() - this.startTime;
        stats.memoryUsed = (stats.nodesGenerated * 100) / 1024 / 1024; 
        const shouldContinue = await onProgress({ ...stats });
        if (!shouldContinue) return { path: [], stats };
      }

      // 检查是否达到目标
      if (isGoal(current.board)) {
        stats.success = true;
        stats.timeElapsed = performance.now() - this.startTime;
        stats.pathLength = current.depth;
        return { path: getPath(current), stats };
      }

      // 检查深度限制
      if (current.depth >= this.maxDepth) continue;

      const neighbors = getNeighbors(current);
      // 反向遍历以匹配典型的栈顺序（可选）
      for (let i = neighbors.length - 1; i >= 0; i--) {
        const neighbor = neighbors[i];
        
        // 优化：不回退到直接父节点（虽然访问集合已处理，但这个检查更便宜）
        if (current.parent && serialize(neighbor.board) === serialize(current.parent.board)) continue;
        
        // 简化的 DFS：检查全局访问集合
        // 对于纯树搜索 DFS，我们可能跟踪路径访问，但对于拼图图，全局访问更安全/更快
        // 但是，标准 DFS 可能找到次优路径
        const s = serialize(neighbor.board);
        
        // 只是标准的访问检查以防止循环
        if (!visited.has(s)) {
             visited.add(s);
             stack.push(neighbor);
             stats.nodesGenerated++;
        }
      }
    }

    // 未找到解决方案
    return { path: [], stats };
  }

  /**
   * A* 算法实现
   * 
   * 使用启发式函数引导搜索，结合了最佳优先搜索和 Dijkstra 算法的优点。
   * 评估函数：f(n) = g(n) + h(n)
   * - g(n)：从起点到节点 n 的实际代价
   * - h(n)：从节点 n 到目标的启发式估计代价
   * 
   * 特点：
   * - 如果启发式是可容许的（不过高估计），保证找到最优解
   * - 比 BFS 更高效，因为启发式引导搜索方向
   * - 空间复杂度与 BFS 相似，需要维护开放和封闭集合
   * 
   * @param start - 初始状态
   * @param onProgress - 进度回调函数
   * @returns 求解结果
   */
  private async aStar(start: PuzzleState, onProgress: ProgressCallback): Promise<SolverResult> {
    // 根据算法类型选择启发式函数
    let heuristic: (b: Board) => number;
    switch(this.algorithm) {
        case AlgorithmType.A_STAR_EUCLIDEAN: heuristic = euclidean; break;
        case AlgorithmType.A_STAR_MISPLACED: heuristic = misplaced; break;
        default: heuristic = manhattan;
    }

    // 初始化起始状态的代价值
    start.g = 0;
    start.h = heuristic(start.board);
    start.f = start.g + start.h;

    // 开放列表：使用优先队列，按 f 值排序
    const openList = new PriorityQueue<PuzzleState>();
    openList.enqueue(start, start.f);

    // 开放集合：跟踪状态 -> g 值的映射（用于快速查找）
    const openSet = new Map<string, number>();
    openSet.set(serialize(start.board), 0);

    // 封闭集合：已探索的状态
    const closedSet = new Set<string>();
    
    const stats: SearchStats = {
      nodesExplored: 0,
      nodesGenerated: 1,
      timeElapsed: 0,
      memoryUsed: 0,
      pathLength: 0,
      success: false,
      maxDepthReached: 0
    };

    // 捕获探索的节点用于"显示完整过程"功能
    const searchHistory: Board[] = [];
    
    while (!openList.isEmpty()) {
      const current = openList.dequeue()!;
      const currentStr = serialize(current.board);
      
      // 如果已经处理过此节点的更好路径，跳过（延迟删除）
      if (closedSet.has(currentStr)) continue;
      
      closedSet.add(currentStr);
      openSet.delete(currentStr); // 从开放映射跟踪中移除
      
      stats.nodesExplored++;
      if (current.depth > stats.maxDepthReached) stats.maxDepthReached = current.depth;
      searchHistory.push(current.board);

      // 定期让出执行权（每 100 个节点）
      if (stats.nodesExplored % 100 === 0) {
        stats.timeElapsed = performance.now() - this.startTime;
        stats.memoryUsed = ((closedSet.size + openList.size()) * 150) / 1024 / 1024; 
        const shouldContinue = await onProgress({ ...stats });
        if (!shouldContinue) return { path: [], stats, fullSearchHistory: searchHistory };
      }

      // 检查是否达到目标
      if (isGoal(current.board)) {
        stats.success = true;
        stats.timeElapsed = performance.now() - this.startTime;
        stats.pathLength = current.depth;
        return { path: getPath(current), stats, fullSearchHistory: searchHistory };
      }

      // 处理当前节点的所有邻居
      const neighbors = getNeighbors(current);
      for (const neighbor of neighbors) {
        const neighborStr = serialize(neighbor.board);
        
        // 跳过已在封闭集合中的节点
        if (closedSet.has(neighborStr)) continue;

        // 计算通过当前节点到达邻居的 g 值
        const tentativeG = current.g + 1;
        
        // 如果邻居不在开放集合中，或者找到了更好的路径
        if (!openSet.has(neighborStr) || tentativeG < (openSet.get(neighborStr) ?? Infinity)) {
            // 更新邻居的代价值
            neighbor.g = tentativeG;
            neighbor.h = heuristic(neighbor.board);
            neighbor.f = neighbor.g + neighbor.h;
            neighbor.parent = current;
            
            // 添加到开放集合
            openSet.set(neighborStr, tentativeG);
            openList.enqueue(neighbor, neighbor.f);
            stats.nodesGenerated++;
        }
      }
    }

    // 未找到解决方案
    return { path: [], stats, fullSearchHistory: searchHistory };
  }

  /**
   * 随机搜索算法实现
   * 
   * 从当前状态随机选择一个邻居进行移动，重复直到找到解或达到最大迭代次数。
   * 这不是一个实用的算法，主要用于对比实验和教学目的。
   * 
   * 特点：
   * - 内存使用恒定（只保留当前状态）
   * - 不保证找到解（可能陷入循环）
   * - 效率极低，纯粹靠运气
   * - 用于展示启发式搜索的重要性
   * 
   * @param start - 初始状态
   * @param onProgress - 进度回调函数
   * @returns 求解结果
   */
  private async randomSearch(start: PuzzleState, onProgress: ProgressCallback): Promise<SolverResult> {
    let current = start;
    const stats: SearchStats = {
      nodesExplored: 0,
      nodesGenerated: 1,
      timeElapsed: 0,
      memoryUsed: 0,
      pathLength: 0,
      success: false,
      maxDepthReached: 0
    };

    const history: Board[] = [current.board];

    for (let i = 0; i < this.maxIterations; i++) {
        stats.nodesExplored++;
        
        // 定期让出执行权（每 1000 次迭代）
        if (i % 1000 === 0) {
            stats.timeElapsed = performance.now() - this.startTime;
            stats.memoryUsed = 0.5; // 基本上是常量内存
            const shouldContinue = await onProgress({ ...stats });
            if (!shouldContinue) return { path: [], stats, fullSearchHistory: history };
        }

        // 检查是否达到目标
        if (isGoal(current.board)) {
            stats.success = true;
            stats.timeElapsed = performance.now() - this.startTime;
            stats.pathLength = current.depth;
            return { path: getPath(current), stats, fullSearchHistory: history };
        }

        // 获取所有可能的邻居
        const neighbors = getNeighbors(current);
        if (neighbors.length === 0) break; // 理论上不应该发生
        
        // 随机选择一个邻居
        const next = neighbors[Math.floor(Math.random() * neighbors.length)];
        
        // 防止立即回退到父节点（稍微改善随机游走）
        if (current.parent && serialize(next.board) === serialize(current.parent.board) && neighbors.length > 1) {
            // 过滤掉父节点后重新随机选择
            const filtered = neighbors.filter(n => serialize(n.board) !== serialize(current.parent!.board));
            current = filtered[Math.floor(Math.random() * filtered.length)];
        } else {
            current = next;
        }
        
        history.push(current.board);
        stats.nodesGenerated++;
        if (current.depth > stats.maxDepthReached) stats.maxDepthReached = current.depth;
    }

    // 达到最大迭代次数，未找到解
    return { path: [], stats, fullSearchHistory: history };
  }
}