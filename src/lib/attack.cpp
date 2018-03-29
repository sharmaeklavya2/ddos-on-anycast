#include "attack.hpp"
#include <queue>
#include <functional>
#include <tuple>

using std::tuple;
typedef std::tuple<int, int> PQElem;
typedef std::priority_queue<PQElem, vector<PQElem>, std::greater<PQElem> > PQ;

void dijkstra(const Network& network, PQ& pq, ivec& target, ivec& target_dist, vector<bool>& visited, int side_weight) {
    while(!pq.empty()) {
        int u, du;
        std::tie(du, u) = pq.top();
        pq.pop();
        if(visited[u]) {
            continue;
        }
        visited[u] = true;
        if(du != target_dist[u]) {
            fprintf(stderr, "u=%d, du=%d, target_dist[%d]=%d\n", u, du, u, target_dist[u]);
        }
        ipvec l;
        l.reserve(network.in_nbrs[u].size() + network.side_nbrs[u].size());
        for(int v: network.in_nbrs[u]) {
            l.emplace_back(v, 1);
        }
        for(int v: network.side_nbrs[u]) {
            l.emplace_back(v, side_weight);
        }
        for(const iipair& p: l) {
            int v = p.first, w = p.second;
            int dv = target_dist[v];
            if(dv == -1 || iipair(du+w, target[u]) < iipair(dv, target[v])) {
                target_dist[v] = du+w;
                target[v] = target[u];
                pq.emplace(du+w, v);
            }
        }
    }
}

void attack(const Network& network, const ivec& victims, ivec& target, int side_weight) {
    // TODO: Remove side edge transitivity

    int n = network.size();
    int h = network.height();

    // init targets
    target.resize(n, -1);
    ivec target_dist(n, -1);
    for(int v: victims) {
        target[v] = v;
        target_dist[v] = 0;
    }

    /*
    vector<ivec> depthwise_victims(h+1);
    for(int victim: victims) {
        depthwise_victims[network.depth[victim]].push_back(victim);
    }
    */

    // Apply Dijkstra's algorithm
    vector<bool> visited(n, false);
    for(int d=h; d>=0; --d) {
        // initialize targets, target_dists and heap
        PQ pq;
        for(int u: network.depthwise[d]) {
            if(target[u] == -1 && !network.down_nbrs[u].empty()) {
                ipvec dvlist;
                dvlist.reserve(network.down_nbrs[u].size());
                for(int v: network.down_nbrs[u]) {
                    if(target_dist[v] != -1) {
                        dvlist.emplace_back(target_dist[v], target[v]);
                    }
                }
                if(!dvlist.empty()) {
                    iipair min_elem = *(std::min_element(dvlist.begin(), dvlist.end()));
                    target[u] = min_elem.second;
                    target_dist[u] = 0;
                }
            }
            if(target_dist[u] != -1) {
                if(target_dist[u] != 0) {
                    fprintf(stderr, "target_dist[%d] = %d instead of 0 or -1\n", u, target_dist[u]);
                }
                pq.push(PQElem(0, u));
            }
        }

        dijkstra(network, pq, target, target_dist, visited, side_weight);
    }

    typedef tuple<int, int, int> itrip;
    ivec target_depth(network.depth.begin(), network.depth.end());

    for(int d=1; d<=h; ++d) {
        PQ pq;
        for(int u: network.depthwise[d]) {
            if(target_dist[u] == -1 && !network.up_nbrs[u].empty()) {
                vector<itrip> dvlist;
                for(int v: network.up_nbrs[u]) {
                    if(target_dist[v] != -1) {
                        dvlist.emplace_back(-target_depth[v], target_dist[v], target[v]);
                    }
                }
                if(!dvlist.empty()) {
                    int v = std::get<2>(*(std::min_element(dvlist.begin(), dvlist.end())));
                    target[u] = target[v];
                    target_dist[u] = 0;
                    target_depth[u] = target_depth[v];
                    pq.emplace(target_dist[u], u);
                }
            }
        }

        dijkstra(network, pq, target, target_dist, visited, side_weight);
    }
    // cerr << "target_dist: " << target_dist << endl;

    for(int i=0; i<n; ++i) {
        if(network.netsize[i] == 0 && (target[i] == -1 || target_dist[i] < 0)) {
            fprintf(stderr, "target[%d] = %d and target_dist[%d] = %d\n", i, target[i], i, target_dist[i]);
        }
    }
}
