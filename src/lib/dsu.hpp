#include "base.hpp"

class DisjointSets {
public:
    int n;
    ivec subsize;   // subsize[i] is the size of the subtree rooted at i
    ivec parent;

    void init(int _n) {
        n = _n;
        subsize.resize(n);
        parent.resize(n);
        for(int i=0; i<n; ++i) {
            parent[i] = i;
            subsize[i] = 1;
        }
    }

    explicit DisjointSets(int _n) {
        init(_n);
    }

    int find(int x) {
        if(parent[x] == x) {
            return x;
        }
        else {
            int p = find(parent[x]);
            parent[x] = p;
            return p;
        }
    }

    bool joined(int x, int y) {
        return find(x) == find(y);
    }

    void unite(int x, int y) {
        x = find(x);
        y = find(y);
        if(subsize[x] > subsize[y]) {
            std::swap(x, y);
        }
        parent[x] = y;
        subsize[y] += subsize[x];
    }
};
