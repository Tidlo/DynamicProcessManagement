//
// Created by focjoe on 12/08/2019.
//

#include "mpi.h"
#include "main.h"
#include<unistd.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

void works(MPI_Comm everyone) {
    sleep(1);
    int rank, size;
    MPI_Comm_rank(everyone, &rank);
    MPI_Comm_size(everyone, &size);
    printf("%s: rank %d in %d doing work...\n", name, rank, size);
}


int main(int argc, char *argv[]) {
    int world_size;
    name = "parent";
    char slave_names[12][4] = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l"};


    int current_slaves = 0;      /**< Current alive slave numbers */

    int init_slave_num = 7;     /** Initially created slave numbers */


    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    if (world_size != 1) printf("Master program should be ran with \" -np 1 \"");


    MPI_Comm everyone;
    // spawn a node
    spawn_and_merge(slave_path, "a", 1, &everyone);
    current_slaves++;

    // loop to create more slave process(es)
    for (int i = 1; i < init_slave_num; ++i) {
        add_process(slave_names[i], &everyone);
        current_slaves++;
    }

    // test to remove process "c"
//    int rank_to_remove = 3;
//    MPI_Send(&rank_to_remove, 1, MPI_INT, rank_to_remove, TAG_REMOVE_SELF, everyone);



    int matched;
    // Main loop

    remove_process(1, &everyone);
    remove_process(1, &everyone);
    remove_process(1, &everyone);


    int i = 0;
    while (1) {
        // remove one process
//        MPI_Iprobe(MPI_ANY_SOURCE, TAG_REMOVE_PROC, everyone, &matched, MPI_STATUS_IGNORE);
//        if(matched) remove_process_handler(&everyone);
//        else printf("parent: did't receive remove process message");

        /*
         *   :::       :::  ::::::::  :::::::::  :::    :::  ::::::::
         *   :+:       :+: :+:    :+: :+:    :+: :+:   :+:  :+:    :+:
         *   +:+       +:+ +:+    +:+ +:+    +:+ +:+  +:+   +:+
         *   +#+  +:+  +#+ +#+    +:+ +#++:++#:  +#++:++    +#++:++#++
         *   +#+ +#+#+ +#+ +#+    +#+ +#+    +#+ +#+  +#+          +#+
         *    #+#+# #+#+#  #+#    #+# #+#    #+# #+#   #+#  #+#    #+#
         *     ###   ###    ########  ###    ### ###    ###  ########
         *
         *    Replace below placeholder works() function with yours.
         */
        works(everyone);
        i++;


    }

    MPI_Finalize();
    return 0;
}

#pragma clang diagnostic pop