#ifndef ATTACK_HPP
#define ATTACK_HPP

#include "network.hpp"

void attack(const Network& network, const ivec& victims, ivec& target, int side_weight);

#endif  // ATTACK_HPP
