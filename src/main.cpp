#include "mpi.h"
#include "../include/Target.h"
#include "../include/Agent.h"

#define num_targets 9
using namespace std;

vector<Target *> create_simulation();
vector<Target *> load_simulation(int);
int main(int argc, char **argv) {
	// Initialize MPI
	MPI_Init(&argc, &argv);

	// Determine identity
	int agent_id;
	MPI_Comm_rank(MPI_COMM_WORLD, &agent_id);

	// Vector of targets (common to all agents)
	vector<Target *> targets;

	if (agent_id == 0){
		targets = create_simulation();
	} else {
		targets = load_simulation(agent_id);
	}

}

vector<Target *> create_simulation(){
	vector<Target *> targets;
	for (int t=0;t<num_targets;t++){
        Target * temp_target;
		if (t<num_targets/3){
            temp_target = new Target(rand()%751,rand()%751+2500,0.5);
		} else if (t<2*num_targets/3){
            temp_target = new Target(rand()%751,rand()%751+2500,0.7);
		} else{
            temp_target = new Target(rand()%751,rand()%751+2500,0.9);
		}
		targets.push_back( temp_target );
	}
    Agent agent(0,rand()%751,rand()%751,rand()%num_targets,targets,21,0.5);//TODO make 21 be num_agents

    // Broadcast the target vector to all agents
	return targets;
}

vector<Target *> load_simulation(int id){
    vector<Target *> targets;
    Agent agent(id,rand()%751,rand()%751,rand()%num_targets,targets,21,0.5);//TODO make 21 be num_agents
	return targets;
}
