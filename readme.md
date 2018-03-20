# BoR-Sim

Barrier of Routers Simulator.

This project simulates a DDoS attack and computes the amount of attack traffic reaching a set of anycasted victims.
This simulator was created as part of the study
'Mitigating DNS Amplification Attacks using a set of Geographically-Distributed SDN Routers'

### Steps

1.  Create a network:
    Use concepts from the paper
    '[Modeling Internet Topology](https://www.researchgate.net/profile/Matthew_Doar/publication/3195696_Modeling_Internet_Topology/links/00b495395e3da8a00b000000.pdf)'
    by Calvert, Doar and Zegura.
    This basically creates a randomized multi-tier network according to some well-known properties of the internet.

2.  Analyze the network and place victims accordingly on the lowest tier.

3.  Find closest victim from each node in the network.
    Here closeness is measured as hop distance under intra and inter network policy constraints
    suggested in the paper above.

4.  Randomly place attack nodes and calculate attack traffic reaching each victim.

### How to run

    DEBUG=1 make all
    echo 5 | build/debug/network
