/**
 * App.tsx - 主应用组件
 * 
 * 这是整个 8-Puzzle AI 求解器的核心组件
 * 负责：
 * 1. 管理应用的全局状态（棋盘、算法、参数、统计等）
 * 2. 协调求解器执行和动画播放
 * 3. 处理用户交互（开始、停止、随机化）
 * 4. 整合所有子组件（棋盘、控制面板、统计面板）
 */

import React, { useState, useEffect, useRef } from 'react';
import PuzzleBoard from './components/PuzzleBoard';
import Controls from './components/Controls';
import StatsPanel from './components/StatsPanel';
import { AlgorithmType, Board, SearchStats, TileTheme } from './types';
import { generateRandomBoard, Solver } from './services/solver';

// 默认的初始棋盘状态（已解决状态）
const INITIAL_BOARD: Board = [
  [1, 2, 3],
  [4, 5, 6],
  [7, 8, 0]
];

// 初始统计数据（所有指标为零）
const INITIAL_STATS: SearchStats = {
  nodesExplored: 0,
  nodesGenerated: 0,
  timeElapsed: 0,
  memoryUsed: 0,
  pathLength: 0,
  success: false,
  maxDepthReached: 0
};

const App: React.FC = () => {
  // ==================== 状态管理 ====================
  
  // 当前棋盘状态（初始为随机生成的可解状态）
  const [board, setBoard] = useState<Board>(generateRandomBoard());
  
  // 选中的搜索算法
  const [algorithm, setAlgorithm] = useState<AlgorithmType>(AlgorithmType.A_STAR_MANHATTAN);
  
  // 动画播放速度（毫秒）
  const [speed, setSpeed] = useState<number>(200);
  
  // 是否显示完整搜索过程
  const [showProcess, setShowProcess] = useState<boolean>(false);
  
  // DFS 算法的最大深度限制
  const [maxDepth, setMaxDepth] = useState<number>(50);
  
  // Random 算法的最大迭代次数
  const [maxIterations, setMaxIterations] = useState<number>(10000);

  // 是否正在执行搜索算法
  const [isSearching, setIsSearching] = useState<boolean>(false);
  
  // 是否正在播放动画
  const [isAnimating, setIsAnimating] = useState<boolean>(false);

  // 瓦片数字显示主题
  const [tileTheme, setTileTheme] = useState<TileTheme>(TileTheme.ARABIC);
  
  // 搜索统计信息
  const [stats, setStats] = useState<SearchStats>(INITIAL_STATS);

  // ==================== 引用（用于异步控制）====================
  
  // 停止信号标志（用于中断搜索和动画）
  const stopSignalRef = useRef<boolean>(false);
  
  // 动画定时器引用（用于清理定时器）
  const animationTimeoutRef = useRef<number | null>(null);

  // ==================== 事件处理函数 ====================
  
  /**
   * 随机化按钮处理函数
   * 生成新的随机可解棋盘并重置统计信息
   */
  const handleRandomize = () => {
    setBoard(generateRandomBoard());
    setStats(INITIAL_STATS);
  };

  /**
   * 停止按钮处理函数
   * 停止当前正在执行的搜索或动画
   */
  const handleStop = () => {
    stopSignalRef.current = true;
    if (animationTimeoutRef.current) {
      clearTimeout(animationTimeoutRef.current);
      animationTimeoutRef.current = null;
    }
    setIsSearching(false);
    setIsAnimating(false);
  };

  /**
   * 动画播放函数
   * 按照设定的速度依次播放棋盘状态序列
   * 
   * @param sequence - 要播放的棋盘状态序列
   */
  const animateSequence = (sequence: Board[]) => {
    setIsAnimating(true);
    let index = 0;

    const playStep = () => {
      // 检查停止信号
      if (stopSignalRef.current) {
        setIsAnimating(false);
        return;
      }

      // 检查是否播放完毕
      if (index >= sequence.length) {
        setIsAnimating(false);
        return;
      }

      // 更新当前显示的棋盘状态
      setBoard(sequence[index]);
      index++;
      
      // 设置下一帧的定时器
      animationTimeoutRef.current = window.setTimeout(playStep, speed);
    };

    playStep();
  };

  /**
   * 开始求解按钮处理函数
   * 执行选定的搜索算法并在成功时播放动画
   */
  const handleStart = async () => {
    // 重置状态
    setStats(INITIAL_STATS);
    stopSignalRef.current = false;
    setIsSearching(true);

    // 创建求解器实例
    const solver = new Solver(board, algorithm, maxDepth, maxIterations);

    try {
      // 执行求解，传入进度回调函数
      const result = await solver.solve(async (currentStats) => {
        // 更新统计信息到 UI
        setStats(currentStats);
        
        // 让出执行权，允许 UI 更新（避免阻塞主线程）
        await new Promise(resolve => setTimeout(resolve, 0));
        
        // 返回是否应该继续（检查停止信号）
        return !stopSignalRef.current;
      });

      setIsSearching(false);

      // 如果手动停止了，直接返回
      if (stopSignalRef.current) return;

      // 更新最终统计信息
      setStats(result.stats);

      // 如果成功找到解
      if (result.stats.success) {
        // 准备动画序列
        let sequence = result.path;
        
        // 如果启用了"显示完整搜索过程"且有搜索历史
        if (showProcess && result.fullSearchHistory && result.fullSearchHistory.length > 0) {
            // 将搜索历史和解决方案路径合并
            // 注意：对于大规模搜索，这可能会导致非常长的动画
            sequence = [...result.fullSearchHistory, ...result.path];
        }

        // 添加短暂延迟后开始播放动画（让用户先看到统计数据）
        setTimeout(() => {
            if (!stopSignalRef.current) animateSequence(sequence);
        }, 500);
      }

    } catch (e) {
      console.error(e);
      setIsSearching(false);
    }
  };

  /**
   * 组件卸载时的清理函数
   * 确保清理所有定时器和停止所有异步操作
   */
  useEffect(() => {
    return () => {
      stopSignalRef.current = true;
      if (animationTimeoutRef.current) clearTimeout(animationTimeoutRef.current);
    };
  }, []);

  // ==================== 渲染 ====================
  
  return (
    <div className="min-h-screen bg-slate-900 text-slate-100 p-4 md:p-8 flex flex-col items-center">
      {/* 页面头部 */}
      <header className="my-8 text-center space-y-2">
        <h1 className="text-3xl md:text-4xl font-extrabold text-transparent bg-clip-text bg-gradient-to-r from-blue-400 to-emerald-400">
          8-Puzzle Solver
        </h1>
      </header>

      {/* 主内容区域 - 左右两列布局 */}
      <main className="w-full max-w-6xl grid grid-cols-1 lg:grid-cols-2 gap-8 lg:gap-12 items-center flex-grow">
        {/* 左列：拼图棋盘 */}
        <div className="flex flex-col items-center space-y-6">
          <PuzzleBoard board={board} isAnimating={isAnimating} tileTheme={tileTheme} />
          
          {/* 状态指示徽章 - 固定高度预留空间 */}
          <div className="flex gap-4 justify-center min-h-[40px] items-center">
             {/* 搜索中指示器 */}
             {isSearching && (
                 <span className="inline-flex items-center gap-2 px-3 py-1 rounded-full bg-blue-900/50 text-blue-300 border border-blue-700/50 text-sm animate-pulse">
                     <span className="w-2 h-2 rounded-full bg-blue-400"></span>
                     Running Algorithm...
                 </span>
             )}
             {/* 动画播放中指示器 */}
             {isAnimating && (
                 <span className="inline-flex items-center gap-2 px-3 py-1 rounded-full bg-emerald-900/50 text-emerald-300 border border-emerald-700/50 text-sm">
                     <span className="w-2 h-2 rounded-full bg-emerald-400 animate-bounce"></span>
                     Animating Solution
                 </span>
             )}
          </div>
        </div>

        {/* 右列：统计面板和控制面板 */}
        <div className="space-y-3 flex flex-col w-full">
          {/* 统计面板组件 */}
          <StatsPanel stats={stats} searching={isSearching} />
          
          {/* 控制面板组件 */}
          <Controls 
            algorithm={algorithm}
            setAlgorithm={setAlgorithm}
            speed={speed}
            setSpeed={setSpeed}
            showProcess={showProcess}
            setShowProcess={setShowProcess}
            maxDepth={maxDepth}
            setMaxDepth={setMaxDepth}
            maxIterations={maxIterations}
            setMaxIterations={setMaxIterations}
            onRandomize={handleRandomize}
            onStart={handleStart}
            onStop={handleStop}
            isSearching={isSearching}
            isAnimating={isAnimating}
            stats={stats}
            tileTheme={tileTheme}
            onToggleTileTheme={() => setTileTheme(theme => theme === TileTheme.ARABIC ? TileTheme.CHINESE : TileTheme.ARABIC)}
          />
        </div>
      </main>
    </div>
  );
};

export default App;