#include "lib/network.hpp"
#include "lib/graph_gen.hpp"
#include "lib/place_victims.hpp"
#include "lib/attack.hpp"
#include <map>

static int iipair_second_rcmp(const iipair& a, const iipair& b) {
    return a.second > b.second;
}

const char usage_fmt[] = "usage: %s"
" graphgen_n graphgen_avg_degree graphgen_n_sigma"
" num_children num_children_sigma"
" prob_multi_upstream prob_self_multi_upstream"
" prob_side_peering prob_self_side_peering"
" n_layers n_victims seed"
"\n";

using std::atoi;
using std::atof;

int main(int argc, char* argv[]) {
    if(argc != 13) {
        fprintf(stderr, usage_fmt, argv[0]);
        return 2;
    }

    int graphgen_n;
    double graphgen_n_sigma, graphgen_avg_degree;
    GrowParams grow_params;

    graphgen_n = atoi(argv[1]);
    graphgen_avg_degree = atof(argv[2]);
    graphgen_n_sigma = atof(argv[3]);
    grow_params.num_children = atoi(argv[4]);
    grow_params.num_children_sigma = atof(argv[5]);
    grow_params.prob_multi_upstream = atof(argv[6]);
    grow_params.prob_self_multi_upstream = atof(argv[7]);
    grow_params.prob_side_peering = atof(argv[8]);
    grow_params.prob_self_side_peering = atof(argv[9]);
    int n_layers = atoi(argv[10]);
    int n_victims = atoi(argv[11]);
    int seed = atoi(argv[12]);

    RandomConnectedGraphGen rcgen(graphgen_n, graphgen_avg_degree);
    NormalOrderGraphGen explode_graph_gen(graphgen_n_sigma, rcgen);
    grow_params.p_explode_graph_gen = &explode_graph_gen;

    Network network;
    if(!network.long_sanity_check()) {
        puts("Network has inconsistencies!");
        return 1;
    }

    network.grow(0, n_layers, false, grow_params, seed);

    if(!network.long_sanity_check()) {
        puts("Network has inconsistencies!");
        return 1;
    }
    printf("Vertices: %d\n", network.num_vertices());
    printf("Edges: %d\n", network.num_edges());

    // network.print(stdout, false);

    ivec victims, targets;
    place_victims_randomly(network, n_victims, victims, seed);
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
