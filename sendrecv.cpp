#include "mpi.h"

#define BUF_SIZE 2

int main(int argc, char *argv[]){
    int proc, id;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    MPI_Barrier(MPI_COMM_WORLD);

    double *send_buf=(double*)malloc(sizeof(double)*BUF_SIZE);
    double *recv_buf=(double*)malloc(sizeof(double)*BUF_SIZE);

    send_buf[0] = 1;
    recv_buf[0] = 0;

    if (id == 0)
}