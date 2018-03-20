#include "place_victims.hpp"

void place_victims_randomly(const Network& network, int n, ivec& victims, int seed) {
    int maxdepth = *std::max_element(network.depth.begin(), network.depth.end());
    victims.reserve(network.size());
    for(int i=0; i<network.size(); ++i) {
        if(network.netsize[i] == 0 && network.depth[i] == maxdepth) {
            victims.push_back(i);
        }
    }
    n = std::min<int>(n, victims.size());
    std::shuffle(victims.begin(), victims.end(), rng_t(seed));
    victims.resize(n);
    victims.shrink_to_fit();
}
