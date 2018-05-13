
#include <stdio.h>
#include <mpi.h>

int main (int argc, char *argv[]) {

	int BUFF_SIZE = 200;

	MPI_Status status;
	int id, p, i, rounds;
	double secs;
	char name[BUFF_SIZE];

	MPI_Init (&argc, &argv);

	MPI_Comm_rank (MPI_COMM_WORLD, &id);
	MPI_Comm_size (MPI_COMM_WORLD, &p);

	if(argc != 2) {
		if (!id)
			printf ("Command line: %s <n-rounds>\n", argv[0]);

		MPI_Finalize();
		exit (1);
	}

	rounds = atoi(argv[1]);

	MPI_Barrier (MPI_COMM_WORLD);
	secs = - MPI_Wtime();


	MPI_Get_processor_name(name, &BUFF_SIZE);
	printf ("This Process %d sends greetings from machine %s!\n", id, name);


	for(i = 0; i < rounds; i++){
		printf("p[%d] at %s : %d\n", id, name, i);

		if(!id) {
			MPI_Send(&i, 1, MPI_INT, 1, i, MPI_COMM_WORLD);
			MPI_Recv(&i, 1, MPI_INT, p-1, i, MPI_COMM_WORLD, &status);

		} else {
			MPI_Recv(&i, 1, MPI_INT, id-1, i, MPI_COMM_WORLD, &status);
			MPI_Send(&i, 1, MPI_INT, (id+1)%p, i, MPI_COMM_WORLD);
		}

		printf("p[%d] at %s : %d\n", id, name, i);
	}

	MPI_Barrier (MPI_COMM_WORLD);
	secs += MPI_Wtime();

	if(!id) {
		printf("\nRounds= %d, \nN Processes = %d, \nTime = %8.6f sec,\n", rounds, p, secs);
		printf ("Average time per Send/Recv = %6.2f us\n", secs * 1e6 / (2*rounds*p));
	}
	
	MPI_Finalize();

	return 0;
}

		