// sudoku.cpp
#include <iostream>
#include <cstdlib>   // rand, srand
#include <ctime>     // time (for seeding)

using namespace std;

static const int N = 9;
static const int UNASSIGNED = 0;

void printGrid(const int g[N][N]) {
    cout << "+-------+-------+-------+\n";
    for (int r = 0; r < N; ++r) {
        cout << "| ";
        for (int c = 0; c < N; ++c) {
            if (g[r][c] == 0) cout << ". ";
            else cout << g[r][c] << ' ';
            if ((c + 1) % 3 == 0) cout << "| ";
        }
        cout << '\n';
        if ((r + 1) % 3 == 0) cout << "+-------+-------+-------+\n";
    }
}

bool usedInRow(const int g[N][N], int row, int num) {
    for (int c = 0; c < N; ++c) if (g[row][c] == num) return true;
    return false;
}

bool usedInCol(const int g[N][N], int col, int num) {
    for (int r = 0; r < N; ++r) if (g[r][col] == num) return true;
    return false;
}

bool usedInBox(const int g[N][N], int boxStartRow, int boxStartCol, int num) {
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            if (g[boxStartRow + r][boxStartCol + c] == num) return true;
    return false;
}

bool isSafe(const int g[N][N], int row, int col, int num) {
    return !usedInRow(g, row, num) &&
           !usedInCol(g, col, num) &&
           !usedInBox(g, row - row % 3, col - col % 3, num) &&
           g[row][col] == UNASSIGNED;
}

bool findUnassigned(const int g[N][N], int &row, int &col) {
    for (row = 0; row < N; ++row)
        for (col = 0; col < N; ++col)
            if (g[row][col] == UNASSIGNED) return true;
    return false;
}

// Fisher-Yates shuffle for digits 1..9 without <algorithm> or <vector>
void shuffledDigits(int out[9]) {
    for (int i = 0; i < 9; ++i) out[i] = i + 1;
    for (int i = 8; i > 0; --i) {
        int j = rand() % (i + 1);
        int tmp = out[i]; out[i] = out[j]; out[j] = tmp;
    }
}

// Standard backtracking solver (single solution search)
bool solveSudoku(int g[N][N]) {
    int row, col;
    if (!findUnassigned(g, row, col)) return true;

    int order[9]; shuffledDigits(order);
    for (int i = 0; i < 9; ++i) {
        int num = order[i];
        if (isSafe(g, row, col, num)) {
            g[row][col] = num;
            if (solveSudoku(g)) return true;
            g[row][col] = UNASSIGNED;
        }
    }
    return false;
}

// Solution counter with early cutoff at 'limit'
void countSolutionsDFS(int g[N][N], int &count, int limit) {
    if (count >= limit) return; // early stop
    int row, col;
    if (!findUnassigned(g, row, col)) { ++count; return; }
    // Try digits in a deterministic but slightly shuffled order for variety
    int order[9]; shuffledDigits(order);
    for (int i = 0; i < 9; ++i) {
        int num = order[i];
        if (isSafe(g, row, col, num)) {
            g[row][col] = num;
            countSolutionsDFS(g, count, limit);
            if (count >= limit) { g[row][col] = UNASSIGNED; return; }
            g[row][col] = UNASSIGNED;
        }
    }
}

int countSolutions(int g[N][N], int limit = 2) {
    int tmp[N][N];
    for (int r = 0; r < N; ++r)
        for (int c = 0; c < N; ++c)
            tmp[r][c] = g[r][c];
    int cnt = 0;
    countSolutionsDFS(tmp, cnt, limit);
    return cnt;
}

// Fill the grid completely using backtracking with randomized digits
bool fillGrid(int g[N][N]) {
    int row, col;
    if (!findUnassigned(g, row, col)) return true;
    int order[9]; shuffledDigits(order);
    for (int i = 0; i < 9; ++i) {
        int num = order[i];
        if (isSafe(g, row, col, num)) {
            g[row][col] = num;
            if (fillGrid(g)) return true;
            g[row][col] = UNASSIGNED;
        }
    }
    return false;
}

// Attempt to remove 'toRemove' cells while preserving uniqueness
void carveUnique(int g[N][N], int toRemove) {
    // Prepare a list of all cell indices [0..80], shuffled
    int cells[81];
    for (int i = 0; i < 81; ++i) cells[i] = i;
    for (int i = 80; i > 0; --i) {
        int j = rand() % (i + 1);
        int tmp = cells[i]; cells[i] = cells[j]; cells[j] = tmp;
    }

    int removed = 0;
    for (int idx = 0; idx < 81 && removed < toRemove; ++idx) {
        int pos = cells[idx];
        int r = pos / 9, c = pos % 9;
        if (g[r][c] == UNASSIGNED) continue;
        int backup = g[r][c];
        g[r][c] = UNASSIGNED;

        // Check if puzzle still has a unique solution
        int solCount = countSolutions(g, 2);
        if (solCount == 1) {
            ++removed; // keep it removed
        } else {
            g[r][c] = backup; // revert removal
        }
    }
}

// Build a full valid grid, then carve according to difficulty (unique solution)
void generatePuzzle(int out[N][N], const string& difficulty) {
    // Start with empty
    for (int r = 0; r < N; ++r)
        for (int c = 0; c < N; ++c)
            out[r][c] = UNASSIGNED;

    // Fill complete solution
    if (!fillGrid(out)) {
        // extremely unlikely; re-try recursively
        generatePuzzle(out, difficulty);
        return;
    }

    // Copy solution (if you want to keep the answer separately, keep 'solution')
    // int solution[N][N]; // not strictly needed here

    // Decide how many cells to remove
    // We'll map by "clues" (remaining filled cells). 81 - clues = toRemove.
    int clues;
    if (difficulty == "easy") clues = 45 + (rand() % 6);         // 45..50
    else if (difficulty == "medium") clues = 34 + (rand() % 6);  // 34..39
    else if (difficulty == "hard") clues = 24 + (rand() % 6);    // 24..29
    else clues = 34 + (rand() % 6); // default medium if unknown

    int toRemove = 81 - clues;
    carveUnique(out, toRemove);
}

void copyGrid(const int src[N][N], int dst[N][N]) {
    for (int r = 0; r < N; ++r)
        for (int c = 0; c < N; ++c)
            dst[r][c] = src[r][c];
}

int main() {
    srand(static_cast<unsigned int>(time(NULL)));

    cout << "=== Sudoku Generator & Solver (C++ / 2D Array / Backtracking) ===\n";
    cout << "Choose difficulty [easy | medium | hard]: ";
    string difficulty;
    if (!(cin >> difficulty)) {
        cerr << "Input error.\n";
        return 1;
    }

    int puzzle[N][N];
    generatePuzzle(puzzle, difficulty);

    cout << "\nGenerated " << difficulty << " puzzle:\n";
    printGrid(puzzle);

    cout << "\nOptions:\n";
    cout << "  1) Solve and show solution\n";
    cout << "  2) Exit\n";
    cout << "Enter choice: ";
    int choice = 0;
    cin >> choice;

    if (choice == 1) {
        int work[N][N];
        copyGrid(puzzle, work);
        if (solveSudoku(work)) {
            cout << "\nSolution:\n";
            printGrid(work);
        } else {
            cout << "No solution found (unexpected for generated puzzles).\n";
        }
    } else {
        cout << "Goodbye!\n";
    }

    return 0;
}
