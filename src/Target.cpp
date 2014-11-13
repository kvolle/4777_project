#include "../include/Target.h"
#include <time.h>

Target::Target(float x_pos,float y_pos,float prob){
    position[0] = x_pos;
    position[1] = y_pos;
    position[2] = 0.0f;
    pk = prob;
    destroyed = false;


}

