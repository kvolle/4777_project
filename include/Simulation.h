#ifndef SIMULATION_H
#define SIMULATION_H
#include "Target.h"
#include <vector>
#include <mpi.h>
class Simulation
{
public:
    Simulation(int);
    std::vector<Target> targets;
    MPI_Datatype report_type;
};

#endif // SIMULATION_H
