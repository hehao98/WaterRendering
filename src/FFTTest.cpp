//
// This file is a collection of different FFT implementatons
// for future reference
//
// Created by 何昊 on 2018/03/14.
//

#include <iostream>
#include <complex>
#include <vector>

using namespace std;

const double PI = 3.1415926f;

// The straightforward implementation of FFT
vector<complex<double>> recursiveFFT(vector<complex<double>> a)
{
    auto n = a.size();
    if (n == 1) return a;

    complex<double> wn = complex<double>(cos(2*PI/n), sin(2*PI/n));
    complex<double> w = 1;
    vector<complex<double>> a0, a1;
    for (int i = 0; i <= n - 2; i += 2)
        a0.push_back(a[i]);
    for (int i = 1; i <= n - 1; i += 2)
        a1.push_back(a[i]);

    auto y0 = recursiveFFT(a0), y1 = recursiveFFT(a1);
    vector<complex<double>> y(n); // Stores result
    for (int k = 0; k <= n/2 - 1; ++k) {
        y[k] = y0[k] + w * y1[k];
        y[k + n/2] = y0[k] - w * y1[k];
        w = w * wn;
    }
    return y;
}

void bitReverseCopy(const vector<complex<double>> &a, vector<complex<double>> &A)
{
    auto n = (int)A.size();
    for (int i = 0; i < n; ++i) {
        // 0b001 -> 0b100
        int revi = i;
        int len = 1;
        for (int j = 0; j < 32; ++j) {
            if ((1 << j) == n) {
                len = j;
                break;
            }
        }
        for (int j = 0; j < len/2; ++j) {
            int tmp1 = (revi >> j) & 0x1;
            int tmp2 = (revi >> (len - 1 - j)) & 0x1;
            revi = revi & ~(1 << j);
            revi = revi | (tmp2 << j);
            revi = revi & ~(1 << (len - 1 - j));
            revi = revi | (tmp1 << (len - 1 - j));
        }
        //cout << i << " " << revi << endl;
        A[revi] = a[i];
    }
}

vector<complex<double>> iterativeFFT(const vector<complex<double>> &a)
{
    auto n = a.size();
    vector<complex<double>> A(n);
    bitReverseCopy(a, A);
    for (int s = 1; (1 << s) <= n; ++s) {
        auto m = (1 << s);
        auto wm = complex<double>(cos(2*PI/m), sin(2*PI/m));
        for (int k = 0; k < n; k += m) {
            complex<double> w = 1.0;
            for (int j = 0; j < m/2; ++j) {
                auto t = w * A[k + j + m/2];
                auto u = A[k + j];
                A[k + j] = u + t;
                A[k + j + m/2] = u - t;
                w = w * wm;
            }
        }
    }
    return A;
}

vector<complex<double>> reverseRecursiveFFT(vector<complex<double>> a)
{
    auto n = a.size();
    if (n == 1) return a;

    complex<double> wn = complex<double>(cos(2*PI/n), sin(2*PI/n));
    complex<double> w = 1.0;
    vector<complex<double>> a0, a1;
    for (int i = 0; i <= n - 2; i += 2)
        a0.push_back(a[i]);
    for (int i = 1; i <= n - 1; i += 2)
        a1.push_back(a[i]);

    auto y0 = reverseRecursiveFFT(a0), y1 = reverseRecursiveFFT(a1);
    vector<complex<double>> y(n); // Stores result
    for (int k = 0; k <= n/2 - 1; ++k) {
        y[k] = (y0[k] + w * y1[k]) / (double)n;
        y[k + n/2] = (y0[k] - w * y1[k]) / (double)n;
        w = w * (1.0/wn);
    }
    return y;
}

int main()
{
    vector<complex<double>> a = {0.1, 0.2, 0.3, 0.4, 0.0, 0.0, 0.0, 0.0};

    auto y = recursiveFFT(a);
    cout << "Result of recursive FFT: " << endl;
    for (auto i : y) cout << i;
    cout << endl;
    auto ry = reverseRecursiveFFT(y);
    cout << "Then reverse FFT to original results: " << endl;
    for (auto i : ry) cout << i;
    cout << endl;

    y = iterativeFFT(a);
    cout << "Result of iterative FFT: " << endl;
    for (auto i : y) cout << i;
    cout << endl;

    return 0;
}