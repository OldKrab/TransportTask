#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include "matrix.h"

#define FILE_NAME R"(D:\GoogleDrive\sync\source\clion\transportTask\in.txt)"

struct Indexes {
    int i, j;
};

struct Cell {
    int value = 0, cost = 0;
    bool isBasic = false;

    bool IsEps() const { return isBasic && value == 0; }
};

typedef vector<vector<Cell>> cellMatrix;

class TransportTask {
public:
    TransportTask(vector<int> &a, vector<int> &b, vector<vector<int>> &costs)
            : m((int) a.size()), n((int) b.size()), a(a), b(b) {
        cells.resize(m, vector<Cell>(n));
        for (int i = 0; i < m; i++)
            for (int j = 0; j < n; ++j)
                cells[i][j].cost = costs[i][j];
    }


    cellMatrix FindOptimalPlan(){
        auto currPlan = FindBasePlan();
        return currPlan;
    }

    cellMatrix FindBasePlan() {
        if (IsOpenModel())
            ConvertOpenModelToClosed();
        if (!sorted) {
            SortFieldsByC();
            sorted = true;
        }
        auto ac = a, bc = b;
        for (auto ind: fieldsSortedByC) {
            int i = ind.i, j = ind.j;

            if (ac[i] == bc[j] && ac[i] > 0)
                // Set active cell in row to basic eps
                for (int col = 0; col < n; col++)
                    if (bc[col] > 0) {
                        cells[i][col].isBasic = true;
                        break;
                    }

            int delta = min(ac[i], bc[j]);
            cells[i][j].value += delta;
            if (delta > 0)
                cells[i][j].isBasic = true;
            ac[i] -= delta;
            bc[j] -= delta;
        }
        return cells;
    }

private:


    void SortFieldsByC() {
        fieldsSortedByC = vector<Indexes>(n * m);
        for (int i = 0; i < m; i++)
            for (int j = 0; j < n; j++)
                fieldsSortedByC[i * n + j] = {i, j};
        sort(fieldsSortedByC.begin(), fieldsSortedByC.end(),
             [this](Indexes first, Indexes second) {
                 return cells[first.i][first.j].cost < cells[second.i][second.j].cost;
             });
    }

    bool IsOpenModel() {
        int sumA = accumulate(a.begin(), a.end(), 0);
        int sumB = accumulate(b.begin(), b.end(), 0);
        if (sumB != sumA)
            return true;
        return false;
    }

    void ConvertOpenModelToClosed() {
        int sumA = accumulate(a.begin(), a.end(), 0);
        int sumB = accumulate(b.begin(), b.end(), 0);
        if (sumB > sumA) {
            a.push_back(sumB - sumA);
            cells.push_back(vector<Cell>(n));
            m++;
        } else if (sumA > sumB) {
            b.push_back(sumA - sumB);
            for (int i = 0; i < n; i++)
                cells[i].push_back(Cell{});
            n++;
        }
    }

    int m, n;
    vector<int> a, b;
    cellMatrix cells;
    vector<Indexes> fieldsSortedByC;
    bool sorted = false;
};


void Input(vector<int> &a, vector<int> &b, vector<vector<int>> &costs) {
    ifstream fin(FILE_NAME);
    int m, n;
    fin >> m >> n;
    a.resize(m);
    for (int i = 0; i < m; i++)
        fin >> a[i];
    b.resize(n);
    for (int i = 0; i < n; i++)
        fin >> b[i];
    costs = vector<vector<int>>(m, vector<int>(n));
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            fin >> costs[i][j];
    fin.close();
}

void OutputPlan(vector<int> &a, vector<int> &b, cellMatrix &cells) {
    setlocale(LC_ALL, "rus");
    int wid = 8, firstWid = 12, prc = 4;

    cout << setw(firstWid) << "";
    for (int i = 0; i < b.size(); i++)
        cout << setw(wid) << "B_" + to_string(i + 1);
    cout << setw(wid) << "Запасы" << endl;

    for (int i = 0; i < a.size(); i++) {
        cout << setw(firstWid) << "A_" + to_string(i + 1);
        for (int j = 0; j < b.size(); j++)
            if (cells[i][j].IsEps())
                printf("%*s", wid, "eps");
            else
                printf("%*g", wid, round(cells[i][j].value * pow(10, prc)) / pow(10, prc));
        cout << setw(wid) << a[i] << endl;
    }
    cout << setw(firstWid) << "Потребности";
    for (double bi : b)
        cout << setw(wid) << bi;

    cout << setw(wid) << accumulate(a.begin(), a.end(), 0.);

    db cost = 0;
    for (int i = 0; i < a.size(); i++)
        for (int j = 0; j < b.size(); j++)
            cost += cells[i][j].cost * cells[i][j].value;
    cout << "\nОбщая стоимость: " << cost;
}

int main() {
    vector<int> a, b;
    vector<vector<int>> costs;

    Input(a, b, costs);
    TransportTask tr(a, b, costs);
    auto plan = tr.FindBasePlan();
    OutputPlan(a, b, plan);
}