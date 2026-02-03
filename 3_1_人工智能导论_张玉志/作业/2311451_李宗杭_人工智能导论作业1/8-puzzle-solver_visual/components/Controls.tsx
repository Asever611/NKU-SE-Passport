/**
 * Controls.tsx - 控制面板组件
 * 
 * 提供用户界面用于：
 * 1. 选择搜索算法
 * 2. 配置算法特定参数（DFS最大深度、Random最大迭代次数）
 * 3. 调整动画速度
 * 4. 开启/关闭完整搜索过程显示
 * 5. 执行操作（随机化、开始求解、停止）
 */

import React from 'react';
import { AlgorithmType, SearchStats, TileTheme } from '../types';
import { Play, Square, Shuffle, Settings, CheckCircle, XCircle, Languages } from 'lucide-react';

/**
 * 控制面板组件的属性接口
 */
interface ControlsProps {
  algorithm: AlgorithmType;                    // 当前选中的算法
  setAlgorithm: (a: AlgorithmType) => void;   // 设置算法的回调函数
  speed: number;                               // 动画速度（毫秒）
  setSpeed: (s: number) => void;              // 设置速度的回调函数
  showProcess: boolean;                        // 是否显示完整搜索过程
  setShowProcess: (b: boolean) => void;       // 设置显示过程的回调函数
  maxDepth: number;                            // DFS 的最大深度
  setMaxDepth: (n: number) => void;           // 设置最大深度的回调函数
  maxIterations: number;                       // Random 的最大迭代次数
  setMaxIterations: (n: number) => void;      // 设置最大迭代次数的回调函数
  onRandomize: () => void;                     // 随机化按钮的回调函数
  onStart: () => void;                         // 开始求解按钮的回调函数
  onStop: () => void;                          // 停止按钮的回调函数
  isSearching: boolean;                        // 是否正在搜索
  isAnimating: boolean;                        // 是否正在播放动画
  stats: SearchStats;                          // 搜索统计信息
  tileTheme: TileTheme;                        // 数字显示主题
  onToggleTileTheme: () => void;               // 换肤按钮回调
}

const Controls: React.FC<ControlsProps> = ({
  algorithm,
  setAlgorithm,
  speed,
  setSpeed,
  showProcess,
  setShowProcess,
  maxDepth,
  setMaxDepth,
  maxIterations,
  setMaxIterations,
  onRandomize,
  onStart,
  onStop,
  isSearching,
  isAnimating,
  stats,
  tileTheme,
  onToggleTileTheme,
}) => {
  // 计算是否处于忙碌状态（搜索中或动画中）
  const isBusy = isSearching || isAnimating;

  return (
    <div className="bg-slate-800 p-6 rounded-xl border border-slate-700 space-y-6">
      
      {/* 算法选择和额外配置区域 - 固定在一行 */}
      <div className="flex gap-3">
        {/* 左半部分：算法选择下拉菜单 - 固定占左半行 */}
        <div className="flex-1 space-y-2">
          <label className="text-xs font-bold text-slate-400 uppercase tracking-wider flex items-center gap-2 h-[18px]">
              <Settings size={14} /> Algorithm
          </label>
          <select
            disabled={isBusy}
            value={algorithm}
            onChange={(e) => setAlgorithm(e.target.value as AlgorithmType)}
            className="w-full h-[42px] bg-slate-900 border border-slate-600 rounded-lg px-3 py-2 text-slate-200 focus:outline-none focus:ring-2 focus:ring-blue-500 disabled:opacity-50"
          >
            {Object.values(AlgorithmType).map((algo) => (
              <option key={algo} value={algo}>{algo}</option>
            ))}
          </select>
        </div>

        {/* 右半部分：算法特定配置 - 固定占右半行 */}
        <div className="flex-1 space-y-2">
          {/* DFS 最大深度配置 */}
          {algorithm === AlgorithmType.DFS && (
            <>
              <label className="text-xs font-bold text-slate-400 uppercase tracking-wider block h-[18px] leading-[18px]">Max Depth</label>
              <input
                type="number"
                disabled={isBusy}
                value={maxDepth}
                onChange={(e) => setMaxDepth(parseInt(e.target.value))}
                className="w-full h-[42px] bg-slate-900 border border-slate-600 rounded-lg px-3 py-2 text-slate-200 focus:outline-none focus:ring-2 focus:ring-blue-500 disabled:opacity-50"
              />
            </>
          )}
          
          {/* Random 最大迭代次数配置 */}
          {algorithm === AlgorithmType.RANDOM && (
            <>
              <label className="text-xs font-bold text-slate-400 uppercase tracking-wider block h-[18px] leading-[18px]">Max Iterations</label>
              <input
                type="number"
                disabled={isBusy}
                value={maxIterations}
                onChange={(e) => setMaxIterations(parseInt(e.target.value))}
                className="w-full h-[42px] bg-slate-900 border border-slate-600 rounded-lg px-3 py-2 text-slate-200 focus:outline-none focus:ring-2 focus:ring-blue-500 disabled:opacity-50"
              />
            </>
          )}
          
          {/* 当没有额外配置时，右侧保持空白但占位 */}
          {algorithm !== AlgorithmType.DFS && algorithm !== AlgorithmType.RANDOM && (
            <div className="h-[18px]"></div>
          )}
        </div>
      </div>

      {/* 动画设置区域 */}
      <div className="space-y-4 pt-4 border-t border-slate-700">
        {/* 动画速度滑块 */}
        <div className="space-y-2">
            <div className="flex justify-between">
                <label className="text-xs font-bold text-slate-400 uppercase tracking-wider">Animation Speed</label>
                <span className="text-xs text-slate-400">{speed}ms</span>
            </div>
            <input
            type="range"
            min="10"
            max="1000"
            step="10"
            value={speed}
            onChange={(e) => setSpeed(parseInt(e.target.value))}
            className="w-full h-2 bg-slate-600 rounded-lg appearance-none cursor-pointer accent-blue-500"
            />
        </div>

        {/* 显示完整搜索过程开关 */}
        <div className="flex items-center justify-between gap-3">
             <label className="relative inline-flex items-center cursor-pointer">
                <input 
                    type="checkbox" 
                    checked={showProcess} 
                    onChange={(e) => setShowProcess(e.target.checked)} 
                    disabled={isBusy}
                    className="sr-only peer"
                />
                <div className="w-11 h-6 bg-slate-700 peer-focus:outline-none peer-focus:ring-2 peer-focus:ring-blue-500 rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:border-gray-300 after:border after:rounded-full after:h-5 after:w-5 after:transition-all peer-checked:bg-blue-600"></div>
                <span className="ml-3 text-sm font-medium text-slate-300">Show Full Search Process</span>
            </label>

            <button
              type="button"
              onClick={onToggleTileTheme}
              disabled={isBusy}
              className="inline-flex items-center gap-2 px-3 py-2 bg-slate-700 hover:bg-slate-600 text-slate-200 rounded-lg font-semibold text-sm transition-colors disabled:opacity-50 disabled:cursor-not-allowed"
              title="切换棋盘数字皮肤"
            >
              <Languages size={16} />
              {/* {tileTheme === TileTheme.CHINESE ? '中文数字' : '阿拉伯数字'} */}
              {tileTheme === TileTheme.CHINESE ? '' : ''}
            </button>
        </div>
      </div>

      {/* 操作按钮区域 */}
      <div className="pt-4 border-t border-slate-700 grid grid-cols-2 gap-3">
        {/* 随机化按钮 */}
        <button
          onClick={onRandomize}
          disabled={isBusy}
          className="flex items-center justify-center gap-2 px-4 py-3 bg-slate-700 hover:bg-slate-600 text-slate-200 rounded-lg font-semibold transition-colors disabled:opacity-50 disabled:cursor-not-allowed"
        >
          <Shuffle size={18} /> Randomize
        </button>

        {/* 根据状态显示开始或停止按钮 */}
        {isBusy ? (
            // 停止按钮（忙碌时显示）
            <button
                onClick={onStop}
                className="flex items-center justify-center gap-2 px-4 py-3 bg-red-600 hover:bg-red-500 text-white rounded-lg font-semibold transition-colors animate-pulse"
            >
                <Square size={18} fill="currentColor" /> Stop
            </button>
        ) : (
            // 开始求解按钮（空闲时显示）
            <button
                onClick={onStart}
                className="flex items-center justify-center gap-2 px-4 py-3 bg-blue-600 hover:bg-blue-500 text-white rounded-lg font-semibold transition-colors"
            >
                <Play size={18} fill="currentColor" /> Start Solving
            </button>
        )}
      </div>

      {/* 结果显示区域 - 固定高度预留空间 */}
      <div className="pt-4 border-t border-slate-700">
        <div className="h-[100px] flex items-center justify-center">
          {/* 成功结果展示 - 仅在搜索完成且成功时显示 */}
          {!isSearching && stats.success && (
          <div className="bg-emerald-900/30 p-4 rounded-lg border border-emerald-700/50 flex flex-col items-start w-full">
            <div className="flex items-center gap-2 mb-2 text-emerald-400">
              <CheckCircle size={18} />
              <span className="text-xs font-semibold uppercase tracking-wider opacity-80">Result</span>
            </div>
            <div className="flex items-baseline gap-2">
              <span className="text-xl font-bold text-emerald-200">Success</span>
              <span className="text-sm text-emerald-400/70">Length: {stats.pathLength}</span>
            </div>
          </div>
        )}

        {/* 失败结果展示 - 仅在搜索完成、未成功且至少探索了一些节点时显示 */}
        {!isSearching && !stats.success && stats.nodesExplored > 0 && (
          <div className="bg-red-900/30 p-4 rounded-lg border border-red-700/50 flex flex-col items-start w-full">
            <div className="flex items-center gap-2 mb-2 text-red-400">
              <XCircle size={18} />
              <span className="text-xs font-semibold uppercase tracking-wider opacity-80">Result</span>
            </div>
            <span className="text-xl font-bold text-red-200">Failed / Stopped</span>
          </div>
        )}
        </div>
      </div>
    </div>
  );
};

export default Controls;