#include "sample.hpp"
#include "segtree.hpp"

void multi_sample(const vector<double>& weights, int m, ivec& indices, rng_t& rng) {
    int n = weights.size();
    indices.clear();
    if(m >= n) {
        for(int i=0; i<n; ++i) {
            indices.push_back(i);
        }
        return;
    }

    std::uniform_real_distribution<double> rdist;
    indices.reserve(m);
    SegTree segtree(weights);
    for(int i=0; i<m; ++i) {
        int index = segtree.cumsum_search(segtree.get_root() * rdist(rng));
        indices.push_back(index);
        segtree.update(index, 0.0);
    }
}
