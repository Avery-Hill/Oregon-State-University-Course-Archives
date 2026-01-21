'''
    This file contains the template for Assignment3. For testing it, I will place it
    in a different directory, call the function <min_steps_flood_escape>, and check its output.
    So, you can add/remove  whatever you want to/from this file. But, don't change the name
    of the file or the name/signature of the following function.

    I will use <python3> to run this code.
'''

def min_steps_flood_escape(grid: list[str]) -> int:
    '''
    Compute the minimum number of steps to reach the shelter, in a board specified by
    a list of strings; grid[i] is the ith row of the grid.  grid[i][j] is the character
    at the intersection of row i and column j.

    @param grid: list[str] - representing the board.
    Output: minium number of steps to reach from the initial point top-left to the
    shelter bottom-right.
    '''

    from collections import deque

    n = len(grid)
    G = [list(row) for row in grid]

    dirs = [(1,0),(-1,0),(0,1),(0,-1)]

    start = None
    shelter = None
    water_sources = []

    for i in range(n):
        for j in range(n):
            if G[i][j] == 'I':
                start = (i, j)
            elif G[i][j] == 'S':
                shelter = (i, j)
            elif G[i][j] == 'W':
                water_sources.append((i, j))

    if start is None or shelter is None:
        return -1

    INF = 10**15
    flood_time = [[INF]*n for _ in range(n)]
    q = deque()

    for (r, c) in water_sources:
        flood_time[r][c] = 0
        q.append((r, c))

    while q:
        r, c = q.popleft()
        t = flood_time[r][c]
        for dr, dc in dirs:
            nr, nc = r + dr, c + dc
            if 0 <= nr < n and 0 <= nc < n:
                if G[nr][nc] not in ('R', 'S'):
                    if flood_time[nr][nc] == INF:
                        flood_time[nr][nc] = t + 1
                        q.append((nr, nc))

    dist = [[INF]*n for _ in range(n)]
    sr, sc = start
    dist[sr][sc] = 0

    dq = deque()
    dq.append((sr, sc))

    while dq:
        r, c = dq.popleft()
        t = dist[r][c]

        if (r, c) == shelter:
            return t

        for dr, dc in dirs:
            nr, nc = r + dr, c + dc
            if 0 <= nr < n and 0 <= nc < n:
                if G[nr][nc] != 'W':
                    arrival = t + 1

                    if arrival < flood_time[nr][nc]:
                        if dist[nr][nc] > arrival:
                            dist[nr][nc] = arrival
                            dq.append((nr, nc))

    return -1
