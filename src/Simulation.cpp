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
            temp_target.pk = 0.5;

        } else if (t<2*num_targets/3){
            temp_target.pk = 0.7;
        } else{
            temp_target.pk = 0.9;
        }
        targets.push_back( temp_target );
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
}
