#include "../include/Agent.h"
#include <algorithm>
#include <iostream>
using namespace std;
#define model_tag 10
#define confidence_tag 11

#include <cmath>
#include <mpi.h>
Agent::Agent(int id,float x_pos,float y_pos, int init_target,int numAgents,float eff){
    agent_id = id-1;
    state.velocity = 0.0f;
    state.heading = 0.0f;
    state.descent_rate = 0.0f;
    destroyed = false;
    effectiveness = eff;
    max_accel = 15;
    max_glide_ratio = 6.0;
    current_target = init_target;
    state.position[0]=x_pos;
    state.position[1]=y_pos;
    state.position[2]=500;

    model.resize(numAgents,-1);
    model_confidence_index.resize(numAgents,pow(2,30)-1);
    model[agent_id] = current_target;
    model_confidence_index[agent_id] = 0;
}
Agent::~Agent(){}

bool
Agent::exchange_messages(int recipient){
    MPI_Status status;
    std::vector<int> tmp_model,tmp_confidence;
    uint num_agents = model.size();
    tmp_model.resize(num_agents,-1);
    tmp_confidence.resize(num_agents,10000);
    MPI_Send(&model[0],num_agents,MPI_INT,recipient,model_tag,MPI_COMM_WORLD);
    MPI_Send(&model_confidence_index[0],num_agents,MPI_INT,recipient,confidence_tag,MPI_COMM_WORLD);
    MPI_Recv(&tmp_model[0],num_agents,MPI_INT,recipient,model_tag,MPI_COMM_WORLD,&status);
    MPI_Recv(&tmp_confidence[0],num_agents,MPI_INT,recipient,confidence_tag,MPI_COMM_WORLD,&status);

    bool changed = false;
    for (uint t=0;t<num_agents;t++){
        if ((tmp_confidence[t]<=model_confidence_index[t])&&(tmp_model[t]!=model[t])){
            model[t] = tmp_model[t];
            model_confidence_index[t] = tmp_confidence[t]+1;
            changed  = true;
        }
    }
    cout << endl;
    return changed;
}

void Agent::submit_report(){

    report.x_position = state.position[0];
    report.y_position = state.position[1];
    report.z_position = state.position[2];
    report.current_target = current_target;
    if (destroyed){
        report.destroyed = 1;
    }
    else {
        report.destroyed = 0;
    }


}
