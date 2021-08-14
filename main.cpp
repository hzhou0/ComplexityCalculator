#include <iostream>
#include <string>
#include <functional>
#include <chrono>
#include <deque>
#include <algorithm>
#include <cmath>
#include <numeric>


const size_t N1_TRIAL_NUM = 10;
const size_t MAX_N = 20;
const size_t MAX_E = 20;

using namespace std;
using namespace std::chrono;


enum class compType {
    constant, log, n, nlogn, e, uncertain
};

string toString(compType type, unsigned count) {
    string ret;
    if (type == compType::constant)
        ret = "Constant";
    else if (type == compType::log)
        ret = "Log n";
    else if (type == compType::n) {
        ret = "n^";
        ret.append(to_string(count));
    }
    else if (type == compType::nlogn) {
        ret = "n^";
        ret.append(to_string(count));
        ret.append("logn");
    }
    else if (type == compType::e) {
        ret = to_string(count);
        ret.append("^n");
    }
    return ret;
}

struct complexity {
    unsigned n = 0;
    double ratio = 0;
};

void test(int n) {
    int sum1 = 0;
    for (int k = 1; k <= n; k++)
        for (int k = 1; k <= n; k++)
            for (int j = 1; j <= n; j++){
                sum1++;
                int a=sum1;
            }
}

auto
computeQuotients(const deque<complexity> &comp, compType type, unsigned count) {
    deque<double> q;
    for (auto c: comp) {
        double bound;
        if (type == compType::constant)
            bound = log(c.n);
            // logn is bounded above by c.n
        else if (type == compType::log)
            bound = c.n;
            // c.n^b is bounded above by c.n^blogn
        else if (type == compType::n)
            bound = pow(c.n, count) * log(c.n);
            //b^c.n is bounded above by (b+1)^c.n
        else if (type == compType::nlogn)
            bound = pow(c.n, count + 1);
        else
            bound = pow(count + 1, c.n);
        q.push_back(c.ratio / bound);
    }
    return q;
}

auto isDecreasingSequence(const deque<double> &x) {
    auto front = accumulate(x.begin(), x.begin() + (int) (x.size() / 2), 0.0);
    auto back = accumulate(x.end() - (int) (x.size() / 2), x.end(), 0.0);
    return front > back&&(front-back>1e-8*(int)x.size());
}

auto getOComplexity(const function<void(unsigned)> &f,
                    double maxRunTimeSeconds = 100) {
    //calibration runs
    deque<double> n1Trials;
    for (int i = 0; i < N1_TRIAL_NUM; i++) {
        auto start = steady_clock::now();
        f(1);
        auto end = steady_clock::now();
        n1Trials.emplace_back(
                duration_cast<duration<double>>(end - start).count());
    }
    sort(n1Trials.begin(), n1Trials.end());
    auto runTimeN1 = n1Trials[n1Trials.size() / 2];
    double runTime = 0, cumRunTime = 0;
    deque<complexity> tail{};
    unsigned n = 1;
    while (cumRunTime * 2 < maxRunTimeSeconds) {
        auto start = steady_clock::now();
        deque<double> trials;
        for (int i = 0; i < N1_TRIAL_NUM; i++) {
            auto fStart = steady_clock::now();
            f(n);
            auto fEnd = steady_clock::now();
            trials.emplace_back(
                    duration_cast<duration<double>>(fEnd - fStart).count());
        }
        sort(trials.begin(), trials.end());
        runTime = trials[trials.size() / 2];
        complexity c;
        c.ratio = runTime / runTimeN1;
        c.n = n;
        if (tail.size() > 1000)
            tail.pop_front();
        tail.emplace_back(c);
        n++;
        auto end = steady_clock::now();
        cumRunTime += duration_cast<duration<double>>(end - start).count();
    }
    while(tail.size()>n/10){
        tail.pop_front();
    }
    compType type = compType::constant;
    int count = 0;
    while (true) {
        deque<double> q = computeQuotients(tail, type, count);
        if (
                all_of(q.begin(), q.end(),
                       [](double q) { return q < 0.1; }) &&
                isDecreasingSequence(q)) {
            if (type == compType::n) {
                q = computeQuotients(tail, compType::nlogn, count - 1);
                if (
                        all_of(q.begin(), q.end(),
                               [](double q) { return q < 0.1; }) &&
                        isDecreasingSequence(q)) {
                    type = compType::nlogn;
                    count--;
                }
            }
            break;
        }
        else if (type == compType::constant) {
            type = compType::log;
        }
        else if (type == compType::log) {
            type = compType::n;
            count = 1;
        }
        else if (type == compType::n) {
            if (count > MAX_N) {
                type = compType::e;
                count = 2;
            }
            else
                count++;
        }
        else {
            if (count > MAX_N) {
                count = INFINITY;
                break;
            }
            else
                count++;
        }
    }
    return toString(type, count);
}

int main() {
    auto a = getOComplexity(test, 100);
    cout << a << endl;
    return 0;
}
