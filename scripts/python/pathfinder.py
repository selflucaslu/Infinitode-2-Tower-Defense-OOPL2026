import heapq


def _heuristic(a, b):
    return abs(a[0] - b[0]) + abs(a[1] - b[1])


def _neighbors(x, y):
    return ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1))


def _in_bounds(grid, x, y):
    return 0 <= y < len(grid) and 0 <= x < len(grid[0])


def _passable(grid, x, y):
    return grid[y][x] == 0


def _reconstruct_path(came_from, current):
    result = [current]
    while current in came_from:
        current = came_from[current]
        result.append(current)
    result.reverse()
    return result


def a_star(grid, start, goal):
    """
    grid: 2D list，0 代表可通行，1 代表障礙。
    start/goal: (x, y) tuple
    回傳: [(x0, y0), (x1, y1), ...]；找不到則回傳空陣列。
    """
    if not grid or not grid[0]:
        return []
    if not _in_bounds(grid, *start) or not _in_bounds(grid, *goal):
        return []
    if not _passable(grid, *start) or not _passable(grid, *goal):
        return []

    open_heap = []
    heapq.heappush(open_heap, (_heuristic(start, goal), 0, start))
    came_from = {}
    g_score = {start: 0}

    while open_heap:
        _, current_cost, current = heapq.heappop(open_heap)
        if current == goal:
            return _reconstruct_path(came_from, current)

        if current_cost > g_score.get(current, float("inf")):
            continue

        for nx, ny in _neighbors(*current):
            if not _in_bounds(grid, nx, ny) or not _passable(grid, nx, ny):
                continue

            tentative = current_cost + 1
            if tentative < g_score.get((nx, ny), float("inf")):
                came_from[(nx, ny)] = current
                g_score[(nx, ny)] = tentative
                f_score = tentative + _heuristic((nx, ny), goal)
                heapq.heappush(open_heap, (f_score, tentative, (nx, ny)))

    return []


def find_best_path(grid, starts, goals):
    """
    grid: 2D list，0 可走、1 牆。
    starts/goals: [(x, y), ...]
    回傳所有組合中最短可行路徑，找不到回傳 []。
    """
    best = []
    for start in starts:
        for goal in goals:
            candidate = a_star(grid, tuple(start), tuple(goal))
            if not candidate:
                continue
            if not best or len(candidate) < len(best):
                best = candidate
    return best
