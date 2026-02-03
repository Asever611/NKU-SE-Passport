"""
人工智能导论课 - 8数码问题求解器
实现了BFS、DFS、A*启发式搜索和随机搜索算法
支持扩展到N数码问题
"""

import time  # 用于计算算法耗时
import random  # 用于随机搜索和生成随机谜题
import tracemalloc  # 用于内存使用跟踪
from collections import deque  # 用于BFS的队列（高效弹出头部元素）
from heapq import heappush, heappop  # 用于A*的优先队列
import math  # 用于欧几里得距离计算


class PuzzleState:
    """
    数码问题的状态类
    支持N数码问题的通用实现
    """

    def __init__(self, board, parent=None, move="", depth=0, size=3):
        """
        初始化状态
        :param board: 二维列表表示的棋盘，0表示空格
        :param parent: 父状态
        :param move: 从父状态到当前状态的移动
        :param depth: 当前状态的深度（初始状态为0，每移动一步+1）
        :param size: 棋盘大小
        """
        self.board = board
        self.parent = parent
        self.move = move
        self.depth = depth
        self.size = size
        self.zero_pos = self.find_zero()
        self._tuple = self.to_tuple()

    def find_zero(self):
        """找到空格（0）的位置"""
        for i in range(self.size):
            for j in range(self.size):
                if self.board[i][j] == 0:
                    return (i, j)
        return None

    def get_neighbors(self):
        """
        获取当前状态的所有邻接状态
        返回可以移动到的所有新状态
        """
        neighbors = []
        row, col = self.zero_pos

        # 定义四个方向
        directions = [
            (-1, 0, "上"),  # 空格向上移（数字向下移）
            (1, 0, "下"),
            (0, -1, "左"),
            (0, 1, "右"),
        ]

        for dr, dc, move_name in directions:
            new_row, new_col = row + dr, col + dc

            # 检查边界
            if 0 <= new_row < self.size and 0 <= new_col < self.size:
                # 创建新状态
                new_board = [row[:] for row in self.board]
                # 交换空格和目标位置
                new_board[row][col], new_board[new_row][new_col] = \
                    new_board[new_row][new_col], new_board[row][col]

                new_state = PuzzleState(
                    new_board, 
                    parent=self, 
                    move=move_name, 
                    depth=self.depth + 1,
                    size=self.size
                )
                neighbors.append(new_state)

        return neighbors

    def is_goal(self, goal_state):
        """检查是否达到目标状态"""
        return self.board == goal_state

    def to_tuple(self):
        """将状态转换为元组，用于哈希"""
        return tuple(tuple(row) for row in self.board)

    def __hash__(self):
        return hash(self._tuple)

    def __eq__(self, other):
        if not isinstance(other, PuzzleState):
            return False
        return self.board == other.board

    def __str__(self):
        """美化输出棋盘"""
        result = []
        for row in self.board:
            result.append(" ".join(str(x) if x != 0 else "□" for x in row))
        return "\n".join(result)

    def get_path(self):
        """获取从初始状态到当前状态的路径"""
        path = []
        current = self
        while current.parent is not None:
            path.append((current.move, current.board))
            current = current.parent
        path.reverse()
        return path


class HeuristicFunctions:
    """启发式函数集合"""
    
    @staticmethod
    def manhattan_distance(state:PuzzleState, goal_positions, goal_state=None):
        """
        曼哈顿距离启发式函数
        计算每个数字到其目标位置的曼哈顿距离之和
        :param state: 当前状态
        :param goal_positions: 目标位置字典，数字 -> (行, 列)
        :param goal_state: 目标状态
        """
        distance = 0
        size = state.size
        
        # 计算曼哈顿距离
        for i in range(size):
            for j in range(size):
                if state.board[i][j] != 0:
                    value = state.board[i][j]
                    goal_i, goal_j = goal_positions[value]
                    distance += abs(i - goal_i) + abs(j - goal_j)
        
        return distance
    
    @staticmethod
    def euclidean_distance(state:PuzzleState, goal_positions, goal_state=None):
        """
        欧几里得距离启发式函数
        计算每个数字到其目标位置的欧几里得距离之和
        :param state: 当前状态
        :param goal_positions: 目标位置字典，数字 -> (行, 列)
        :param goal_state: 目标状态
        """
        distance = 0
        size = state.size
        
        # 计算欧几里得距离
        for i in range(size):
            for j in range(size):
                if state.board[i][j] != 0:
                    value = state.board[i][j]
                    goal_i, goal_j = goal_positions[value]
                    distance += math.sqrt((i - goal_i)**2 + (j - goal_j)**2)
        
        return distance
    
    @staticmethod
    def misplaced_tiles(state:PuzzleState, goal_positions, goal_state=None):
        """
        错位数启发式函数
        计算不在正确位置的数字个数
        :param state: 当前状态
        :param goal_positions: 目标位置字典，数字 -> (行, 列)
        :param goal_state: 目标状态
        """
        count = 0
        size = state.size
        for i in range(size):
            for j in range(size):
                if state.board[i][j] != 0 and state.board[i][j] != goal_state[i][j]:
                    count += 1
        return count


class SearchStatistics:
    """搜索统计信息类"""
    
    def __init__(self):
        self.nodes_explored = 0  # 探索的节点数
        self.nodes_generated = 0  # 生成的节点数
        self.time_elapsed = 0  # 耗时
        self.memory_used = 0  # 内存使用
        self.path_length = 0  # 路径长度
        self.success = False  # 是否成功
        
    def __str__(self):
        return f"""
========== 搜索统计 ==========
探索节点数: {self.nodes_explored}
生成节点数: {self.nodes_generated}
路径长度: {self.path_length}
耗时: {self.time_elapsed * 1000:.4f} ms
内存使用: {self.memory_used:.4f} MB
搜索结果: {'成功' if self.success else '失败'}
=============================
"""


class PuzzleSolver:
    """数码问题求解器"""

    def __init__(self, initial_state, goal_state, size=3):
        """
        初始化求解器
        :param initial_state: 初始状态的棋盘
        :param goal_state: 目标状态的棋盘
        :param size: 棋盘大小
        """
        self.initial_state = PuzzleState(initial_state, size=size)
        self.goal_state = goal_state
        self.size = size
        self.stats = SearchStatistics()
        self.goal_positions = {}
        for i in range(size):
            for j in range(size):
                if goal_state[i][j] != 0:
                    self.goal_positions[goal_state[i][j]] = (i, j)
    def bfs(self, visualize=False):
        """
        宽度优先搜索（BFS）
        :param visualize: 是否动态显示搜索过程
        :return: (成功标志, 路径, 统计信息)
        """
        tracemalloc.start() # 启动内存跟踪
        start_time = time.perf_counter() # 记录开始时间

        queue = deque([self.initial_state])  # 队列：存储待探索状态

        # 初始化统计信息
        self.stats = SearchStatistics()
        self.stats.nodes_generated = 1  # 初始状态已生成

        while queue:  # 当队列不为空时继续搜索
            current_state = queue.popleft()  # 弹出队列头部状态
            self.stats.nodes_explored += 1

            if visualize and self.stats.nodes_explored % 100 == 0:
                print(f"\r探索节点数: {self.stats.nodes_explored}, 队列长度: {len(queue)}", end="")

            # 检查是否达到目标
            if current_state.is_goal(self.goal_state):
                # 记录成功信息
                self.stats.success = True
                self.stats.time_elapsed = time.perf_counter() - start_time
                self.stats.path_length = current_state.depth

                # 记录内存使用情况
                _, peak = tracemalloc.get_traced_memory()
                tracemalloc.stop()
                self.stats.memory_used = peak / 1024 / 1024  # 转换为MB

                # 获取路径
                path = current_state.get_path()
                return True, path, self.stats

            # 扩展邻接状态
            for neighbor in current_state.get_neighbors():
                if neighbor != current_state.parent if current_state.parent else True:  # 避免回到父状态
                    queue.append(neighbor)
                    self.stats.nodes_generated += 1

        # 未找到解
        self.stats.time_elapsed = time.perf_counter() - start_time
        _, peak = tracemalloc.get_traced_memory()
        tracemalloc.stop()
        self.stats.memory_used = peak / 1024 / 1024
        return False, [], self.stats

    def dfs(self, max_depth=50, visualize=False):
        """
        深度优先搜索（DFS）
        :param max_depth: 最大深度限制
        :param visualize: 是否动态显示搜索过程
        :return: (成功标志, 路径, 统计信息)
        """
        tracemalloc.start()
        start_time = time.perf_counter()

        stack = [self.initial_state] # bfs使用队列，dfs使用栈

        self.stats = SearchStatistics()
        self.stats.nodes_generated = 1

        while stack:
            current_state = stack.pop()
            self.stats.nodes_explored += 1

            if visualize and self.stats.nodes_explored % 100 == 0:
                print(f"\r探索节点数: {self.stats.nodes_explored}, 栈深度: {len(stack)}", end="")

            # 检查是否达到目标
            if current_state.is_goal(self.goal_state):
                self.stats.success = True
                self.stats.time_elapsed = time.perf_counter() - start_time
                self.stats.path_length = current_state.depth

                _, peak = tracemalloc.get_traced_memory()
                tracemalloc.stop()
                self.stats.memory_used = peak / 1024 / 1024

                path = current_state.get_path()
                return True, path, self.stats

            # 深度限制
            if current_state.depth >= max_depth:
                continue

            # 扩展邻接状态
            neighbors = current_state.get_neighbors()
            for neighbor in reversed(neighbors):
                if neighbor != current_state.parent if current_state.parent else True:  # 避免回到父状态
                    stack.append(neighbor)
                    self.stats.nodes_generated += 1

        # 未找到解
        self.stats.time_elapsed = time.perf_counter() - start_time
        _, peak = tracemalloc.get_traced_memory()
        tracemalloc.stop()
        self.stats.memory_used = peak / 1024 / 1024
        print("\n未找到解!")
        return False, [], self.stats

    def a_star(self, heuristic_func, visualize=False):
        """
        A*启发式搜索
        :param heuristic_func: 启发式函数
        :param visualize: 是否动态显示搜索过程
        :return: (成功标志, 路径, 统计信息)
        """
        tracemalloc.start()
        start_time = time.perf_counter()

        # 优先队列：(f值, 计数器, 状态)
        counter = 0  # 解决f值相同的状态排序问题（避免heapq比较状态对象）
        h = heuristic_func(self.initial_state, self.goal_positions, self.goal_state)
        pq = [(h, counter, self.initial_state)]
        open = {self.initial_state._tuple:(counter, self.initial_state.depth)}  # 状态 -> (计数器, g值)
        open_count = {self.initial_state._tuple:1}  # 状态 -> 出现次数
        closed = {}  # 状态 -> g值

        self.stats = SearchStatistics()
        self.stats.nodes_generated = 1

        while pq:
            f, count, current_state = heappop(pq)
            latest, _ = open[current_state._tuple]
            open_count[current_state._tuple] -= 1
            if open_count[current_state._tuple] == 0:
                del open_count[current_state._tuple]
                del open[current_state._tuple]
            if count < latest:
                continue  # 跳过过时的状态
            self.stats.nodes_explored += 1

            if visualize and self.stats.nodes_explored % 100 == 0:
                print(f"\r探索节点数: {self.stats.nodes_explored}, 队列长度: {len(pq)}, f值: {f:.2f}", end="")

            # 检查是否达到目标
            if current_state.is_goal(self.goal_state):
                self.stats.success = True
                self.stats.time_elapsed = time.perf_counter() - start_time
                self.stats.path_length = current_state.depth

                _, peak = tracemalloc.get_traced_memory()
                tracemalloc.stop()
                self.stats.memory_used = peak / 1024 / 1024

                path = current_state.get_path()
                return True, path, self.stats

            closed[current_state._tuple] = current_state.depth

            # 扩展邻接状态
            for neighbor in current_state.get_neighbors():
                state_tuple = neighbor._tuple
                g = neighbor.depth
                h = heuristic_func(neighbor, self.goal_positions, self.goal_state)
                f = g + h
                counter += 1

                if state_tuple == current_state.parent._tuple if current_state.parent else False:
                    continue  # 避免回到父状态

                # 不在open表也不在closed表，直接加入open表
                if state_tuple not in open and state_tuple not in closed:
                    heappush(pq, (f, counter, neighbor))
                    open[state_tuple] = (counter, g)
                    open_count[state_tuple] = 1
                    self.stats.nodes_generated += 1
                # 如果已在open中，检查是否找到更好的路径
                elif state_tuple in open:  
                    _, existing_g = open[state_tuple]
                    if g < existing_g:
                        heappush(pq, (f, counter, neighbor))
                        open[state_tuple] = (counter, g)
                        open_count[state_tuple] += 1
                        self.stats.nodes_generated += 1
                # 如果已在closed中，检查是否找到更好的路径
                elif state_tuple in closed:
                    existing_g = closed[state_tuple]
                    if g < existing_g:
                        del closed[state_tuple]
                        heappush(pq, (f, counter, neighbor))
                        open[state_tuple] = (counter, g)
                        if state_tuple in open_count:
                            open_count[state_tuple] += 1
                        else:
                            open_count[state_tuple] = 1
                        self.stats.nodes_generated += 1

        # 未找到解
        self.stats.time_elapsed = time.perf_counter() - start_time
        _, peak = tracemalloc.get_traced_memory()
        tracemalloc.stop()
        self.stats.memory_used = peak / 1024 / 1024
        return False, [], self.stats

    def random_search(self, max_iterations=100000, visualize=False):
        """
        随机决策搜索
        在每个状态随机选择一个邻接状态进行探索
        :param max_iterations: 最大迭代次数
        :param visualize: 是否动态显示搜索过程
        :return: (成功标志, 路径, 统计信息)
        """
        tracemalloc.start()
        start_time = time.perf_counter()

        current_state = self.initial_state

        self.stats = SearchStatistics()
        self.stats.nodes_generated = 1
        self.stats.nodes_explored = 0

        for iteration in range(max_iterations):
            self.stats.nodes_explored += 1

            if visualize and iteration % 1000 == 0:
                print(f"\r迭代次数: {iteration}, 当前深度: {current_state.depth}", end="")

            # 检查是否达到目标
            if current_state.is_goal(self.goal_state):
                self.stats.success = True
                self.stats.time_elapsed = time.perf_counter() - start_time
                self.stats.path_length = current_state.depth

                _, peak = tracemalloc.get_traced_memory()
                tracemalloc.stop()
                self.stats.memory_used = peak / 1024 / 1024

                path = current_state.get_path()
                return True, path, self.stats

            # 获取所有邻接状态
            neighbors = current_state.get_neighbors()

            if neighbors:
                # 随机选择一个
                current_state = random.choice(neighbors)
            else:
                # 无路可走
                break

            self.stats.nodes_generated += 1

        # 未找到解
        self.stats.time_elapsed = time.perf_counter() - start_time
        _, peak = tracemalloc.get_traced_memory()
        tracemalloc.stop()
        self.stats.memory_used = peak / 1024 / 1024
        return False, [], self.stats


def print_path(path, show_all=False):
    """
    打印搜索路径
    :param path: 路径列表
    :param show_all: 是否显示所有步骤
    """
    if not path:
        print("无路径")
        return
    
    print(f"\n========== 解决方案（共{len(path)}步）==========")
    
    if show_all:
        for i, (move, board) in enumerate(path, 1):
            print(f"\n步骤 {i}: {move}")
            state = PuzzleState(board, size=len(board))
            print(state)
    else:
        # 只显示前5步和后5步
        print("\n前5步:")
        for i, (move, board) in enumerate(path[:5], 1):
            print(f"\n步骤 {i}: {move}")
            state = PuzzleState(board, size=len(board))
            print(state)
        
        if len(path) > 10:
            print(f"\n... 省略 {len(path) - 10} 步 ...")
        
        print("\n后5步:")
        for i, (move, board) in enumerate(path[-5:], len(path) - 4):
            print(f"\n步骤 {i}: {move}")
            state = PuzzleState(board, size=len(board))
            print(state)


def is_solvable(board, size=3):
    """
    检查数码问题是否可解
    对于N数码问题：
    - 当N为奇数时，逆序数为偶数则可解
    - 当N为偶数时，需要考虑空格位置
    """
    # 将二维数组展平，去除0
    flat = []
    zero_row = 0
    for i in range(size):
        for j in range(size):
            if board[i][j] == 0:
                zero_row = i
            else:
                flat.append(board[i][j])
    
    # 计算逆序数
    inversions = 0
    for i in range(len(flat)):
        for j in range(i + 1, len(flat)):
            if flat[i] > flat[j]:
                inversions += 1
    
    # 判断可解性
    if size % 2 == 1:
        # 奇数规模：逆序数为偶数
        return inversions % 2 == 0
    else:
        # 偶数规模：逆序数+空格所在行（从底部1算起）为奇数
        row_from_bottom = size - zero_row
        return (inversions + row_from_bottom) % 2 == 1


def generate_random_puzzle(size=3, moves=100):
    """
    生成随机的可解数码问题
    :param size: 棋盘大小
    :param moves: 从目标状态随机移动的步数
    :return: 初始状态，目标状态
    """
    # 从目标状态开始
    goal = [[i * size + j for j in range(1, size + 1)] for i in range(size)]
    goal[-1][-1] = 0  # 最后一个位置为空格
    
    current = PuzzleState(goal, size=size)
    
    # 随机移动
    for _ in range(moves):
        neighbors = current.get_neighbors()
        current = random.choice(neighbors)
    
    return current.board, goal


def compare_algorithms(initial_state, goal_state, size=3):
    """
    比较不同算法的性能
    """
    print("\n初始状态:")
    print(PuzzleState(initial_state, size=size))
    print("\n目标状态:")
    print(PuzzleState(goal_state, size=size))

    # 检查可解性
    if not is_solvable(initial_state, size):
        print("\n该初始状态不可解!")
        return

    results = []
    repeat = 5

    for i in range(repeat):    
        # 1. BFS
        solver = PuzzleSolver(initial_state, goal_state, size)
        success, path, stats = solver.bfs()
        results.append(("BFS", stats))

        # 2. DFS
        solver = PuzzleSolver(initial_state, goal_state, size)
        success, path, stats = solver.dfs(max_depth=50)
        results.append(("DFS", stats))

        # 3. A* with Manhattan Distance
        solver = PuzzleSolver(initial_state, goal_state, size)
        success, path, stats = solver.a_star(HeuristicFunctions.manhattan_distance)
        results.append(("A* (Manhattan)", stats))

        # 4. A* with Euclidean Distance
        solver = PuzzleSolver(initial_state, goal_state, size)
        success, path, stats = solver.a_star(HeuristicFunctions.euclidean_distance)
        results.append(("A* (Euclidean)", stats))

        # 5. A* with Misplaced Tiles
        solver = PuzzleSolver(initial_state, goal_state, size)
        success, path, stats = solver.a_star(HeuristicFunctions.misplaced_tiles)
        results.append(("A* (Misplaced)", stats))

        # 6. Random Search
        solver = PuzzleSolver(initial_state, goal_state, size)
        success, path, stats = solver.random_search(max_iterations=100000)
        results.append(("Random", stats))

    # 输出对比总结
    print("\n" + "="*60)
    print(f"{'算法':<14} {'成功率':>9} {'节点数':>9} {'时间(ms)':>10} {'内存(MB)':>10}")
    print("-"*60)

    avg_results = {}
    win_rates = {}
    best_length = 0

    for algo_name, stat in results:
        if algo_name not in avg_results:
            avg_results[algo_name] = SearchStatistics()

        avg_results[algo_name].nodes_explored += stat.nodes_explored
        avg_results[algo_name].nodes_generated += stat.nodes_generated
        avg_results[algo_name].time_elapsed += stat.time_elapsed
        avg_results[algo_name].memory_used += stat.memory_used
        if stat.success:
            if algo_name not in win_rates:
                win_rates[algo_name] = 0
            win_rates[algo_name] += 1

            if algo_name == "A* (Manhattan)":
                best_length = stat.path_length

    for algo_name, stat in avg_results.items():
        success_rate = (win_rates.get(algo_name, 0) / repeat) * 100
        stat.nodes_explored /= repeat
        stat.time_elapsed = stat.time_elapsed * 1000 / repeat
        stat.memory_used /= repeat
        print(f"{algo_name:<16} {success_rate:>12.2f}%"
              f"{stat.nodes_explored:>12} {stat.time_elapsed:>12.4f} {stat.memory_used:>12.4f}")

    print("="*60)
    print("最佳路径：", best_length)
    return avg_results

def find_longest_search_path():
    """
    寻找8数码问题的最长搜索链
    使用已知的最难案例
    """
    print("\n" + "="*60)
    print("寻找8数码问题的最长搜索路径")
    print("="*60)
    
    # 已知的8数码最难案例之一（需要31步）
    hardest_initial = [
        [8, 6, 7],
        [2, 5, 4],
        [3, 0, 1]
    ]
    
    goal_state = [
        [1, 2, 3],
        [4, 5, 6],
        [7, 8, 0]
    ]
    
    print("\n8数码问题的已知最难案例之一")
    print("\n初始状态:")
    print(PuzzleState(hardest_initial, size=3))
    
    solver = PuzzleSolver(hardest_initial, goal_state, size=3)
    success, path, stats = solver.a_star(
        HeuristicFunctions.manhattan_distance,
        "曼哈顿距离"
    )
    
    print(stats)
    
    if success:
        print_path(path, show_all=False)


def test_15_puzzle():
    """
    测试15数码问题的可扩展性
    """
    print("\n" + "="*60)
    print("15数码问题测试")
    print("="*60)

    goal_state = [
        [1, 2, 3, 4],
        [5, 6, 7, 8],
        [9, 10, 11, 12],
        [13, 14, 15, 0]
    ]

    # 简单的15数码案例
    initial_state = [
        [1, 2, 3, 4],
        [5, 6, 7, 8],
        [9, 10, 11, 12],
        [13, 14, 0, 15]
    ]

    # 随机的15数码案例
    random_initial, _ = generate_random_puzzle(size=4, moves=50)

    print("\n目标状态:")
    print(PuzzleState(goal_state, size=4))

    print("\n简单的15数码案例:")
    print("\n初始状态:")
    print(PuzzleState(initial_state, size=4))
    # 使用A*搜索
    solver = PuzzleSolver(initial_state, goal_state, size=4)
    success, path, stats = solver.a_star(
        HeuristicFunctions.manhattan_distance,
        "曼哈顿距离"
    )
    print(stats)

    print("随机生成的15数码案例:")
    print("\n初始状态:")
    print(PuzzleState(random_initial, size=4))
    # 使用A*搜索
    solver = PuzzleSolver(random_initial, goal_state, size=4)
    success, path, stats = solver.a_star(
        HeuristicFunctions.manhattan_distance,
        "曼哈顿距离"
    )
    print(stats)


def find_longest_search_path():
    """
    寻找8数码问题的最长搜索链
    """
    print("\n" + "=" * 60)
    print("寻找8数码问题的最长搜索路径")
    print("=" * 60)
    start_time = time.perf_counter()

    # 扁平化处理以提高速度 (使用元组作为哈希键)
    # 将二维列表转换为一维元组: ((1,2,3),(4,5,6),(7,8,0)) -> (1,2,3,4,5,6,7,8,0)
    goal_state = [[1, 2, 3], [4, 5, 6], [7, 8, 0]]
    goal_tuple = tuple(num for row in goal_state for num in row)

    # 预计算邻接关系 (索引 0-8 在 3x3 网格中的移动关系)
    # 0 1 2
    # 3 4 5
    # 6 7 8
    neighbors_map = {
        0: (1, 3),
        1: (0, 2, 4),
        2: (1, 5),
        3: (0, 4, 6),
        4: (1, 3, 5, 7),
        5: (2, 4, 8),
        6: (3, 7),
        7: (4, 6, 8),
        8: (5, 7),
    }

    # BFS 初始化
    queue = deque([(goal_tuple, 0)])  # (状态, 距离)
    visited = {goal_tuple: 0}  # 记录已访问状态及其距离

    max_distance = 0
    hardest_states = []

    # 开始遍历
    while queue:
        current_state, dist = queue.popleft()

        # 更新最大距离记录
        if dist > max_distance:
            max_distance = dist
            hardest_states = [current_state]
        elif dist == max_distance:
            hardest_states.append(current_state)

        # 找到空格(0)的位置
        zero_idx = current_state.index(0)

        # 遍历邻居
        for neighbor_idx in neighbors_map[zero_idx]:
            # 创建新状态
            new_list = list(current_state)
            new_list[zero_idx], new_list[neighbor_idx] = (
                new_list[neighbor_idx],
                new_list[zero_idx],
            )
            new_state = tuple(new_list)

            if new_state not in visited:
                visited[new_state] = dist + 1
                queue.append((new_state, dist + 1))

    duration = time.perf_counter() - start_time
    print(f"遍历完成!")
    print(f"耗时: {duration:.4f} 秒")
    print(f"总计可解状态数: {len(visited)}")
    print(f"最长路径长度: {max_distance}")
    print(f"最难解状态数量: {len(hardest_states)}")

    # 还原其中一个最难的状态为二维列表
    hardest_flat = hardest_states[0]
    hardest_state = [
        list(hardest_flat[0:3]),
        list(hardest_flat[3:6]),
        list(hardest_flat[6:9]),
    ]

    print("\n" + "=" * 60)
    print(f"验证：使用 A* (曼哈顿距离) 求解这个最难状态")
    print("=" * 60)

    print("最难初始状态:")
    for row in hardest_state:
        print(row)

    # 使用 A* 求解验证
    solver = PuzzleSolver(hardest_state, goal_state, size=3)

    # 使用曼哈顿距离求解
    success, path, stats = solver.a_star(
        HeuristicFunctions.manhattan_distance,
        visualize=False,
    )

    print(stats)

    if success:
        print(f"验证结果: 算法求出的路径长度为 {len(path)}")
        if len(path) == max_distance:
            print("验证成功")
        else:
            print("验证差异")

def main():
    """主函数 - 演示所有功能"""

    # 测试1：比较不同算法
    initial_states = [
        [[1, 2, 3], [4, 5, 6], [7, 8, 0]],
        [[1, 2, 3], [4, 5, 6], [0, 7, 8]],
        [[0, 1, 3], [4, 2, 6], [7, 5, 8]],
        [[4, 1, 2], [5, 0, 3], [7, 8, 6]],
        [[1, 2, 3], [7, 0, 4], [5, 8, 6]],
        [[4, 5, 1], [2, 0, 3], [7, 8, 6]],
        [[2, 7, 3], [1, 0, 4], [8, 6, 5]],
        [[5, 1, 6], [4, 3, 8], [0, 2, 7]],
        [[5, 1, 6], [4, 0, 8], [2, 3, 7]],
        [[5, 6, 0], [4, 1, 8], [2, 3, 7]],
        [[0, 1, 2], [3, 5, 4], [7, 6, 8]],
        [[0, 1, 2], [3, 4, 7], [5, 6, 8]],
        [[0, 1, 2], [3, 4, 7], [6, 8, 5]],
    ]

    goal_state = [
        [1, 2, 3],
        [4, 5, 6],
        [7, 8, 0]
    ]

    all_avg_results = []
    for i, state in enumerate(initial_states):
        print(f"\n--- 案例 {i+1} ---")
        avg_results = compare_algorithms(state, goal_state, size=3)
        all_avg_results.append(avg_results)

    print("\n所有案例的平均结果汇总:")
    print("1.节点数:")
    print(f"{'案例':>2}", end="")
    for algo_name in all_avg_results[0].keys():
        print(f"{algo_name:>16}", end="")
    for i, avg_results in enumerate(all_avg_results):
        print(f"\n{i+1:>4}", end="")
        for algo_name, stat in avg_results.items():
            print(f"{stat.nodes_explored:>16}", end="")

    print("\n\n2.时间(ms):")
    print(f"{'案例':>2}", end="")
    for algo_name in all_avg_results[0].keys():
        print(f"{algo_name:>16}", end="")
    for i, avg_results in enumerate(all_avg_results):
        print(f"\n{i+1:>4}", end="")
        for algo_name, stat in avg_results.items():
            print(f"{stat.time_elapsed:>16.4f}", end="")

    print("\n\n3.内存(MB):")
    print(f"{'案例':>2}", end="")
    for algo_name in all_avg_results[0].keys():
        print(f"{algo_name:>16}", end="")
    for i, avg_results in enumerate(all_avg_results):
        print(f"\n{i+1:>4}", end="")
        for algo_name, stat in avg_results.items():
            print(f"{stat.memory_used:>16.4f}", end="")

    # 测试2：寻找最长路径
    find_longest_search_path()

    # 测试3：15数码
    test_15_puzzle()

    print("\n\n" + "="*80)
    print("所有测试完成!")
    print("="*80)


if __name__ == "__main__":
    main()
