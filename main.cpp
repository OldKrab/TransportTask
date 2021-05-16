#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include "matrix.h"

#define FILE_NAME R"(D:\source\clion\transportTask\in.txt)"

struct Indexes {
    int i, j;
};

class TransportTask {
public:
    dmatrix FindBasicPlan() {
        HandleBalance();
        if (!sorted) {
            SortByC();
            sorted = true;
        }
        auto ac = a, bc = b;
        for (auto ind: sortedByC) {
            int i = ind.i, j = ind.j;
            if (abs(ac[i] - bc[j]) < eps/m && ac[i] > eps)
                ac[i] += eps * i / m;
            db delta = min(ac[i], bc[j]);
            x[i][j] = delta;
            ac[i] -= delta;
            bc[j] -= delta;
        }
        return x;
    }


    void Input() {
        ifstream fin(FILE_NAME);
        fin >> m >> n;
        a.resize(m);
        for (int i = 0; i < m; i++)
            fin >> a[i];
        b.resize(n);
        for (int i = 0; i < n; i++)
            fin >> b[i];
        c.resize(m);
        for (int i = 0; i < m; i++) {
            c[i].resize(n);
            for (int j = 0; j < n; j++)
                fin >> c[i][j];
        }
        x = dmatrix(m, dvector(n, 0));
        fin.close();
    }


    void OutputResult() {
        setlocale(LC_ALL, "rus");
        int wid = 8, firstWid = 12, prc = 3;
        cout << setw(firstWid) << "";
        for (int i = 0; i < b.size(); i++)
            cout << setw(wid) << "B_" + to_string(i + 1);
        cout << setw(wid) << "������" << endl;

        for (int i = 0; i < x.size(); i++) {
            cout << setw(firstWid) << "A_" + to_string(i + 1);
            for (int j = 0; j < x[0].size(); j++)
                printf("%8g",  round(x[i][j]*10*prc)/10./prc);
            cout << setw(wid) << a[i] << endl;
        }
        cout << setw(firstWid) << "�����������";
        for (double bi : b)
            cout << setw(wid) << bi;
        cout << setw(wid) << accumulate(a.begin(), a.end(), 0.);
        db cost = 0;
        for(int i = 0; i < m; i++)
            for(int j = 0; j < n; j++)
                cost += c[i][j]*x[i][j];
        cout << "\n����� ���������: " << cost;
    }

    TransportTask() :sorted(false), m(0), n(0) {}

private:
    void SortByC() {
        sortedByC = vector<Indexes>(n * m);
        for (int i = 0; i < m; i++)
            for (int j = 0; j < n; j++)
                sortedByC[i * n + j] = {i, j};
        sort(sortedByC.begin(), sortedByC.end(),
             [this](Indexes first, Indexes second) { return c[first.i][first.j] < c[second.i][second.j]; });
    }

    void HandleBalance(){
        db sumA = accumulate(a.begin(), a.end(), 0.);
        db sumB = accumulate(b.begin(), b.end(), 0.);
        if(sumB - sumA > eps)
        {
            a.push_back(sumB-sumA);
            x.push_back(dvector(n, 0));
            c.push_back(dvector(n, 0));
            m++;
        }
        else if(sumA - sumB > eps){
            b.push_back(sumA - sumB);
            for(int i = 0; i < n; i++){
                x[i].push_back(0);
                c[i].push_back(0);
            }
            n++;
        }
    }

    int m, n;
    dvector a, b;
    dmatrix c, x;
    vector<Indexes> sortedByC;
    bool sorted;
    db eps = 1e-2;
};


int main() {
    TransportTask tr;
    tr.Input();
    tr.FindBasicPlan();
    tr.OutputResult();

}
