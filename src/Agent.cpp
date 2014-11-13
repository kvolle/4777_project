#include "../include/Agent.h"
#include <time.h>
#include <algorithm>

#include <cmath>
Agent::Agent(int id,float x_pos,float y_pos, int init_target,std::vector<Target*> &tar,int numAgents,float eff):all_targets(tar){//,osg::Group* &root_node): all_targets(tar){
    agent_id = id;
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
};
Agent::~Agent(){};

void
Agent::set_velocity(){
    if ((!destroyed)&&std::isfinite(all_targets[current_target]->position[0])&&std::isfinite(all_targets[current_target]->position[1])){

        // Starting with constant velocity
        // This will change for synchronization
        float x_diff,y_diff;
        x_diff = all_targets[current_target]->position[0] - state.position[0];
        y_diff = all_targets[current_target]->position[1] - state.position[1];
        if (x_diff*x_diff+y_diff*y_diff > 2500){
            state.velocity = 50.0f;
        } else{
            state.velocity = sqrt(x_diff*x_diff + y_diff*y_diff);
        }
        float old_heading = state.heading;
        state.heading = (float)atan2(x_diff,y_diff);
        float accel;
        accel = state.velocity*(cos(state.heading)-cos(old_heading))/0.1;

        if (accel>max_accel){
            state.heading = acos(max_accel*0.1/state.velocity + cos(old_heading))*old_heading/sqrt(old_heading*old_heading);
        }

        state.descent_rate = state.velocity*state.position[2]/sqrt(pow(x_diff,2) + pow(y_diff,2));

	}
	else {
        // This should never get called, but it is good to have to be safe
        if(!destroyed){
            agent_destroyed();
        }
	}
}

void
Agent::update_state(){
    if(!destroyed){
        // Point dynamics, no rotations
        // Delta t = 0.1s (for now)
        // North-East-Down
        float world_velocity[3];
        world_velocity[0] = state.velocity*sin(state.heading);
        world_velocity[1] = state.velocity*cos(state.heading);
        world_velocity[2] = -state.descent_rate;

        // Since velocity doesn't change over the timestep, using Euler
        for(int i=0;i<3;i++){
            state.position[i] = state.position[i] + world_velocity[i]*0.1;
        }
        FILE * sit_rep = fopen("../output_files/sitrep.txt","a");
        if (pow(all_targets[current_target]->position[0]-state.position[0],2)+pow(all_targets[current_target]->position[1]-state.position[1],2)+pow(all_targets[current_target]->position[2]-state.position[2],2)<1.0f){
            // Agent has hit its target
            int threshold = rand()%1001;
            if (all_targets[current_target]->destroyed){
                fprintf(sit_rep,"Agent %d has hit target %d but it was already destroyed\n",agent_id,current_target);
            }
            if (threshold<(1000*effectiveness)){
                all_targets[current_target]->destroyed = true;
            }
            agent_destroyed();
        } else if (state.position[2]<0.1){
            // Agent has hit the ground with no target there
            agent_destroyed();
        }
        fclose(sit_rep);
	}
}
void
Agent::agent_destroyed(){
    current_target = -1;
    model[agent_id] = -1;
    destroyed = true;
}

bool
Agent::get_message(std::vector<int>* message,std::vector<int>* index){
    bool changed = false;
    for (int i=0;(uint)i<message->size();i++){
        if (!(index->at(i)>model_confidence_index[i])){
            //printf("\t about %d  is accepted. %d is less than/equal to %d\n",i,index->at(i),model_confidence_index[i]);
            if (!(model[i] == message->at(i))){
                model[i] = message->at(i);
                changed = true;
            }
            model_confidence_index[i] = index->at(i)+1;
        }else{/*printf("\t about %d rejected. %d is greater than %d\n",i,index->at(i),model_confidence_index[i]);*/}
    }
    return changed;
}
float
Agent::get_cost(std::vector<int> &tmp_state,const std::vector<float> &target_pk,float attrition){
    attrition = 0.0;
    float cost = 0;
    for (uint i=0;i<tmp_state.size();i++){
        if (1-pow((1-effectiveness+effectiveness*attrition),tmp_state[i])<target_pk[i]){
            cost += (target_pk[i]-1+pow((1-effectiveness+effectiveness*attrition),tmp_state[i]))/(1-target_pk[i]);
        }
    }
    return cost;
}
std::vector<int>
Agent::get_candidates(){
    std::vector<int> candidate_targets;
    for (uint i=0;i<all_targets.size();i++){
        if(!all_targets[i]->destroyed){
            float x_dist = all_targets[i]->position[0]-state.position[0];
            float y_dist = all_targets[i]->position[1]-state.position[1];
            if (sqrt(x_dist*x_dist+y_dist*y_dist)/state.position[2] < max_glide_ratio){
                candidate_targets.push_back(i);
            }
        }
    }
    return candidate_targets;
}
void
Agent::retarget_bn(const std::vector<float> &target_pk, int timestep_counter){
    if(!destroyed){
        world_state_from_model();
        // Create a vector of potential targets
        std::vector<int> candidate_targets;
        candidate_targets = get_candidates();
        //random_shuffle(candidate_targets.begin(),candidate_targets.end());

        float tmp_attrition;
        tmp_attrition = get_attrition(current_target);
        //float min_cost = get_cost(world_state,target_pk,tmp_attrition);
        float min_cost = INFINITY;
        // Default to staying on the same target
        int new_target = current_target;
        float plan_cost = 0.0;
        for (uint i=0;i<candidate_targets.size();i++){
            std::vector<int> result = world_state;
            result[current_target]--;
            result[candidate_targets[i]]++;
            tmp_attrition = get_attrition(candidate_targets[i]);
            plan_cost = get_cost(result,target_pk, tmp_attrition);//TODO attrition (properly)
            if (plan_cost < min_cost){
                min_cost = plan_cost;
                new_target = candidate_targets[i];
            }else{}
        }

        current_target = new_target;
        model[agent_id] = new_target;
    }
}
void
Agent::retarget_wl(const std::vector<float> &target_pk, int timestep_counter){
    world_state_from_model();
    // Create a vector of potential targets
    std::vector<int> candidate_targets;
    candidate_targets = get_candidates();
    /*
    for (uint c=0;c<candidate_targets.size();c++){
        if (world_state[candidate_targets[c]] == 0){
            candidate_targets.erase(candidate_targets.begin()+c);
        }
    }*/
    random_shuffle(candidate_targets.begin(),candidate_targets.end());
    // Attempting to minimize cost
    float min_cost = INFINITY;
    // Default to staying on the same target
    int new_target = current_target;

    float plan_cost = 0.0;
    float absent_cost = 0.0;
    float tmp_attrition;
    for (uint i=0;i<candidate_targets.size();i++){
        std::vector<int> result = world_state;
        result[current_target]--;
        absent_cost = get_cost(result,target_pk,1);
        result[candidate_targets[i]]++;
        tmp_attrition = get_attrition(candidate_targets[i]);
        plan_cost = get_cost(result,target_pk, tmp_attrition)-absent_cost;//TODO attrition (properly)
        if (plan_cost < min_cost){
            //printf("Switching - %d from %d to %d\n",agent_id,current_target,candidate_targets[i]);
            min_cost = plan_cost;
            new_target = candidate_targets[i];
        }else{}//printf("NOT - %f > %f Staying %d -- %d\n",plan_cost,min_cost,new_target,timestep_counter);}
    }
    current_target = new_target;
    model[agent_id] = new_target;
}
void
Agent::retarget_sa(const std::vector<float> &target_pk,int timestep_counter){
    world_state_from_model();
    // Create a vector of potential targets
    std::vector<int> candidate_targets;
    candidate_targets = get_candidates();

    for (int i =0;i<5;i++){
        int potential = candidate_targets[rand()%candidate_targets.size()];
        float current_cost = get_cost(world_state,target_pk,attrition_to_target);
        std::vector<int> result = world_state;
        result[current_target]--;
        result[potential]++;
        float tmp_attrition = get_attrition(potential);
        float potential_cost = get_cost(result,target_pk,tmp_attrition);
        float threshold = exp((current_cost-potential_cost)/(1-timestep_counter/10000.0));
        result.clear();
        float random_var = ((float)(rand()%1000))/1000.;
        //printf("%f %f %f : %f\n",current_cost, potential_cost,threshold,random_var);
        if (threshold > random_var){
            //printf("\t %d is switching from %d to %d at time %d\n",agent_id,current_target,potential,timestep_counter);
            current_target = potential;
            model[agent_id] = potential;
        }else{}//printf("\t %d is NOT switching from %d to %d at time %d\n",agent_id,current_target,potential,timestep_counter);}
    }
}
float
Agent::get_attrition(int target_index){
    float attrition;
    if (target_index > -1){
        float x_dist = state.position[0]-all_targets[target_index]->position[0];
        float y_dist = state.position[1]-all_targets[target_index]->position[1];
        attrition = 0.00002*sqrt(x_dist*x_dist + y_dist*y_dist);
    }
    else{
        attrition = 0.0;
    }
    return attrition;
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
