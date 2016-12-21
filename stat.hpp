#pragma once

#include <algorithm>
#include <numeric>
#include <vector>
#include <cassert>

template <typename T>
T mean(std::vector<T> a)
{
    return std::accumulate(a.begin(), a.end(), T(0.0)) / a.size();
}

template <typename T>
T weighted_mean(std::vector<T> a, std::vector<double> weight)
{
    double total_weight=0;
    double total=0;
    assert(a.size() == weight.size());
    for(size_t i=0; i<a.size(); ++i)
    {
        total += a[i]*weight[i];
        total_weight += weight[i];
    }
    return total/total_weight;
}

template <typename T>
T variance(std::vector<T> a, T m=T(0))
{
    if(!m)
        m = mean(a); // calculate mean here

    T tmp = std::accumulate(a.begin(), a.end(), T(0.0), [&](T part, T next){return part + (m - next) * (m - next);});
    return tmp/a.size();
}

template <typename T>
T sdev(std::vector<T> a, T m=T(0))
{
    return sqrt(variance(a, m));
}

/// trapz integration with uni-spaced samples
/// https://en.wikipedia.org/wiki/Trapezoidal_rule
inline double trapz(std::vector<double> y, double a, double b)
{
    int N = y.size() - 1;
    double sum = y[0] + y[N];
    for(int i=1; i<N-1; ++i)
        sum += 2*y[i];
    return (b-a)/(2*N) * sum;
}

/// trapz integration with explicit x-values
/// x and y need to have the same length
/// https://en.wikipedia.org/wiki/Trapezoidal_rule
inline double trapz(std::vector<double> x, std::vector<double> y)
{
    int N = y.size() - 1;
    double sum = 0;
    for(int i=0; i<N; ++i)
        sum += (x[i+1] - x[i]) * (y[i+1] + y[i]);
    return sum/2;
}

// calculates \f$\int x p(x) dx\f$ with trapz
inline double histogram_mean(std::vector<double> x, std::vector<double> p)
{
    int N = p.size() - 1;
    double sum = 0;
    for(int i=0; i<N; ++i)
        sum += (x[i+1] - x[i]) * (p[i+1]*x[i+1] + p[i]*x[i]);
    return sum/2;
}
