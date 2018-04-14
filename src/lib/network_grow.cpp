#include "network.hpp"
#include "graph_gen.hpp"
#include "util.hpp"
#include "sample.hpp"

void Network::expand_node(int u, GraphGen& graph_gen, int seed) {
    if(is_virtual(u) > 0) {
        fprintf(stderr, "Attempt to expand a virtual node\n");
        return;
    }

    rng_t rng1(seed);

    ipvec l;
    int n = graph_gen(l, rng1());

    int ufirst = make_new_vertices(n, u);
    int uend = ufirst + n;
    int d = depth[u];
    for(int v=ufirst; v<uend; ++v) {
        depthwise[d].push_back(v);
    }

    // connect in_nbrs
    for(iipair& p: l) {
        int u = ufirst + p.first, v = ufirst + p.second;
        add_in_edge(u, v);
    }

    // reconnect previous connections
    for(int v: side_nbrs[u]) {
        if(!find_and_erase(side_nbrs[v], u)) {
            cerr << "Couldn't find " << u << " in side_nbrs[" << v << "] = " << side_nbrs[v] << endl;
        }
        int u2 = iunidist(ufirst, ufirst + n - 1, rng1);
        add_side_edge(u2, v);
    }
    for(int v: up_nbrs[u]) {
        if(!find_and_erase(down_nbrs[v], u)) {
            cerr << "Couldn't find " << u << " in down_nbrs[" << v << "] = " << down_nbrs[v] << endl;
        }
        int u2 = iunidist(ufirst, ufirst + n - 1, rng1);
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
        int u2 = iunidist(ufirst, ufirst + n - 1, rng1);
        add_upward_edge(v, u2);
    }
}

void Network::hgrow(GraphGen& graph_gen, int seed) {
    rng_t rng(seed);
    ivec leaves(depthwise.back().begin(), depthwise.back().end());
    for(int u: leaves) {
        if(!is_virtual(u)) {
            expand_node(u, graph_gen, rng());
        }
    }
}

void Network::vgrow(const GrowParams& grow_params, int seed) {

    int n_leaves_rough = depthwise.back().size();
    depthwise.emplace_back();
    int expected_children = grow_params.num_children * n_leaves_rough;
    depthwise.back().reserve(expected_children);
    reserve(size() + expected_children);
    rng_t rng1(seed);

    rng_t rng2(rng1());
    for(int u: depthwise[depthwise.size()-2]) {
        if(!is_virtual(u)) {
            rng_t rng3(rng2());
            int n = do_sigma_n(grow_params.num_children, grow_params.num_children_sigma, rng3);
            int vfirst = make_new_vertices(n, nid[u]);
            int vend = vfirst + n;
            for(int v=vfirst; v<vend; ++v) {
                depth[v] = depth[u]+1;
                depthwise.back().push_back(v);
                add_upward_edge(v, u);
                // optionally add extra upstreams
                rng_t rng4(rng3());
                while(runidist(0, 1, rng4) < grow_params.prob_multi_upstream) {
                    int u2;
                    if(in_nbrs[u].size() + side_nbrs[u].size() == 0 ||
                        runidist(0, 1, rng4) < grow_params.prob_self_multi_upstream) {
                        u2 = u;
                    }
                    else {
                        u2 = sample(in_nbrs[u], side_nbrs[u], rng4);
                    }
                    add_upward_edge(v, u2);
                }
            }
        }
    }

    // add side edges
    rng2.seed(rng1());
    for(int v: depthwise.back()) {
        rng_t rng3(rng2());
        while(runidist(0, 1, rng3) < grow_params.prob_side_peering) {
            ivec weights, items;
            items.reserve(up_nbrs[v].size());
            weights.reserve(up_nbrs[v].size());
            for(int u: up_nbrs[v]) {
                if(down_nbrs[u].size() > 1) {
                    items.push_back(u);
                    weights.push_back(down_nbrs[u].size()-1);
                }
            }
            int u;
            if(weights.empty()) {
                u = sample(up_nbrs[v], rng3);
            }
            else {
                u = weighed_sample(items, weights, rng3);
            }
            int u2 = u;
            if(in_nbrs[u].size() + side_nbrs[u].size() > 0 &&
                runidist(0, 1, rng3) > grow_params.prob_self_side_peering) {
                u2 = sample(in_nbrs[u], side_nbrs[u], rng3);
            }
            int v2 = sample_other(down_nbrs[u2], v, rng3);
            if(v2 != v) {
                add_side_edge(v, v2);
            }
        }
    }
}
