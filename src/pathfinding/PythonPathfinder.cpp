#include "pathfinding/PythonPathfinder.hpp"

#include "Util/Logger.hpp"

#include <string>
#include <vector>

#ifdef ENABLE_PYTHON_PATHFINDING
#include <Python.h>
#endif

namespace {
#ifdef ENABLE_PYTHON_PATHFINDING
PyObject* buildGridList(const std::vector<std::vector<int>>& grid) {
    PyObject* pyGrid = PyList_New(static_cast<Py_ssize_t>(grid.size()));
    if (pyGrid == nullptr) {
        return nullptr;
    }

    for (size_t row = 0; row < grid.size(); ++row) {
        const std::vector<int>& cells = grid[row];
        PyObject* pyRow = PyList_New(static_cast<Py_ssize_t>(cells.size()));
        if (pyRow == nullptr) {
            Py_DECREF(pyGrid);
            return nullptr;
        }

        for (size_t col = 0; col < cells.size(); ++col) {
            PyObject* pyCell = PyLong_FromLong(cells[col]);
            if (pyCell == nullptr) {
                Py_DECREF(pyRow);
                Py_DECREF(pyGrid);
                return nullptr;
            }
            PyList_SET_ITEM(pyRow, static_cast<Py_ssize_t>(col), pyCell);
        }

        PyList_SET_ITEM(pyGrid, static_cast<Py_ssize_t>(row), pyRow);
    }

    return pyGrid;
}

PyObject* buildPointList(const std::vector<std::pair<int, int>>& points) {
    PyObject* pyPoints = PyList_New(static_cast<Py_ssize_t>(points.size()));
    if (pyPoints == nullptr) {
        return nullptr;
    }

    for (size_t i = 0; i < points.size(); ++i) {
        PyObject* pyTuple = Py_BuildValue("(ii)", points[i].first, points[i].second);
        if (pyTuple == nullptr) {
            Py_DECREF(pyPoints);
            return nullptr;
        }
        PyList_SET_ITEM(pyPoints, static_cast<Py_ssize_t>(i), pyTuple);
    }

    return pyPoints;
}

bool appendPythonSearchPath(const std::string& path) {
    PyObject* sysPath = PySys_GetObject("path");
    if (sysPath == nullptr || !PyList_Check(sysPath)) {
        return false;
    }

    PyObject* pyPath = PyUnicode_FromString(path.c_str());
    if (pyPath == nullptr) {
        return false;
    }

    const int appendResult = PyList_Append(sysPath, pyPath);
    Py_DECREF(pyPath);
    return appendResult == 0;
}

std::vector<std::pair<int, int>> parsePath(PyObject* result) {
    std::vector<std::pair<int, int>> path;

    if (!PyList_Check(result)) {
        LOG_ERROR("Python 回傳型別錯誤，預期 list。");
        return path;
    }

    const Py_ssize_t size = PyList_Size(result);
    path.reserve(static_cast<size_t>(size));

    for (Py_ssize_t i = 0; i < size; ++i) {
        PyObject* item = PyList_GetItem(result, i); // borrowed ref
        if (item == nullptr || !PyTuple_Check(item) || PyTuple_Size(item) != 2) {
            continue;
        }

        PyObject* xObj = PyTuple_GetItem(item, 0); // borrowed ref
        PyObject* yObj = PyTuple_GetItem(item, 1); // borrowed ref
        const long x = PyLong_AsLong(xObj);
        const long y = PyLong_AsLong(yObj);
        if (PyErr_Occurred() != nullptr) {
            PyErr_Clear();
            continue;
        }
        path.emplace_back(static_cast<int>(x), static_cast<int>(y));
    }

    return path;
}
#endif
} // namespace

PythonPathfinder::PythonPathfinder() {
#ifdef ENABLE_PYTHON_PATHFINDING
    if (!Py_IsInitialized()) {
        Py_Initialize();
    }

    if (!Py_IsInitialized()) {
        LOG_ERROR("Python interpreter 初始化失敗。");
        initialized = false;
        return;
    }

    initialized = true;
    if (!appendPythonSearchPath(PYTHON_PATHFINDING_SCRIPT_DIR)) {
        LOG_WARN("無法加入 Python 搜尋路徑: {}", PYTHON_PATHFINDING_SCRIPT_DIR);
    }
#else
    initialized = false;
#endif
}

std::vector<std::pair<int, int>> PythonPathfinder::findPath(
    const std::vector<std::vector<int>>& grid,
    std::pair<int, int> start,
    std::pair<int, int> goal
) const {
    std::vector<std::pair<int, int>> path;

#ifdef ENABLE_PYTHON_PATHFINDING
    if (!initialized) {
        return path;
    }

    PyObject* module = PyImport_ImportModule("pathfinder");
    if (module == nullptr) {
        LOG_ERROR("載入 Python 模組 `pathfinder` 失敗。");
        PyErr_Print();
        return path;
    }

    PyObject* func = PyObject_GetAttrString(module, "a_star");
    if (func == nullptr || !PyCallable_Check(func)) {
        LOG_ERROR("找不到可呼叫函式 `a_star`。");
        Py_XDECREF(func);
        Py_DECREF(module);
        return path;
    }

    PyObject* pyGrid = buildGridList(grid);
    PyObject* pyStart = Py_BuildValue("(ii)", start.first, start.second);
    PyObject* pyGoal = Py_BuildValue("(ii)", goal.first, goal.second);
    if (pyGrid == nullptr || pyStart == nullptr || pyGoal == nullptr) {
        LOG_ERROR("建構 Python 參數失敗。");
        Py_XDECREF(pyGrid);
        Py_XDECREF(pyStart);
        Py_XDECREF(pyGoal);
        Py_DECREF(func);
        Py_DECREF(module);
        return path;
    }

    PyObject* result = PyObject_CallFunctionObjArgs(func, pyGrid, pyStart, pyGoal, nullptr);

    Py_DECREF(pyGrid);
    Py_DECREF(pyStart);
    Py_DECREF(pyGoal);
    Py_DECREF(func);
    Py_DECREF(module);

    if (result == nullptr) {
        LOG_ERROR("呼叫 Python `a_star` 失敗。");
        PyErr_Print();
        return path;
    }

    path = parsePath(result);
    Py_DECREF(result);
#else
    (void)grid;
    (void)start;
    (void)goal;
#endif

    return path;
}

std::vector<std::pair<int, int>> PythonPathfinder::findBestPath(
    const std::vector<std::vector<int>>& grid,
    const std::vector<std::pair<int, int>>& starts,
    const std::vector<std::pair<int, int>>& goals
) const {
    std::vector<std::pair<int, int>> path;

#ifdef ENABLE_PYTHON_PATHFINDING
    if (!initialized) {
        return path;
    }

    PyObject* module = PyImport_ImportModule("pathfinder");
    if (module == nullptr) {
        LOG_ERROR("載入 Python 模組 `pathfinder` 失敗。");
        PyErr_Print();
        return path;
    }

    PyObject* func = PyObject_GetAttrString(module, "find_best_path");
    if (func == nullptr || !PyCallable_Check(func)) {
        LOG_ERROR("找不到可呼叫函式 `find_best_path`。");
        Py_XDECREF(func);
        Py_DECREF(module);
        return path;
    }

    PyObject* pyGrid = buildGridList(grid);
    PyObject* pyStarts = buildPointList(starts);
    PyObject* pyGoals = buildPointList(goals);
    if (pyGrid == nullptr || pyStarts == nullptr || pyGoals == nullptr) {
        LOG_ERROR("建構 Python 參數失敗。");
        Py_XDECREF(pyGrid);
        Py_XDECREF(pyStarts);
        Py_XDECREF(pyGoals);
        Py_DECREF(func);
        Py_DECREF(module);
        return path;
    }

    PyObject* result = PyObject_CallFunctionObjArgs(func, pyGrid, pyStarts, pyGoals, nullptr);

    Py_DECREF(pyGrid);
    Py_DECREF(pyStarts);
    Py_DECREF(pyGoals);
    Py_DECREF(func);
    Py_DECREF(module);

    if (result == nullptr) {
        LOG_ERROR("呼叫 Python `find_best_path` 失敗。");
        PyErr_Print();
        return path;
    }

    path = parsePath(result);
    Py_DECREF(result);
#else
    (void)grid;
    (void)starts;
    (void)goals;
#endif

    return path;
}
