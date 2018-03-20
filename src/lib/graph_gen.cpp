#include "graph_gen.hpp"
#include "sample.hpp"
#include "dsu.hpp"
#include <list>
#include <tuple>

static inline double norm(double x, double y) {
    return x * x + y * y;
}

void make_cycle_graph(int n, ipvec& l) {
    // iipair st_graph[] = {{0, 1}, {1, 2}, {2, 3}, {3, 0}, {3, 4}, {0, 5}};
    l.emplace_back(n-1, 0);
    for(int i=1; i<n; ++i) {
        l.emplace_back(i-1, i);
    }
}

void make_complete_graph(int n, ipvec& l) {
    for(int j=1; j<n; ++j) {
        for(int i=0; i<j; ++i) {
            l.emplace_back(i, j);
        }
    }
}

void make_random_connected_graph(int n, int approx_m, ipvec& l, int seed) {
    // Make a connected graph with (n vertices and approximately approx_m edges).
    // Returns the number of nodes in this graph. Returns the anti-symmetric edge list in l.
    // int n = std::max(1, int(std::normal_distribution<double>(n, n * 0.25)(rng));

    rng_t rng(seed);

    int orig_approx_m = approx_m;

    int max_edges = n * (n-1) / 2;
    if(approx_m >= max_edges) {
        for(int i=0; i<n; ++i) {
            for(int j=i+1; j<n; ++j) {
                l.emplace_back(i, j);
            }
        }
        return;
    }

    // get distance between all points
    vector<double> xcoords(n, 0.0), ycoords(n, 0.0);
    for(int i=0; i<n; ++i) {
        xcoords[i] = runidist(0, 1, rng);
        ycoords[i] = runidist(0, 1, rng);
    }

    typedef std::tuple<double, int, int> diitrip;
    vector<diitrip> all_edges;
    all_edges.reserve(max_edges);
    for(int i=0; i<n; ++i) {
        for(int j=i+1; j<n; ++j) {
            diitrip t(norm(xcoords[i]-xcoords[j], ycoords[i]-ycoords[j]), i, j);
            all_edges.push_back(t);
        }
    }
    std::sort(all_edges.begin(), all_edges.end());

    // create MST
    DisjointSets ds(n);
    int n_components = n;
    std::list<pair<double, iipair> > all_edges2;
    double weight_sum = 0;
    // all_edges2.reserve(all_edges.size());
    for(const diitrip& t: all_edges) {
        int i, j; double norm;
        std::tie(norm, i, j) = t;
        int i2 = ds.find(i), j2 = ds.find(j);
        if(i2 == j2) {
            double weight = std::exp(-norm/2);
            weight_sum += weight;
            all_edges2.emplace_back(weight, iipair(i, j));
        }
        else {
            n_components--;
            ds.unite(i, j);
            l.push_back(iipair(i, j));
        }
    }
    approx_m -= n-1;

    // cerr << "all_edges: " << all_edges << endl;
    // cerr << "all_edges2: " << all_edges2 << endl;
    // cerr << "l: " << l << endl;
    // cerr << "approx_m: " << approx_m << endl;

    auto it = all_edges2.begin();
    auto it2 = it;
    int loopi;
    for(loopi=0; loopi < 10000 && approx_m > 0; ++loopi) {
        if(all_edges2.empty()) {
            fprintf(stderr, "mrcg(%d, %d): all_edges2={}, approx_m=%d\n", n, orig_approx_m, approx_m);
            break;
        }
        if(it == all_edges2.end()) {
            it = all_edges2.begin();
        }
        it2 = it; ++it2;
        double weight = std::get<0>(*it);
        const iipair& p = std::get<1>(*it);
        if(runidist(0, 1, rng) < (weight/weight_sum)) {
            --approx_m;
            l.push_back(p);
            all_edges2.erase(it);
            weight_sum -= weight;
        }
        it = it2;
    }
    if(loopi == 1000) {
        cerr << "Too much looping\nall_edges2: " << all_edges2 << endl;
    }
}
