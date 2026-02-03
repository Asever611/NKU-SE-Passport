/**
 * priorityQueue.ts - 优先队列数据结构
 * 
 * 实现了一个简单的基于数组的优先队列，用于 A* 算法。
 * 元素按优先级（代价）从小到大排序。
 * 
 * 特点：
 * - 支持按优先级插入元素
 * - 使用计数器实现 tie-breaking（同优先级时 FIFO）
 * - 简单但对小规模问题足够高效
 * 
 * 注意：对于大规模问题，可以考虑使用二叉堆实现以提高性能
 */

/**
 * 优先队列类
 * 
 * @template T - 队列中元素的类型
 */
export class PriorityQueue<T> {
  // 内部存储的元素数组，每个元素包含实际数据、优先级和 tie-breaker
  private items: { element: T; priority: number; tieBreaker: number }[];
  
  // 计数器，用于 tie-breaking（确保同优先级时先进先出）
  private counter: number;

  /**
   * 构造函数 - 初始化空队列
   */
  constructor() {
    this.items = [];
    this.counter = 0;
  }

  /**
   * 将元素加入队列
   * 
   * 元素会根据优先级插入到正确的位置，保持队列有序。
   * 使用计数器作为 tie-breaker 确保同优先级时 FIFO 行为。
   * 
   * @param element - 要加入的元素
   * @param priority - 元素的优先级（越小越优先）
   */
  enqueue(element: T, priority: number) {
    // 递增计数器作为 tie-breaker
    this.counter++;
    const queueElement = { element, priority, tieBreaker: this.counter };
    
    let added = false;
    
    // 找到第一个优先级大于当前元素的位置并插入
    for (let i = 0; i < this.items.length; i++) {
      if (queueElement.priority < this.items[i].priority) {
        this.items.splice(i, 0, queueElement);
        added = true;
        break;
      }
    }
    
    // 如果没有找到合适位置（当前元素优先级最大），添加到末尾
    if (!added) {
      this.items.push(queueElement);
    }
  }

  /**
   * 移除并返回优先级最高的元素
   * 
   * @returns 优先级最高的元素，如果队列为空则返回 undefined
   */
  dequeue(): T | undefined {
    return this.items.shift()?.element;
  }

  /**
   * 检查队列是否为空
   * 
   * @returns 队列是否为空
   */
  isEmpty(): boolean {
    return this.items.length === 0;
  }

  /**
   * 获取队列中元素的数量
   * 
   * @returns 队列大小
   */
  size(): number {
    return this.items.length;
  }
}