#include "lib/network.hpp"
#include "lib/graph_gen.hpp"
#include "lib/place_victims.hpp"
#include "lib/attack.hpp"
#include <map>

static int iipair_second_rcmp(const iipair& a, const iipair& b) {
    return a.second > b.second;
}

int main() {

    RandomConnectedGraphGen rcgen(8, 3.0);
    NormalOrderGraphGen explode_graph_gen(0.25, rcgen);

    GrowParams grow_params = {
        8, // num_children
        0.25, // num_children_sigma
        0.1, // prob_multi_upstream
        0.3, // prob_self_multi_upstream
        0.1, // prob_side_peering
        0.8, // prob_self_side_peering
        &explode_graph_gen,
    };

    int seed;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
    scanf("%d", &seed);
#pragma GCC diagnostic pop
    Network network;
    if(!network.long_sanity_check()) {
        puts("Network has inconsistencies!");
        return 1;
    }

    network.grow(0, 3, false, grow_params, seed);

    if(!network.long_sanity_check()) {
        puts("Network has inconsistencies!");
        return 1;
    }
    printf("Vertices: %d\n", network.num_vertices());
    printf("Edges: %d\n", network.num_edges());

    // network.print(stdout, false);

    ivec victims, targets;
    place_victims_randomly(network, 9, victims, seed);
    attack(network, victims, targets, 100);

    std::map<int, int> freq;
    int leaves = 0;
    for(int v: victims) {
        freq[v] = 0;
    }
    for(int i=0; i<network.size(); ++i) {
        if(network.netsize[i] == 0 && network.depth[i] == network.height) {
            leaves++;
            freq[targets[i]]++;
        }
    }
    vector<iipair> freq2(freq.begin(), freq.end());
    std::sort(freq2.begin(), freq2.end(), iipair_second_rcmp);
    printf("Leaves: %d\n\n", leaves);
    printf("freqs (victim_id, attackers, misdisfact):\n");
    for(const iipair& p: freq2) {
        double rfreq = double(p.second * victims.size()) / leaves;
        printf("  %d: %d: %lf\n", p.first, p.second, rfreq);
    }

    /*
    ipvec l = network.edge_list();
    for(const iipair& p: l) {
        printf("%d %d\n", p.first, p.second);
    }
    */
    return 0;
}
