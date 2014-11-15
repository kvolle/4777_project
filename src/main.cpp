#include "mpi.h"
#include "../include/Target.h"
#include "../include/Agent.h"
#include <iostream>

#define num_targets 9
using namespace std;

vector<Target> create_simulation();
vector<Target *> load_simulation();
int main(int argc, char **argv) {
	// Initialize MPI
    int ntasks,num_agents,agent_id;
	MPI_Init(&argc, &argv);
    MPI_Status status;
    MPI_Comm_size(MPI_COMM_WORLD,&ntasks);
    num_agents = ntasks-1;
	// Determine identity
	MPI_Comm_rank(MPI_COMM_WORLD, &agent_id);

    // Create Target datatype
    int num_datatypes = 2;
    MPI_Datatype array_of_types[num_datatypes];
    array_of_types[0] = MPI_FLOAT;
    array_of_types[1] = MPI_INT;
    int array_of_blocklengths[num_datatypes];
    array_of_blocklengths[0] = 3;
    array_of_blocklengths[1] = 1;

    // Says where every block starts in memory, counting rom the beginning of the array
    /*
    MPI_Aint array_of_displacements[num_datatypes];
    MPI_Aint address1, address2, address3,address4;
    MPI_Get_address(&targets[0],&address1);
    MPI_Get_address(&targets[0].y_position,&address2);
    MPI_Get_address(&targets[0].pk,&address3);
    MPI_Get_address(&targets[0].destroyed,&address4);
    array_of_displacements[0] = address2 - address1;
    cout << array_of_displacements[0] << "  " << address3-address2 << "  " << address4-address3 << endl;
    */
    MPI_Aint array_of_displacements[num_datatypes];
    array_of_displacements[0] = 4;
    array_of_displacements[1] = 4;

    // Create new MPI Datatype
    MPI_Datatype target_type;
    MPI_Type_create_struct(num_datatypes,array_of_blocklengths,array_of_displacements,array_of_types,&target_type);
    MPI_Type_commit(&target_type);

	// Vector of targets (common to all agents)
    vector<Target> targets;

	if (agent_id == 0){
		targets = create_simulation();
        Target test = targets[0];
        int test_size = sizeof(test);
        cout << array_of_blocklengths[0]*array_of_displacements[0] << " "<< array_of_blocklengths[1]*array_of_displacements[1] << endl;
        for (int rank=1;rank<ntasks;rank++){
            /*
            MPI_Send(&test_size,        // Message Buffer
                     1,                 // One data item
                     MPI_INT,           // It is an integer
                     rank,              // Receiving process
                     1,                 // Not sure yet
                     MPI_COMM_WORLD);   // Default communicator
        */
            // Broadcast the target vector to all agents
            MPI_Send(&test,1,target_type,rank,1,MPI_COMM_WORLD);
        }
	} else {
        //targets = load_simulation();
        int result;
        /*
        MPI_Recv(&result,
                 1,                     // One Data item
                 MPI_INT,               // It is an int
                 0,                     // From Simulation only
                 MPI_ANY_TAG,           // Still not sure
                 MPI_COMM_WORLD,        // Default communicator
                 &status);
                 */
//        MPI_Recv(&targets[0],1,target_type,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
//        printf("Size is %d\n",targets[0].destroyed);
      //  Agent agent(agent_id,rand()%751,rand()%751,rand()%num_targets,targets,21,0.5);//TODO make 21 be num_agents
	}
    MPI_Finalize();
    return 0;
}

vector<Target> create_simulation(){
    vector<Target> targets;
	for (int t=0;t<num_targets;t++){
        Target temp_target;
        temp_target.x_position = rand()%751;
        temp_target.y_position = rand()%751+2500;
        temp_target.destroyed = 0;
		if (t<num_targets/3){
            //Target temp_target(rand()%751,rand()%751+2500,0.5);
            temp_target.pk = 0.5;

		} else if (t<2*num_targets/3){
            //Target temp_target(rand()%751,rand()%751+2500,0.7);
            temp_target.pk = 0.7;
		} else{
            //Target temp_target(rand()%751,rand()%751+2500,0.9);
            temp_target.pk = 0.9;
		}
        targets.push_back( temp_target );

	}
    return targets;
}

vector<Target *> load_simulation(){
    vector<Target *> targets;

	return targets;
}
