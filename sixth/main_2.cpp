#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

bool isCycle(vector<vector<char>> grid, int si, int sj, int sdir, int oi, int oj) {
    int dirs[4][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};
    int ci = si, cj = sj, dir = sdir;
    vector<vector<int>> dirc(grid.size(), vector<int>(grid.at(0).size(), -1));
    do {
        ci += dirs[dir][0], cj += dirs[dir][1];
        if(ci >= 0 && ci < grid.size() && cj >= 0 && cj < grid.at(ci).size()) {
            if(dirc.at(ci).at(cj) == dir) {
                return true;
            } else {
                dirc.at(ci).at(cj) = dir;
            }
            if(grid.at(ci).at(cj) == '#' || (ci == oi && cj == oj)) {
                ci -= dirs[dir][0], cj -= dirs[dir][1];
                dir = (dir + 1) % 4;
            }
        }
    } while(ci >= 0 && ci < grid.size() && cj >= 0 && cj < grid.at(ci).size());
    return false;
}

int part1(vector<vector<char>> grid) {
    int dirs[4][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};
    int result = 0, ci = 0, cj = 0, dir = 0;
    for(int i = 0; i < grid.size(); i++) {
        for(int j = 0; j < grid.at(i).size(); j++) {
            if(grid.at(i).at(j) == '^') {
                ci = i, cj = j;
                break;
            }
        }
    }
    do {
        if(grid.at(ci).at(cj) != 'X') {
            grid.at(ci).at(cj) = 'X';
            result++;
        }
        ci += dirs[dir][0], cj += dirs[dir][1];
        if(ci >= 0 && ci < grid.size() && cj >= 0 && cj < grid.at(ci).size()
           && grid.at(ci).at(cj) == '#') {
            ci -= dirs[dir][0], cj -= dirs[dir][1];
            dir = (dir + 1) % 4;
        }
    } while(ci >= 0 && ci < grid.size() && cj >= 0 && cj < grid.at(ci).size());
    return result;
}

int part2(vector<vector<char>> grid) {
    int result = 0, ci = 0, cj = 0, dir = 0;
    for(int i = 0; i < grid.size(); i++) {
        for(int j = 0; j < grid.at(i).size(); j++) {
            if(grid.at(i).at(j) == '^') {
                ci = i, cj = j;
                break;
            }
        }
    }
    for(int i = 0; i < grid.size(); i++) {
        for(int j = 0; j < grid.at(i).size(); j++) {
            if(i == ci && j == cj) {
                continue;
            }
            auto testGrid(grid);
            if(isCycle(testGrid, ci, cj, dir, i, j)) {
                result++;
            }
        }
    }
    return result;
}

int main(int argc, char** argv) {
    string line;
    ifstream input(argv[1]);
    vector<vector<char>> grid;
    while(getline(input, line)) {
        vector<char> lineChars(line.begin(), line.end());
        grid.push_back(lineChars);
    }
    input.close();
    cout << part1(grid) << endl;
    cout << part2(grid) << endl;
    return 0;
}
