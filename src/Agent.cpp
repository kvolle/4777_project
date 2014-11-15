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
