#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <cassert>
#include "matrix.h"

#define FILE_NAME R"(D:\GoogleDrive\sync\source\clion\transportTask\in.txt)"

struct Indexes {
    int i, j;
};

struct Cell {
    int value = 0;
    bool isBasic = false;

    bool IsEps() const { return isBasic && value == 0; }
};

typedef vector<vector<Cell>> cellMatrix;

struct TransportModel {
    int m = 0, n = 0;
    vector<int> a, b;
    vector<vector<int>> costs;
    cellMatrix values;
};

void OutputPlan(const TransportModel &);


class TransportTask {
public:
    explicit TransportTask(TransportModel model) : plan(std::move(model)) {}

    TransportModel FindOptimalPlan() {
        plan = FindBasePlan();
        Indexes ind = {};
        while ((ind = CheckOptimalPlan()).i != -1) {
            OutputPlan(plan);
            auto cycle = FindCycle(ind);
            auto minCell = cycle[1];
            // Find min cell among negative cycle cell
            for (int i = 3; i < cycle.size(); i += 2)
                if (plan.values[minCell.i][minCell.j].value > plan.values[cycle[i].i][cycle[i].j].value)
                    minCell = cycle[i];
            // Swap basic and non basic values
            plan.values[minCell.i][minCell.j].isBasic = false;
            plan.values[ind.i][ind.j].isBasic = true;
            // Subtract min cell
            auto minValue = plan.values[minCell.i][minCell.j].value;
            for (int i = 0; i < cycle.size(); i++)
                plan.values[cycle[i].i][cycle[i].j].value += minValue * (i % 2 ? -1 : 1);
        }

        return plan;
    }

    TransportModel FindBasePlan() {
        if (IsOpenModel())
            ConvertOpenModelToClosed();
        if (!sorted) {
            SortFieldsByC();
            sorted = true;
        }
        auto res = plan;
        for (auto ind: fieldsSortedByC) {
            int i = ind.i, j = ind.j;

            if (res.a[i] == res.b[j] && res.a[i] > 0)
                // Set active cell in row to basic eps
                for (int col = 0; col < plan.n; col++)
                    if (res.b[col] > 0 && col != j) {
                        res.values[i][col].isBasic = true;
                        break;
                    }

            int delta = min(res.a[i], res.b[j]);
            res.values[i][j].value += delta;
            if (delta > 0)
                res.values[i][j].isBasic = true;
            res.b[j] -= delta;
            res.a[i] -= delta;
        }
        res.a = plan.a;
        res.b = plan.b;
        return res;
    }

private:
    /// Check optimality of values
    /// \return Indexes of cell with negative estimate or {-1, -1} if is optimal values
    Indexes CheckOptimalPlan() {
        vector<int> aPots, bPots;
        GetPotentials(aPots, bPots);
        assert(CheckCorrectnessPotentials(aPots, bPots));
        for (int i = 0; i < plan.m; ++i)
            for (int j = 0; j < plan.n; ++j)
                if (aPots[i] + bPots[j] > plan.costs[i][j])
                    return {i, j};
        return {-1, -1};
    }

    /// Find cycle from given cell
    /// \param ind given cell indexes
    /// \return vector with values in cycle
    vector<Indexes> FindCycle(Indexes ind) {
        auto cycle = vector<Indexes>();
        auto isVertical = true;
        vector<bool> removedRows, removedCols;
        plan.values[ind.i][ind.j].isBasic = true;
        RemoveRowsAndColsWithOneBasic(removedRows, removedCols);
        cycle.push_back(ind);
        auto currInd = FindNextCellInCycle(ind, isVertical, removedRows, removedCols);
        while (currInd.i != ind.i || currInd.j != ind.j) {
            cycle.push_back(currInd);
            isVertical = !isVertical;
            currInd = FindNextCellInCycle(currInd, isVertical, removedRows, removedCols);
        }
        return cycle;
    }

    /// Remove rows and columns with one basic cell
    void RemoveRowsAndColsWithOneBasic(vector<bool> &removedRows, vector<bool> &removedCols) {
        removedRows.resize(plan.m, false);
        removedCols.resize(plan.n, false);
        bool isAllRemoved = false;
        while (!isAllRemoved) {
            isAllRemoved = true;
            vector<int> cntInRows(plan.m, 0), cntInCols(plan.n, 0);
            for (int i = 0; i < plan.m; i++)
                if (!removedRows[i])
                    for (int j = 0; j < plan.n; ++j)
                        if (!removedCols[j] && plan.values[i][j].isBasic) {
                            cntInRows[i]++;
                            cntInCols[j]++;
                        }
            for (int i = 0; i < plan.m; i++)
                if (cntInRows[i] == 1) {
                    removedRows[i] = true;
                    isAllRemoved = false;
                }
            for (int j = 0; j < plan.n; j++)
                if (cntInCols[j] == 1) {
                    removedCols[j] = true;
                    isAllRemoved = false;
                }
        }
    }

    /// Find next cell in cycle by given cell and direction
    /// \param ind - previous cell
    /// \param isVertical - direction
    Indexes FindNextCellInCycle(Indexes ind, bool isVertical,
                                const vector<bool> &removedRows, const vector<bool> &removedCols) {
        if (isVertical) {
            for (int i = 0; i < plan.m; i++)
                if (!removedRows[i] && i != ind.i && plan.values[i][ind.j].isBasic)
                    return {i, ind.j};
        } else {
            for (int j = 0; j < plan.n; j++)
                if (!removedCols[j] && j != ind.j && plan.values[ind.i][j].isBasic)
                    return {ind.i, j};
        }
        throw runtime_error("Does not find next cell in cycle");
    }

    bool CheckCorrectnessPotentials(const vector<int> &aPots, const vector<int> &bPots) const {
        for (int i = 0; i < plan.m; i++)
            for (int j = 0; j < plan.n; ++j)
                if (plan.values[i][j].isBasic && plan.costs[i][j] != aPots[i] + bPots[j])
                    return false;
        return true;
    }

    void GetPotentials(vector<int> &aPots, vector<int> &bPots) const {
        aPots.resize(plan.m);
        bPots.resize(plan.n);
        vector<bool> aPotsKnown(plan.m, false), bPotsKnown(plan.n, false);
        aPots[0] = 0;
        aPotsKnown[0] = true;
        bool isAllKnown = false;
        while (!isAllKnown) {
            isAllKnown = true;
            for (int i = 0; i < plan.m; i++)
                if (aPotsKnown[i])
                    for (int j = 0; j < plan.n; j++)
                        if (plan.values[i][j].isBasic) {
                            bPots[j] = plan.costs[i][j] - aPots[i];
                            if (!bPotsKnown[j]) {
                                bPotsKnown[j] = true;
                                isAllKnown = false;
                            }
                        }
            for (int j = 0; j < plan.n; j++)
                if (bPotsKnown[j])
                    for (int i = 0; i < plan.m; i++)
                        if (plan.values[i][j].isBasic) {
                            aPots[i] = plan.costs[i][j] - bPots[j];
                            if (!aPotsKnown[i]) {
                                aPotsKnown[i] = true;
                                isAllKnown = false;
                            }
                        }
        }
    }

    void SortFieldsByC() {
        fieldsSortedByC = vector<Indexes>(plan.n * plan.m);
        for (int i = 0; i < plan.m; i++)
            for (int j = 0; j < plan.n; j++)
                fieldsSortedByC[i * plan.n + j] = {i, j};
        sort(fieldsSortedByC.begin(), fieldsSortedByC.end(),
             [this](Indexes first, Indexes second) {
                 return plan.costs[first.i][first.j] < plan.costs[second.i][second.j];
             });
    }

    bool IsOpenModel() {
        int sumA = accumulate(plan.a.begin(), plan.a.end(), 0);
        int sumB = accumulate(plan.b.begin(), plan.b.end(), 0);
        if (sumB != sumA)
            return true;
        return false;
    }

    void ConvertOpenModelToClosed() {
        int sumA = accumulate(plan.a.begin(), plan.a.end(), 0);
        int sumB = accumulate(plan.b.begin(), plan.b.end(), 0);
        if (sumB > sumA) {
            plan.a.push_back(sumB - sumA);
            plan.costs.emplace_back(plan.n, 0);
            plan.values.emplace_back(plan.n);
            plan.m++;
        } else if (sumA > sumB) {
            plan.b.push_back(sumA - sumB);
            for (int i = 0; i < plan.n; i++) {
                plan.costs[i].push_back(0);
                plan.values[i].push_back(Cell());
            }
            plan.n++;
        }
    }

    TransportModel plan;
    vector<Indexes> fieldsSortedByC;
    bool sorted = false;
};

void Input(TransportModel &plan) {
    ifstream fin(FILE_NAME);
    fin >> plan.m >> plan.n;
    plan.a.resize(plan.m);
    for (int i = 0; i < plan.m; i++)
        fin >> plan.a[i];
    plan.b.resize(plan.n);
    for (int i = 0; i < plan.n; i++)
        fin >> plan.b[i];
    plan.costs = vector<vector<int>>(plan.m, vector<int>(plan.n));
    plan.values = vector<vector<Cell>>(plan.m, vector<Cell>(plan.n));
    for (int i = 0; i < plan.m; i++)
        for (int j = 0; j < plan.n; j++)
            fin >> plan.costs[i][j];
    fin.close();
}

void OutputPlan(const TransportModel &plan) {
    setlocale(LC_ALL, "rus");
    int wid = 8, firstWid = 12, prc = 4;

    cout << setw(firstWid) << "";
    for (int i = 0; i < plan.b.size(); i++)
        cout << setw(wid) << "B_" + to_string(i + 1);
    cout << setw(wid) << "Запасы" << endl;

    for (int i = 0; i < plan.a.size(); i++) {
        cout << setw(firstWid) << "A_" + to_string(i + 1);
        for (int j = 0; j < plan.b.size(); j++)
            if (plan.values[i][j].IsEps())
                printf("%*s", wid, "eps");
            else if (plan.values[i][j].value == 0)
                printf("%*s", wid, " ");
            else
                printf("%*g", wid, round(plan.values[i][j].value * pow(10, prc)) / pow(10, prc));
        cout << setw(wid) << plan.a[i] << endl;
    }
    cout << setw(firstWid) << "Потребности";
    for (double bi :plan.b)
        cout << setw(wid) << bi;

    cout << setw(wid) << accumulate(plan.a.begin(), plan.a.end(), 0.);

    db cost = 0;
    for (int i = 0; i < plan.a.size(); i++)
        for (int j = 0; j < plan.b.size(); j++)
            cost += plan.costs[i][j] * plan.values[i][j].value;
    cout << "\nОбщая стоимость: " << cost << "\n";
}

int main() {
    TransportModel plan;
    Input(plan);
    TransportTask tr(plan);
    plan = tr.FindOptimalPlan();
    OutputPlan(plan);
}