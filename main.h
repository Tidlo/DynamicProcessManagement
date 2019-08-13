//
// Created by focjoe on 12/08/2019.
//

#ifndef DPM_H
#define DPM_H

#define TAG_ADD_PROC 3
#define TAG_REMOVE_PROC 4
#define TAG_REMOVE_SELF 5

#define COLOR_REMAINED 6
#define COLOR_REMOVED 7

char *name;     /**< process name */
char *slave_path = "/home/focjoe/CLionProjects/dynamic_process_manage/cmake-build-debug/Slave";    /**< path to slave executable*/

/**
 * Spawn a new process and merge the inter-communicator.
 *
 * @param path [in] path to executable
 * @param name [in] process name
 * @param count [in] process number to spawn, mostly is 1
 * @param intra [out] merged intra-communicator, only contains self and parent
 * @return error code of MPI_Comm_merge routine
 */
static int spawn_and_merge(char *path, char *name, int count, MPI_Comm *intra) {
    int err = -1;
    MPI_Comm interr;
    /*MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);*/
    char *argv[2];
    argv[0] = name;
    // spawn a program process and also a inter-communicator with that process
    err = MPI_Comm_spawn(path, argv, count, MPI_INFO_NULL, 0,
                         MPI_COMM_WORLD, &interr, MPI_ERRCODES_IGNORE);

    // merge inter-communicator into a intra-communicator
    err = MPI_Intercomm_merge(interr, 0, intra);
    return err;
}

/**
 * Handler function for non-relative process of spawning to add new process into it
 * local communicator.
 *
 * @param merged [in, out] current local stored intra-communicator
 */
void add_process_handler(MPI_Comm *merged) {
    int err;

    printf("%s: start add_process_handler\n", name);

    int message;
    // consume that message
    MPI_Recv(&message, 1, MPI_INT, MPI_ANY_SOURCE, TAG_ADD_PROC, *merged, MPI_STATUS_IGNORE);
    printf("%s: received message\n", name);
    MPI_Comm inter;
    err = MPI_Intercomm_create(*merged, 0, MPI_COMM_NULL, 0, TAG_ADD_PROC, &inter);
    printf("%s: created inter-communicator between local intra-comm and MPI_COMM_NULL (%d)\n", name, err);

    // barrier to wait all processes finishing inter-communicator create
    err = MPI_Barrier(inter);
    printf("%s: ----- reached barrier: inter in add_process_handler ----- (%d)\n", name, err);

    // merge two end of inter communicator
    MPI_Intercomm_merge(inter, 1, merged);

    // barrier to wait all intercom_merge finished
    err = MPI_Barrier(*merged);
    printf("%s: ----- reached barrier: everyone in add_process_handler ----- (%d)\n", name, err);

    int size;
    MPI_Comm_size(*merged, &size);
    printf("%s: current merged comm size: %d\n", name, size);

}


/**
 * Handler function for non-relative process of the removing to remove process in it
 * local communicator.
 *
 * @param merged [in, out] local intra-communicator
 */
void remove_process_handler(MPI_Comm *merged) {
    int rank;

    printf("%s: start remove_process_handler\n", name);
    int rank_to_remove;
    MPI_Recv(&rank_to_remove, 1, MPI_INT, MPI_ANY_SOURCE, TAG_REMOVE_PROC, *merged, MPI_STATUS_IGNORE);

    printf("%s: removing %d from local communicator\n", name, rank_to_remove);
    MPI_Comm_rank(*merged, &rank);

    int key = (rank > rank_to_remove) ? rank - 1 : rank; // get new rank(key)

    MPI_Comm new_comm;
    MPI_Comm_split(*merged, COLOR_REMAINED, key, &new_comm);

    int err = MPI_Barrier(*merged);
    printf("%s: ----- reached barrier: newcomm in remove_process_handler ----- (%d)\n", name, err);
    *merged = new_comm;
}

/**
 * Handler for remove self from merged communicator
 *
 * @param merged [in] local intra-communicator
 */
void remove_self(MPI_Comm *merged) {
    int self_rank;
    MPI_Comm_rank(*merged, &self_rank);
    int size;
    MPI_Comm_size(*merged, &size);
    for (int i = 0; i < size; ++i) {
        if (i != self_rank) {
            printf("%s: send remove message to rank %d\n", name, i);
            MPI_Send(&self_rank, 1, MPI_INT, i, TAG_REMOVE_PROC, *merged);
        }
    }
    MPI_Comm newcomm;
    // split self into the REMOVED color group
    MPI_Comm_split(*merged, COLOR_REMOVED, self_rank, &newcomm);
    int err = MPI_Barrier(*merged);
    printf("%s: ----- reached barrier: everyone in remove_self_handler ----- (%d)\n", name, err);

}

/**
 * Create new process and add it to all exist process's local intra-communicator.
 *
 * @param name [in] name of newly created process
 * @param everyone [in, out] local intra-communicator that contains all exist processes
 */
void add_process(char *name, MPI_Comm *everyone) {
    // spawn a new process and merge it to 'everyone'
    MPI_Comm master_new_intra, new_inter;
    spawn_and_merge(slave_path, name, 1, &master_new_intra);
    printf("master: spawn and merged %s\n", name);
    // loop to send add process message
    int everyone_size;
    MPI_Comm_size(*everyone, &everyone_size);
    printf("master: size of everyone intra-comm: %d\n", everyone_size);
    int int_buff;
    for (int j = 1; j < everyone_size; ++j) {
        // loop to send add process message to local group
        printf("master: sending message to rank %d\n", j);
        MPI_Send(&int_buff, 1, MPI_INT, j, TAG_ADD_PROC, *everyone);
    }

    MPI_Intercomm_create(*everyone, 0, master_new_intra, 1, TAG_ADD_PROC, &new_inter);
    printf("master: created new inter between everyone intra and new process's local intra via master-new intra \n");

    MPI_Barrier(new_inter);
    printf("master: ----- reached barrier: new_inter -----\n");

    MPI_Intercomm_merge(new_inter, 0, everyone);
    MPI_Barrier(*everyone);
    printf("master: ----- reached barrier: everyone -----\n");
    MPI_Comm_size(*everyone, &everyone_size);
    printf("master: current everyone_intra_size: %d\n", everyone_size);
}


/**
 * Request to remove one process
 *
 * @param everyone [handle] intra-communicator contains the process to remove
 * @param rank_to_remove [in] rank of process to be removed
 */
void remove_process(int rank_to_remove, MPI_Comm *everyone) {
    int buff;
    MPI_Send(&buff, 1, MPI_INT, rank_to_remove, TAG_REMOVE_SELF, *everyone);
    remove_process_handler(everyone);
}

#endif //DPM_H
