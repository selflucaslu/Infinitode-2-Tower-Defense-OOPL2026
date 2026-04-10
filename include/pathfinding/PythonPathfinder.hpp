#pragma once

#include <utility>
#include <vector>

class PythonPathfinder {
public:
    PythonPathfinder();

    std::vector<std::pair<int, int>> findPath(
        const std::vector<std::vector<int>>& grid,
        std::pair<int, int> start,
        std::pair<int, int> goal
    ) const;

    std::vector<std::pair<int, int>> findBestPath(
        const std::vector<std::vector<int>>& grid,
        const std::vector<std::pair<int, int>>& starts,
        const std::vector<std::pair<int, int>>& goals
    ) const;

private:
    bool initialized = false;
};
