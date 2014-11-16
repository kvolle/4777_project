#include "../include/Simulation.h"
#include <time.h>

Simulation::Simulation(int num_targets){
    srand(time(NULL));
;    for (int t=0;t<num_targets;t++){
        Target temp_target;
        temp_target.x_position = rand()%751;
        temp_target.y_position = rand()%751+2500.0f;
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
}
