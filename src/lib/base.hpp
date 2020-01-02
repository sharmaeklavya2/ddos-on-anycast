#ifndef BASE_HPP
#define BASE_HPP

#include <cstdlib>
#include <cstdio>
#include <ctime>

#include <iostream>
#include <utility>
#include <vector>
#include <set>
#include <algorithm>
#include <random>

#include "prettyprint.hpp"

#define MAYBE_INLINE static inline

using std::pair;
using std::vector;
using std::multiset;

using std::ostream;
using std::cerr;
using std::cout;
using std::endl;

typedef long long lli;
typedef long double ldbl;
typedef pair<int, int> iipair;

typedef vector<int> ivec;
typedef multiset<int> imset;
typedef vector<iipair> ipvec;
typedef std::minstd_rand rng_t;

#endif  // BASE_HPP
