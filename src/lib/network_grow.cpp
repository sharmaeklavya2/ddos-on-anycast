#include "network.hpp"
#include "graph_gen.hpp"
#include "util.hpp"
#include "sample.hpp"

void Network::grow(int u, int level, bool fully_expand_last_level, const GrowParams& grow_params, int seed) {
    // fprintf(stderr, "Network::grow(%d, %d) called\n", u, level);

    if(level == 0) {
        return;
    }

    rng_t rng1(seed);

    ipvec l;
    int n = (*(grow_params.p_explode_graph_gen))(l, rng1());

    int ufirst = make_new_vertices(n, u);
    pivot = ufirst;

    // connect in_nbrs
    for(iipair& p: l) {
        int u = ufirst + p.first, v = ufirst + p.second;
        add_in_edge(u, v);
    }

    rng_t rng2(rng1());

    // reconnect previous connections
    for(int v: side_nbrs[u]) {
        if(!find_and_erase(side_nbrs[v], u)) {
            cerr << "Couldn't find " << u << " in side_nbrs[" << v << "] = " << side_nbrs[v] << endl;
        }
        int u2 = iunidist(ufirst, ufirst + n - 1, rng2);
        add_side_edge(u2, v);
    }
    for(int v: up_nbrs[u]) {
        if(!find_and_erase(down_nbrs[v], u)) {
            cerr << "Couldn't find " << u << " in down_nbrs[" << v << "] = " << down_nbrs[v] << endl;
        }
        int u2 = iunidist(ufirst, ufirst + n - 1, rng2);
        /*
        if(u2 < 0) {
            fprintf(stderr, "ufirst=%d, n=%d, u2=%d, u=%d\n", ufirst, n, u2, u);
        }
        */
        add_upward_edge(u2, v);
    }
    for(int v: down_nbrs[u]) {
        if(!find_and_erase(up_nbrs[v], u)) {
            cerr << "Couldn't find " << u << " in up_nbrs[" << v << "] = " << up_nbrs[v] << endl;
        }
        int u2 = iunidist(ufirst, ufirst + n - 1, rng2);
        add_upward_edge(v, u2);
    }

    if(level > 1 || fully_expand_last_level) {

        int first_child = size();
        // make children
        rng2.seed(rng1());
        for(int i=0; i<n; ++i) {
            int u2 = ufirst + i;
            int n2 = do_sigma_n(grow_params.num_children, grow_params.num_children_sigma, rng2);
            int vfirst = make_new_vertices(n2, u);
            for(int j=0; j<n2; ++j) {
                int v = vfirst + j;
                depth[v] = depth[u]+1;
                add_upward_edge(v, u2);
                // optionally add extra upstreams
                rng_t rng3(rng2());
                while(runidist(0, 1, rng3) < grow_params.prob_multi_upstream) {
                    int u3;
                    if(in_nbrs[u2].size() + side_nbrs[u2].size() == 0 ||
                        runidist(0, 1, rng3) < grow_params.prob_self_multi_upstream) {
                        u3 = u2;
                    }
                    else {
                        u3 = sample(in_nbrs[u2], side_nbrs[u2], rng3);
                    }
                    add_upward_edge(v, u3);
                }
            }
        }
        int last_child = size() - 1;

        // add side edges
        rng2.seed(rng1());
        for(int v=first_child; v<=last_child; ++v) {
            rng_t rng3(rng2());
            while(runidist(0, 1, rng3) < grow_params.prob_side_peering) {
                ivec weights, items;
                items.reserve(up_nbrs[v].size());
                weights.reserve(up_nbrs[v].size());
                for(int u2: up_nbrs[v]) {
                    items.push_back(u2);
                    weights.push_back(down_nbrs[u2].size()-1);
                }
                int u2 = weighed_sample(items, weights, rng3);
                int u3 = u2;
                if(runidist(0, 1, rng3) > grow_params.prob_self_side_peering) {
                    u3 = sample(in_nbrs[u2], side_nbrs[u2], rng3);
                }
                int v2 = sample_other(down_nbrs[u3], v, rng3);
                if(v2 != v) {
                    add_side_edge(v, v2);
                }
            }
        }

        // recurse over children
        rng2.seed(rng1());
        for(int v=first_child; v<=last_child; ++v) {
            grow(v, level-1, fully_expand_last_level, grow_params, rng2());
        }
    }

    // fprintf(stderr, "Network::grow(%d, %d) exited\n", u, level);
}
