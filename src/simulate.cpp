#include "lib/network.hpp"
#include "lib/graph_gen.hpp"
#include "lib/place_victims.hpp"
#include "lib/attack.hpp"
#include <cstring>
#include <map>
#include <string>

using std::atoi;
using std::atof;
using std::strcmp;
using std::string;

static int iipair_second_rcmp(const iipair& a, const iipair& b) {
    return a.second > b.second;
}

template<class T>
static T coalesce(T value, T replacement, bool zero_is_null) {
    if(zero_is_null) {
        return value <= 0 ? replacement : value;
    }
    else {
        return value < 0 ? replacement : value;
    }
}

struct ArgSpec {
    string name;
    char type;  // 'i' for int, 'f' for double
    string defval_s;
    union {
        int defval_i;
        double defval_f;
    };
    union {
        int val_i;
        double val_f;
    };
    bool zero_is_null;
    string description;

    ArgSpec(const char* _name, char _type, const char* _defval_s, bool _zero_is_null, const char* _description):
        name(_name), type(_type), defval_s(_defval_s), zero_is_null(_zero_is_null), description(_description) {
        if(type == 'i') {
            defval_i = val_i = atoi(_defval_s);
        }
        else if(type == 'f') {
            defval_f = val_f = atof(_defval_s);
        }
    }

    void set_from_string(const char* s) {
        if(type == 'i') {
            val_i = coalesce(atoi(s), defval_i, zero_is_null);
        }
        else if(type == 'f') {
            val_f = coalesce(atof(s), defval_f, zero_is_null);
        }
    }
};

ArgSpec arg_list[] = {
    ArgSpec("n_top",            'i',  "10", 1, "Number of top level ISPs"),
    ArgSpec("n_expand",         'i',  "10", 1, "Number of nodes in expanded graph"),
    ArgSpec("expand_degree",    'f', "3.0", 1, "Average degree of expanded graph"),
    ArgSpec("n_expand_sigma",   'f', "0.3", 0, ""),
    ArgSpec("n_down",           'i',  "10", 1, "Number of children"),
    ArgSpec("n_down_sigma",     'f', "0.3", 0, ""),
    ArgSpec("p_upstream",       'f', "0.1", 0, "Probability of having multiple upstream connection"),
    ArgSpec("p_self_upstream",  'f', "0.3", 0, "Probability that an extra upstream connection points to primary upstream"),
    ArgSpec("p_peering",        'f', "0.1", 0, "Probability of peering"),
    ArgSpec("p_local_peering",  'f', "0.8", 0, "Probability of a peer being a sibling"),
    ArgSpec("top_side_conns",   'i',   "2", 1, "Number of peering links between any 2 top level ISPs"),
    ArgSpec("n_layers",         'i',   "2", 1, "Number of layers in network"),

    ArgSpec("n_victims",    'i', "10", 1, "Number of barrier routers"),
    ArgSpec("smart_distr",  'i',  "1", 0, "Distribute barrier routers hierarchically"),
    ArgSpec("tries",        'i',  "1", 1, "Number of tries when sending barrier routers to children"),
    ArgSpec("reps",         'i',  "1", 1, "Number of times this experiment should be repeated"),
    ArgSpec("seed",         'i',  "0", 0, "Seed for random number generator"),
};

const int n_arg_list = sizeof(arg_list) / sizeof(ArgSpec);

void print_stats(const char* name, vector<double>& xs) {
    std::sort(xs.begin(), xs.end());
    double xsum = 0;
    double xsum2 = 0;
    for(double x: xs) {
        xsum += x;
        xsum2 += x * x;
    }
    int n = xs.size();
    double mean = xsum / n;
    double variance = (xsum2 - xsum*xsum/n)/(n-1);
    double q3 = xs[(3*n)/4];
    double q1 = xs[n/4];

    printf("%s: mean: %lf, max: %lf, 90p: %lf, 75p: %lf, 50p: %lf, variance: %lf, iqd: %lf\n",
        name, mean, xs.back(), xs[int(0.9*n)], q3, xs[n/2], variance, q3 - q1);
}

static void print_usage(FILE* fp, const char* argv0) {
    fprintf(fp, "usage: %s", argv0);
    for(int i=0; i<n_arg_list; ++i) {
        const ArgSpec& arg_spec = arg_list[i];
        fprintf(fp, " %s(=%s)", arg_spec.name.c_str(), arg_spec.defval_s.c_str());
    }
    fprintf(fp, "\n");
}

static void print_help(FILE* fp) {
    for(int i=0; i<n_arg_list; ++i) {
        fprintf(fp, "%-16s (=%3s) %s\n", arg_list[i].name.c_str(),
            arg_list[i].defval_s.c_str(), arg_list[i].description.c_str());
    }
}

std::map<string, int> arg_name_to_index;

int get_arg_i(const char* name) {
    auto it = arg_name_to_index.find(name);
    if(it == arg_name_to_index.end()) {
        fprintf(stderr, "argument %s not found\n", name);
        return -1;
    }
    else {
        return arg_list[it->second].val_i;
    }
}

double get_arg_f(const char* name) {
    auto it = arg_name_to_index.find(name);
    if(it == arg_name_to_index.end()) {
        fprintf(stderr, "argument %s not found\n", name);
        return -1.0;
    }
    else {
        return arg_list[it->second].val_f;
    }
}

int main(int argc, char* argv[]) {

    if(argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        print_usage(stdout, argv[0]);
        printf("\n");
        print_help(stdout);
        return 0;
    }
    else if(argc != n_arg_list+1) {
        print_usage(stderr, argv[0]);
        return 2;
    }
    else {
        for(int i=0; i<n_arg_list; ++i) {
            arg_list[i].set_from_string(argv[i+1]);
            arg_name_to_index[arg_list[i].name] = i;
        }
    }

    GrowParams grow_params;

    int n_top = get_arg_i("n_top");
    int n_expand = get_arg_i("n_expand");
    double expand_degree = get_arg_f("expand_degree");
    double n_expand_sigma = get_arg_f("n_expand_sigma");
    grow_params.num_children = get_arg_i("n_down");
    grow_params.num_children_sigma = get_arg_f("n_down_sigma");
    grow_params.prob_multi_upstream = get_arg_f("p_upstream");
    grow_params.prob_self_multi_upstream = get_arg_f("p_self_upstream");
    grow_params.prob_side_peering = get_arg_f("p_peering");
    grow_params.prob_self_side_peering = get_arg_f("p_local_peering");
    int top_side_conns = get_arg_i("top_side_conns");
    int n_layers = get_arg_i("n_layers");
    int n_victims = get_arg_i("n_victims");
    int smart_distr = get_arg_i("smart_distr");
    int tries = get_arg_i("tries");
    int reps = get_arg_i("reps");
    int seed = get_arg_i("seed");
    if(seed == 0) {
        seed = int(time(NULL));
    }

    CompleteGraphGen complete_gen(n_top);
    RandomConnectedGraphGen rcgen(n_expand, expand_degree);
    NormalOrderGraphGen explode_graph_gen(n_expand_sigma, rcgen);

    rng_t rng(seed);

    vector<double> relcatches(reps, 0.0), misdisfacts(reps, 0.0);

    for(int repi=0; repi < reps; ++repi) {
        Network network(complete_gen, rng(), false, top_side_conns);
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
        if(reps == 1) {
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
        if(smart_distr) {
            place_victims_using_toposort(network, n_victims, victims, seed, tries, 0.01);
        }
        else {
            place_victims_randomly(network, n_victims, victims, seed);
        }
        attack(network, victims, targets, 1, 100);

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
        double relcatch = double(freq2[0].second) / leaves;
        double misdisfact = double(freq2[0].second * victims.size()) / leaves;
        if(reps == 1) {
            printf("freqs (victim_id, attackers, relcatch, misdisfact):\n");
            for(const iipair& p: freq2) {
                double relcatch = double(p.second) / leaves;
                double misdisfact = double(lli(p.second) * victims.size()) / leaves;
                printf("  %5d: %4d: %lf: %lf\n", p.first, p.second, relcatch, misdisfact);
            }
        }
        relcatches[repi] = relcatch;
        misdisfacts[repi] = misdisfact;
    }

    if(reps > 1) {
        print_stats("relcatches", relcatches);
        print_stats("misdisfacts", misdisfacts);
    }
    return 0;
}
