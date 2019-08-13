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
    MPI_Init(&argc, &argv);
    MPI_Comm parent;
    MPI_Comm intra, inter, everyone;

    name = argv[1];
    MPI_Comm_get_parent(&parent);
    MPI_Intercomm_merge(parent, 1, &intra);
    everyone = intra;
    int err, size;
    printf("%s: Spawned\n", name);
    // if current process is't initially created process
    if (strcmp(name, "a") != 0) {
        MPI_Intercomm_create(MPI_COMM_WORLD, 0, intra, 0, TAG_ADD_PROC, &inter);
        err = printf("%s: created inter comm between local and parent (%d)\n", name, err);

        // barrier to wait inter communicator create
        err = MPI_Barrier(inter);
        printf("%s: ----- reached barrier: inter ----- (%d)\n", name, err);

        // merge two end of inter communicator
        MPI_Intercomm_merge(inter, 1, &everyone);

        err = MPI_Barrier(everyone);
        printf("%s: ----- reached barrier: everyone ----- (%d)\n", name, err);
    }

    int matched = 0;
    // Main loop
    while (1) {
        // add new process
        MPI_Iprobe(MPI_ANY_SOURCE, TAG_ADD_PROC, everyone, &matched, MPI_STATUS_IGNORE);
        if (matched) add_process_handler(&everyone);

        // remove one process
        MPI_Iprobe(MPI_ANY_SOURCE, TAG_REMOVE_PROC, everyone, &matched, MPI_STATUS_IGNORE);
        if (matched) remove_process_handler(&everyone);

        // remove this process
        MPI_Iprobe(MPI_ANY_SOURCE, TAG_REMOVE_SELF, everyone, &matched, MPI_STATUS_IGNORE);
        if (matched) {
            remove_self(&everyone);
            MPI_Finalize();
            printf("%s: finished and released resources\n", name);
            return 0;
        }

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
    }
}


#pragma clang diagnostic pop