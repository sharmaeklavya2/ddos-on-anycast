#ifndef PLACE_VICTIMS_HPP
#define PLACE_VICTIMS_HPP

#include "network.hpp"
void place_victims_randomly(const Network& network, int n, ivec& victims, int seed);
void place_victims_using_toposort(const Network& network, int n_victims, ivec& victim_list, int seed, double max_noise=0.01);

#endif  // PLACE_VICTIMS_HPP
