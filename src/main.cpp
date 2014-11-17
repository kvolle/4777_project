#include "mpi.h"
#include "../include/Target.h"
#include "../include/Agent.h"
#include "../include/Simulation.h"
#include <iostream>
#include <time.h>

#define num_targets 9
using namespace std;

int main(int argc, char **argv) {
    // Initialize MPI
    int ntasks,num_agents,agent_id;
	MPI_Init(&argc, &argv);
    MPI_Status status;
    MPI_Comm_size(MPI_COMM_WORLD,&ntasks);
    num_agents = ntasks-1;
	// Determine identity
	MPI_Comm_rank(MPI_COMM_WORLD, &agent_id);
    // Seed random number generator
    srand(time(NULL)+agent_id);
    // Create Target datatype
    int num_datatypes = 2;
    MPI_Datatype array_of_types[num_datatypes];
    array_of_types[0] = MPI_FLOAT;
    array_of_types[1] = MPI_INT;
    int array_of_blocklengths[num_datatypes];
    array_of_blocklengths[0] = 4;
    array_of_blocklengths[1] = 1;

    MPI_Aint array_of_displacements[num_datatypes];
    array_of_displacements[0] = 4;
    array_of_displacements[1] = 4;

    // Create new MPI Datatype
    MPI_Datatype target_type;
    MPI_Type_create_struct(num_datatypes,array_of_blocklengths,array_of_displacements,array_of_types,&target_type);
    MPI_Type_commit(&target_type);

    if (agent_id == 0){
        Simulation simulation(num_targets);
        // Broadcast the target vector to all agents
        for (int rank=1;rank<ntasks;rank++){
            for (uint t=0;t<simulation.targets.size();t++){
                MPI_Send(&simulation.targets[t],1,target_type,rank,1,MPI_COMM_WORLD);
            }
        }
        int result;
        //MPI_Recv(&result,1,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
        //cout << result << endl;
	} else {
        Agent agent(agent_id,(float)(rand()/(RAND_MAX/750)),(float)(rand()/(RAND_MAX/750)),rand()/(RAND_MAX/num_targets),num_agents,0.5);
        Target tmp_target;
        for (uint t=0;t<num_targets;t++){
            MPI_Recv(&tmp_target,1,target_type,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
            agent.all_targets.push_back(tmp_target);
        }
        agent.exchange_messages(ntasks-agent_id);
	}
    cout << "Exiting" << endl;
    MPI_Finalize();
    return 0;
}
