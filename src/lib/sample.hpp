#ifndef SAMPLE_HPP
#define SAMPLE_HPP

#include "base.hpp"

MAYBE_INLINE int ord_stat(const ivec& v, int i) {return v[i];}
MAYBE_INLINE int ord_stat(const imset& s, int i) {
    return ivec(s.begin(), s.end())[i];
}

MAYBE_INLINE int iunidist(int first, int last, rng_t& rng) {
    std::uniform_int_distribution<int> dist(first, last);
    return dist(rng);
}

MAYBE_INLINE double runidist(int a, int b, rng_t& rng) {
    std::uniform_real_distribution<double> dist(a, b);
    return dist(rng);
}

MAYBE_INLINE int do_sigma_n(int n, double sigma, rng_t& rng) {
    if(sigma == 0.0) {
        return n;
    }
    else {
        return std::max(1, int(n * std::normal_distribution<double>(1.0, sigma)(rng)));
    }
}

template<class C>
inline typename C::value_type sample(const C& v, rng_t& rng) {
    if(v.empty()) {
        fprintf(stderr, "sample: container is empty\n");
    }
    int i = iunidist(0, v.size() - 1, rng);
    return ord_stat(v, i);
}

template<class C>
inline typename C::value_type sample(const C& v1, const C& v2, rng_t& rng) {
    int i = iunidist(0, v1.size() + v2.size() - 1, rng);
    if(v1.size() + v2.size() == 0) {
        fprintf(stderr, "sample: both containers are empty\n");
    }
    if(i < int(v1.size())) {
        return ord_stat(v1, i);
    }
    else {
        return ord_stat(v2, i-v1.size());
    }
}

inline int weighed_sample(const ivec& v, const ivec& weights, rng_t& rng) {
    if(weights.size() != v.size()) {
        fprintf(stderr, "weighed_sample: container and weights have different sizes\n");
    }
    if(weights.empty()) {
        fprintf(stderr, "weighed_sample: nothing_to_sample\n");
    }
    ivec cum_weights(1, 0);
    cum_weights.reserve(v.size()+1);
    for(auto weight: weights) {
        cum_weights.push_back(weight + cum_weights.back());
    }

    int i = iunidist(0, cum_weights.back() - 1, rng);
    int j = std::upper_bound(cum_weights.begin(), cum_weights.end(), i) - cum_weights.begin() - 1;
    return v[j];
}

template<class C>
inline typename C::value_type sample_other(const C& v, typename C::value_type x, rng_t& rng) {
    if(v.empty()) {
        // fprintf(stderr, "sample_other: container is empty\n");
        return x;
    }
    bool all_x = true;
    for(auto y: v) {
        if(y != x) {
            all_x = false;
            break;
        }
    }
    if(all_x) {
        // fprintf(stderr, "sample_other: container contains no other element\n");
        return x;
    }
    typename C::value_type y = x;
    while(y == x) {
        y = sample(v, rng);
    }
    return y;
}

void multi_sample(const vector<double>& weights, int m, ivec& indices, rng_t& rng);

#endif  // SAMPLE_HPP
