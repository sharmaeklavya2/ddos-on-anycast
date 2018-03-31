#include "lib/network.hpp"
#include "lib/graph_gen.hpp"
#include "lib/place_victims.hpp"
#include "lib/attack.hpp"
#include <map>

static int iipair_second_rcmp(const iipair& a, const iipair& b) {
    return a.second > b.second;
}

template<class T>
static T neg_coalesce(T value, T replacement) {
    return value < 0 ? replacement : value;
}
template<class T>
static T nonpos_coalesce(T value, T replacement) {
    return value <= 0 ? replacement : value;
}

const char usage_fmt[] = "usage: %s"
" top_level_n graphgen_n graphgen_avg_degree graphgen_n_sigma"
" num_children num_children_sigma"
" prob_multi_upstream prob_self_multi_upstream"
" prob_side_peering prob_self_side_peering"
" top_level_side_connectivity"
" n_layers n_victims strategic_placement repeat seed"
"\n";

using std::atoi;
using std::atof;

int main(int argc, char* argv[]) {
    if(argc != 17) {
        fprintf(stderr, usage_fmt, argv[0]);
        return 2;
    }

    GrowParams grow_params;

    int argi = 1;
    int top_level_n = nonpos_coalesce(atoi(argv[argi++]), 8);
    int graphgen_n = nonpos_coalesce(atoi(argv[argi++]), 8);
    double graphgen_avg_degree = nonpos_coalesce(atof(argv[argi++]), 3.0);
    double graphgen_n_sigma = neg_coalesce(atof(argv[argi++]), 0.25);
    grow_params.num_children = nonpos_coalesce(atoi(argv[argi++]), 8);
    grow_params.num_children_sigma = neg_coalesce(atof(argv[argi++]), 0.25);
    grow_params.prob_multi_upstream = neg_coalesce(atof(argv[argi++]), 0.1);
    grow_params.prob_self_multi_upstream = neg_coalesce(atof(argv[argi++]), 0.3);
    grow_params.prob_side_peering = neg_coalesce(atof(argv[argi++]), 0.1);
    grow_params.prob_self_side_peering = neg_coalesce(atof(argv[argi++]), 0.8);
    int top_level_side_connectivity = neg_coalesce(atoi(argv[argi++]), 2);
    int n_layers = nonpos_coalesce(atoi(argv[argi++]), 2);
    int n_victims = nonpos_coalesce(atoi(argv[argi++]), 10);
    int strategic_placement = neg_coalesce(atoi(argv[argi++]), 1);
    int repeat = nonpos_coalesce(atoi(argv[argi++]), 1);
    int seed = nonpos_coalesce(atoi(argv[argi++]), int(time(NULL)));

    CompleteGraphGen complete_gen(top_level_n);
    RandomConnectedGraphGen rcgen(graphgen_n, graphgen_avg_degree);
    NormalOrderGraphGen explode_graph_gen(graphgen_n_sigma, rcgen);

    rng_t rng(seed);

    double maxrfreq=0, sumrfreq=0;

    for(int repi=0; repi < repeat; ++repi) {
        Network network(complete_gen, rng(), false, top_level_side_connectivity);
        if(!network.long_sanity_check()) {
            puts("Network has inconsistencies!");
            return 1;
        }

        network.hgrow(explode_graph_gen, rng());

        for(int i=1; i<n_layers; ++i) {
            network.vgrow(grow_params, rng());
            network.hgrow(explode_graph_gen, rng());
        }

        int leaves = 0;
        for(int u: network.depthwise.back()) {
            if(!network.is_virtual(u)) {
                leaves++;
            }
        }
        if(repeat == 1) {
            printf("Vertices: %d\n", network.num_vertices());
            printf("Edges: %d\n", network.num_edges());
            printf("Leaves: %d\n", leaves);
            printf("\n");
        }

        if(!network.long_sanity_check()) {
            puts("Network has inconsistencies!");
            network.print(stdout, false);
            return 1;
        }

        ivec victims, targets;
        if(strategic_placement) {
            place_victims_using_toposort(network, n_victims, victims, seed, 0.01);
        }
        else {
            place_victims_randomly(network, n_victims, victims, seed);
        }
        attack(network, victims, targets, 100);

        std::map<int, int> freq;
        for(int v: victims) {
            freq[v] = 0;
        }
        for(int u: network.depthwise.back()) {
            if(network.netsize[u] == 0) {
                freq[targets[u]]++;
            }
        }
        vector<iipair> freq2(freq.begin(), freq.end());
        std::sort(freq2.begin(), freq2.end(), iipair_second_rcmp);
        double rfreq = double(freq2[0].second * victims.size()) / leaves;
        if(repeat == 1) {
            printf("freqs (victim_id, attackers, misdisfact):\n");
            for(const iipair& p: freq2) {
                double rfreq = double(p.second * victims.size()) / leaves;
                printf("  %d: %d: %lf\n", p.first, p.second, rfreq);
            }
        }
        maxrfreq = std::max(maxrfreq, rfreq);
        sumrfreq += rfreq;
    }

    if(repeat > 1) {
        printf("max: %lf, avg: %lf\n", maxrfreq, sumrfreq/repeat);
    }
    return 0;
}
