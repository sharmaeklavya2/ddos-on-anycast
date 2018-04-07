#include "place_victims.hpp"
#include "dag.hpp"
#include "sample.hpp"

void place_victims_randomly(const Network& network, int n, ivec& victims, int seed) {
    victims.reserve(network.depthwise.back().size());
    for(int u: network.depthwise.back()) {
        if(network.netsize[u] == 0) {
            victims.push_back(u);
        }
    }
    n = std::min<int>(n, victims.size());
    std::shuffle(victims.begin(), victims.end(), rng_t(seed));
    victims.resize(n);
    victims.shrink_to_fit();
}

void make_network_dag(const Network& network, Graph& graph2) {
    Graph graph(network.size());
    for(int d=network.height(); d>=0; --d) {
        for(int v: network.depthwise[d]) {
            int u = network.nid[v];
            if(u < 0 || network.depth[u] != d) {
                if(d > 0) {
                    for(int u: network.up_nbrs[v]) {
                        graph.add_edge(u, v);
                    }
                }
            }
            else {
                graph.add_edge(u, v);
            }
        }
    }
    graph2.swap(graph);
}

void noisy_normalize(vector<double>& weights, int x, double max_noise, rng_t& rng) {
    std::uniform_real_distribution<double> noise_dist(-max_noise, max_noise);
    double weight_sum = std::accumulate(weights.begin(), weights.end(), 0.0);
    for(double& w: weights) {
        w = w / weight_sum + noise_dist(rng);
    }
    weight_sum = std::accumulate(weights.begin(), weights.end(), 0.0);
    for(double& w: weights) {
        w = (w * x) / weight_sum;
    }
}

void distribute(vector<double>& weights, int x, ivec& xvec, rng_t& rng) {
    int n = weights.size();
    if(n == 1) {
        xvec.push_back(x);
        return;
    }

    typedef pair<double, int> dipair;
    vector<dipair> wvec;
    wvec.reserve(n);
    for(int i=0; i<n; ++i) {
        wvec.emplace_back(weights[i], i);
    }
    std::shuffle(wvec.begin(), wvec.end(), rng);

    vector<iipair> xvec2;
    xvec2.reserve(n);
    double acc = 0.000001;
    for(const dipair& p: wvec) {
        double w = p.first;
        int i = p.second;
        acc += w;
        xvec2.emplace_back(i, int(acc+0.4));
        acc -= int(acc+0.4);
    }
    std::sort(xvec2.begin(), xvec2.end());
    xvec.reserve(n);
    for(const iipair& p: xvec2) {
        xvec.push_back(p.second);
    }
}

void place_victims_using_toposort(const Network& network, int n_victims, ivec& victim_list, int seed, double max_noise) {
    int n = network.size();
    Graph graph(0);
    make_network_dag(network, graph);
    ivec rtopolist;
    rtoposort(graph, rtopolist);
    ivec sources, sinks;
    graph.get_sources_and_sinks(sources, sinks);
    // cerr << "rtopolist: " << rtopolist << endl;

    // compute score
    vector<double> score(n, 0.0);
    for(int u: rtopolist) {
        if(graph.succs(u).empty()) {
            // u is a leaf
            score[u] = 1;
        }
        int m = graph.preds(u).size();
        for(int v: graph.preds(u)) {
            score[v] += score[u] / m;
        }
    }

    // cerr << "score: " << score << endl;
    rng_t rng(seed);
    ivec victims(n, 0);

    // divide victims among sources
    vector<double> weights;
    ivec xvec;
    weights.reserve(sources.size());
    for(int u: sources) {
        weights.push_back(score[u]);
    }
    noisy_normalize(weights, n_victims, max_noise, rng);
    distribute(weights, n_victims, xvec, rng);
    int s = sources.size();
    for(int i=0; i<s; ++i) {
        int u = sources[i];
        victims[u] = xvec[i];
    }
    weights.clear();
    xvec.clear();

    // divide victims
    std::reverse(rtopolist.begin(), rtopolist.end());
    for(int u: rtopolist) {
        if(victims[u] > 0) {
            const ivec& succs = graph.succs(u);
            int s = succs.size();
            if(s > 0) {
                weights.reserve(s);
                for(int v: succs) {
                    weights.push_back(score[v] / graph.preds(v).size());
                }
                noisy_normalize(weights, victims[u], max_noise, rng);
                distribute(weights, victims[u], xvec, rng);
                for(int i=0; i<s; ++i) {
                    int v = succs[i];
                    victims[v] += xvec[i];
                }
                weights.clear();
                xvec.clear();
            }
        }
    }

    victim_list.reserve(n_victims);
    for(int u: network.depthwise.back()) {
        if(victims[u] > 0 && !network.is_virtual(u)) {
            victim_list.push_back(u);
        }
    }
}
