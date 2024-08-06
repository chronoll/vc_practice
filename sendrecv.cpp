#include "mpi.h"

#define BUF_SIZE 2
#define LOOP 2

int main(int argc, char *argv[]){
    int proc, id;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    MPI_Barrier(MPI_COMM_WORLD);

    double *send_buf=(double*)malloc(sizeof(double)*BUF_SIZE);
    double *recv_buf=(double*)malloc(sizeof(double)*BUF_SIZE);

    for (int l = 1; l <= LOOP; l++) {
        for (int i = 0; i < BUF_SIZE; i++) send_buf[i] = l;
        for (int i = 0; i < BUF_SIZE; i++) recv_buf[i] = 0;
    }
}