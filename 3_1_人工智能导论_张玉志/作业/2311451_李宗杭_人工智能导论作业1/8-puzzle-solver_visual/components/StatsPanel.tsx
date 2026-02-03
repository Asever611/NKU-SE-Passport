/**
 * StatsPanel.tsx - 统计面板组件
 * 
 * 显示搜索算法的实时性能统计信息
 * 包括：
 * 1. 时间消耗（毫秒）
 * 2. 内存使用量（MB）
 * 3. 探索的节点数
 * 4. 生成的节点数
 * 5. 搜索结果（成功/失败）和解决方案长度
 */

import React from 'react';
import { SearchStats } from '../types';
import { Clock, Cpu, Activity, Move } from 'lucide-react';

/**
 * 统计面板组件的属性接口
 */
interface StatsPanelProps {
  stats: SearchStats;   // 当前的搜索统计数据
  searching: boolean;   // 是否正在搜索（用于控制结果显示）
}

const StatsPanel: React.FC<StatsPanelProps> = ({ stats, searching }) => {
  /**
   * 统计项组件
   * 用于显示单个统计指标
   * 
   * @param icon - Lucide 图标组件
   * @param label - 指标标签
   * @param value - 指标值
   * @param color - 颜色类（Tailwind CSS）
   */
  const StatItem = ({ icon: Icon, label, value, color }: any) => (
    <div className="bg-slate-800 p-4 rounded-lg border border-slate-700 flex flex-col items-start">
      <div className={`flex items-center gap-2 mb-2 ${color}`}>
        <Icon size={18} />
        <span className="text-xs font-semibold uppercase tracking-wider opacity-80">{label}</span>
      </div>
      <span className="text-2xl font-mono font-bold text-slate-100">{value}</span>
    </div>
  );

  return (
    <div className="grid grid-cols-2 gap-3 w-full">
      {/* 时间统计 */}
      <StatItem 
        icon={Clock} 
        label="Time (ms)" 
        value={stats.timeElapsed.toFixed(0)} 
        color="text-blue-400" 
      />
      
      {/* 内存使用统计 */}
      <StatItem 
        icon={Cpu} 
        label="Memory (MB)" 
        value={stats.memoryUsed.toFixed(2)} 
        color="text-purple-400" 
      />
      
      {/* 探索节点数统计 */}
      <StatItem 
        icon={Activity} 
        label="Nodes Explored" 
        value={stats.nodesExplored.toLocaleString()} 
        color="text-amber-400" 
      />
      
      {/* 生成节点数统计 */}
      <StatItem 
        icon={Move} 
        label="Nodes Generated" 
        value={stats.nodesGenerated.toLocaleString()} 
        color="text-indigo-400" 
      />
    </div>
  );
};

export default StatsPanel;