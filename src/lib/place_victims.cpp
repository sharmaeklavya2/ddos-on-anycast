#include "place_victims.hpp"

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
