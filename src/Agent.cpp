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
    //max_accel = 15;
    //max_glide_ratio = 6.0;
    current_target = init_target;
    state.position[0]=x_pos;
    state.position[1]=y_pos;
    state.position[2]=500;

    model.resize(numAgents,-1);
    model_confidence_index.resize(numAgents,pow(2,30)-1);
    model[agent_id] = current_target;
    model_confidence_index[agent_id] = 0;

    // TODO Fix this
    int num_datatypes = 2;
    MPI_Datatype array_of_types[num_datatypes];
    array_of_types[0] = MPI_FLOAT;
    array_of_types[1] = MPI_INT;
    int array_of_blocklengths[num_datatypes];
    array_of_blocklengths[0] = 3;
    array_of_blocklengths[1] = 2;

    MPI_Aint array_of_displacements[num_datatypes];
    array_of_displacements[0] = 4;
    array_of_displacements[1] = 4;

    // Create new MPI Datatype
    MPI_Type_create_struct(num_datatypes,array_of_blocklengths,array_of_displacements,array_of_types,&report_type);
    MPI_Type_commit(&report_type);
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
/*
    cout << agent_id << ": ";
    for (int j=0;j<num_agents;j++){
        cout << model[j] << " ";
    }
    cout << endl;
*/
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
void
Agent::world_state_from_model(){
    world_state.clear();
    world_state.resize(all_targets.size(),0);
    for (uint i=0;i<world_state.size();i++){
        for (uint j=0;j<model.size();j++){
            if ((int)i == model[j]){
                world_state[i]++;
            }
        }
    }
}
float
Agent::get_cost(std::vector<int> &tmp_state,float attrition){
    attrition = 0.0;
    float cost = 0;
    for (uint i=0;i<tmp_state.size();i++){
        if (1-pow((1-effectiveness+effectiveness*attrition),tmp_state[i])<all_targets[i].pk){
            cost += (all_targets[i].pk-1+pow((1-effectiveness+effectiveness*attrition),tmp_state[i]))/(1-all_targets[i].pk);
        }
    }
    return cost;
}
std::vector<int>
Agent::get_candidates(){
    std::vector<int> candidate_targets;
    for (uint i=0;i<all_targets.size();i++){
        if(!all_targets[i].destroyed){
            //float x_dist = all_targets[i].x_position-state.position[0];
            //float y_dist = all_targets[i].y_position-state.position[1];
            //if (sqrt(x_dist*x_dist+y_dist*y_dist)/state.position[2] < max_glide_ratio){
                candidate_targets.push_back(i);
            //}
        }
    }
    return candidate_targets;
}
void
Agent::retarget_bn(){
    if(!destroyed){
        world_state_from_model();
        // Create a vector of potential targets
        std::vector<int> candidate_targets;
        candidate_targets = get_candidates();
        random_shuffle(candidate_targets.begin(),candidate_targets.end());

//        float tmp_attrition;
//        tmp_attrition = get_attrition(current_target);
        // NOT SURE WHICH WORKS BETTER
        //float min_cost = get_cost(world_state,target_pk,tmp_attrition);
        float min_cost = INFINITY;
        // Default to staying on the same target
        int new_target = current_target;
        float plan_cost = 0.0;
        for (uint i=0;i<candidate_targets.size();i++){
            std::vector<int> result = world_state;
            result[current_target]--;
            result[candidate_targets[i]]++;
//            tmp_attrition = get_attrition(candidate_targets[i]);
            plan_cost = get_cost(result,0);
            if (agent_id == 10){
                printf("%d: %f vs %f, from %d vs %d\n", current_target, min_cost, plan_cost,new_target,candidate_targets[i]);
            }
            if (plan_cost < min_cost){
                min_cost = plan_cost;
                new_target = candidate_targets[i];
            }else{}
        }

        current_target = new_target;
        model[agent_id] = new_target;
        //current_target = candidate_targets[rand()/(RAND_MAX/candidate_targets.size())];
        //model[agent_id] = current_target;
    }
}
