#ifndef AGENT_H
#define AGENT_H

#include <iostream>
#include <cmath>
#include "Target.h"
#include <vector>

class Agent{
public:
	int agent_id;
    struct State {
        float position[3];
        float velocity;
		float heading;
		float descent_rate;
    } state;

    int current_target;
	bool destroyed;
    float effectiveness;
    float attrition_to_target;
    float max_accel;
    float max_glide_ratio;

    std::vector<int> model;
    std::vector<int> model_confidence_index;
    //std::vector<int> world_state;

    std::vector<Target> all_targets;
    Agent(int,float,float,int,int,float);//, osg::Group* &);
    ~Agent();
	void set_velocity();
    void update_state();
    void agent_destroyed();
    bool get_message(std::vector<int>*,std::vector<int>* );
    float get_cost(std::vector<int> &,const std::vector<float> &,float);
    float get_attrition(int);
    std::vector<int> get_candidates();
    void retarget_bn(const std::vector<float> &,int);
    void retarget_wl(const std::vector<float> &,int);
    void retarget_sa(const std::vector<float> &,int);
    void world_state_from_model();
};
#endif // AGENT_H
