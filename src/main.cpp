#include "mpi.h"
#include "../include/Target.h"
#include "../include/Agent.h"
#include "../include/Simulation.h"
#include <iostream>
#include <time.h>

#define num_targets 36
#define sync 99
using namespace std;

int main(int argc, char **argv) {
    // Initialize MPI
    int ntasks,num_agents,task_id;
	MPI_Init(&argc, &argv);
    MPI_Status status;
    MPI_Comm_size(MPI_COMM_WORLD,&ntasks);
    num_agents = ntasks-1;
	// Determine identity
    MPI_Comm_rank(MPI_COMM_WORLD, &task_id);
    // Seed random number generator
    srand(time(NULL)+task_id);
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

    if (task_id == 0){
        Simulation simulation(num_targets);
        // Broadcast the target vector to all agents
        for (int rank=1;rank<ntasks;rank++){
            for (uint t=0;t<simulation.targets.size();t++){
                MPI_Send(&simulation.targets[t],1,target_type,rank,1,MPI_COMM_WORLD);
            }
        }
        int tmp;
        for (int timestamp = 0;timestamp<10;timestamp++){
            for (int n=1;n<ntasks;n++){
                MPI_Recv(&tmp,1,MPI_INT,n,sync,MPI_COMM_WORLD,&status);
            }
            for (int n=1;n<ntasks;n++){
                MPI_Send(&timestamp,1,MPI_INT,n,sync,MPI_COMM_WORLD);
            }
        }
	} else {
        // Generate new agent
        Agent agent(task_id,(float)(rand()/(RAND_MAX/750)),(float)(rand()/(RAND_MAX/750)),rand()/(RAND_MAX/num_targets),num_agents,0.5);
        Target tmp_target;
        // Initialize "RTC"
        int timestamp = 0;
        // Get the set of targets from the master process
        for (uint t=0;t<num_targets;t++){
            MPI_Recv(&tmp_target,1,target_type,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
            agent.all_targets.push_back(tmp_target);
        }
        // Not sure why this is done 100 times. Look into that
        for (int z=0;z<num_agents;z++){
            // Exchange messages with EVERY agent
            for (int j=0;j<num_agents;j++){
                if (j != agent.agent_id){
                    agent.exchange_messages(j+1);
                }
            }
            // Retarget when it is that agent's turn
            if (z%num_agents == agent.agent_id){
                agent.retarget_bn();
            }
            MPI_Send(&timestamp,1,MPI_INT,0,sync,MPI_COMM_WORLD);
            //MPI_Recv(&timestamp,1,MPI_INT,0,sync,MPI_COMM_WORLD,&status);
        }
        printf("%2d W: ",agent.agent_id);
        //cout << agent.agent_id << "W: ";
        for (int a=0;a<agent.world_state.size();a++){
            cout << agent.world_state[a] << " ";
        }
        cout << endl;

	}
    MPI_Finalize();
    return 0;
}
