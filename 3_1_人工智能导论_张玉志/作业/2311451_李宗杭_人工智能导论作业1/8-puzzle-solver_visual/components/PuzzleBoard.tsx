/**
 * PuzzleBoard.tsx - 拼图棋盘组件
 * 
 * 负责渲染 8-Puzzle 的可视化界面
 * 功能：
 * 1. 显示 3x3 的拼图网格
 * 2. 使用绝对定位和 CSS transform 实现瓦片的平滑动画
 * 3. 通过颜色区分正确和错误位置的瓦片
 * 4. 支持响应式布局
 */

import React from 'react';
import { Board, TileTheme } from '../types';

/**
 * 拼图棋盘组件的属性接口
 */
interface PuzzleBoardProps {
  board: Board;          // 当前的棋盘状态（3x3 二维数组）
  isAnimating: boolean;  // 是否正在播放动画（用于调整过渡速度）
  tileTheme: TileTheme;  // 瓦片数字显示主题
}

// 中文数字映射表，索引与瓦片数字一致
const CHINESE_NUMERALS = ['0', '哈', '基', '米', '莫', '南', '北', '绿', '豆'];

const PuzzleBoard: React.FC<PuzzleBoardProps> = ({ board, isAnimating, tileTheme }) => {
  /**
   * 需要将棋盘扁平化，但同时跟踪值到坐标的映射
   * 数字 '0' 代表空格（不显示）
   */
  
  // 创建一个映射：数字值 -> 当前位置(row, col)
  const positions = new Map<number, { r: number; c: number }>();
  for (let r = 0; r < 3; r++) {
    for (let c = 0; c < 3; c++) {
      positions.set(board[r][c], { r, c });
    }
  }

  /**
   * 检查瓦片是否在正确的位置（用于颜色编码）
   * 目标状态：1 2 3 / 4 5 6 / 7 8 0
   * 
   * @param val - 瓦片上的数字
   * @param r - 当前行位置
   * @param c - 当前列位置
   * @returns 是否在正确位置
   */
  const isCorrectPosition = (val: number, r: number, c: number) => {
    if (val === 0) return true; // 空格不需要视觉判断
    
    // 计算目标位置
    const targetR = Math.floor((val - 1) / 3);
    const targetC = (val - 1) % 3;
    return r === targetR && c === targetC;
  };

  // 固定的瓦片列表（1-8），根据它们当前的网格位置进行绝对定位渲染
  const tiles = [1, 2, 3, 4, 5, 6, 7, 8];

  return (
    <div className="relative w-full max-w-md aspect-square bg-slate-800 rounded-lg p-2 shadow-2xl border-4 border-slate-700">
      <div className="relative w-full h-full bg-slate-900 rounded-md overflow-hidden">
        {/* 渲染网格背景线（美化效果）*/}
        <div className="absolute inset-0 grid grid-cols-3 grid-rows-3 gap-2 p-2">
            {[...Array(9)].map((_, i) => (
                <div key={i} className="bg-slate-800/50 rounded-md w-full h-full"></div>
            ))}
        </div>

        {/* 渲染瓦片 */}
        <div className="absolute inset-0 p-2">
            {tiles.map((val) => {
            const pos = positions.get(val);
            if (!pos) return null; // 理论上不应该发生

            // 检查当前瓦片是否在正确位置
            const correct = isCorrectPosition(val, pos.r, pos.c);
            
            // 计算百分比位置用于 transform
            // 3列网格，每个单元格占 33.33%
            const x = pos.c * 100;
            const y = pos.r * 100;

            return (
                <div
                key={val}
                className={`absolute top-0 left-0 w-1/3 h-1/3 p-1 transition-transform duration-200 ease-in-out z-10`}
                style={{
                    transform: `translate(${x}%, ${y}%)`,
                    // 根据是否在动画中动态调整过渡速度
                    transitionDuration: isAnimating ? '150ms' : '300ms'
                }}
                >
                <div
                    className={`
                    w-full h-full flex items-center justify-center text-3xl font-bold rounded-lg shadow-lg
                    ${correct 
                        ? 'bg-emerald-600 text-white shadow-emerald-900/50 border-b-4 border-emerald-800' 
                        : 'bg-amber-500 text-white shadow-amber-900/50 border-b-4 border-amber-700'
                    }
                    `}
                >
                    {tileTheme === TileTheme.CHINESE ? CHINESE_NUMERALS[val] : val}
                </div>
                </div>
            );
            })}
        </div>
      </div>
    </div>
  );
};

export default PuzzleBoard;