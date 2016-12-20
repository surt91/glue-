#pragma once

#include <algorithm>
#include <numeric>
#include <vector>

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
