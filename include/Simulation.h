#ifndef SIMULATION_H
#define SIMULATION_H
#include "Target.h"
#include <vector>
class Simulation
{
public:
    Simulation(int);
    std::vector<Target> targets;
};

#endif // SIMULATION_H
