
/****************************************************************************
* 																			*
* Parallel and Distributed Computing 										*
* project part 2															*
* 																			*
* group 4, Alameda															*
* 																			*
* 75657 Paulo Gouveia														*
* 76213 Gonçalo Lopes														*
* 70969 Mario Reis															*
* 																			*
* MPI implementation														*
* 																			*
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <mpi.h>


// 81 spaces, 162 cells (2 digits), '\n', '\0', possible '\r'
#define MAX_LINE_SIZE 246
// 9^2=81 => 81^2 + (81*80)/2 , board size + candidates per cell
#define MAX_STACK_SIZE 9801

#define CLOSED_BRACH 1
#define NEW_NODES 2
#define SOLVED 3

#define MSG_EXIT 1
#define MSG_PRINT_ORDERED 2
#define MSG_PRINT_UNORDERED 3



/****************************************************************************/

int **Board(int SIZE);
void getPuzzleFromFile(FILE *fp, int **board, int SIZE);

int master(MPI_Comm master_comm, MPI_Comm new_comm, char *filename);
int slave(MPI_Comm master_comm, MPI_Comm new_comm);
void freeBoard(int **board, int SIZE);

void printBoard(int **board, int SIZE);



/****************************************************************************/

int master(MPI_Comm master_comm, MPI_Comm new_comm, char *filename) {
	char buf[256], buf2[256];
	int i,j;
	int size, nslave, firstmsg;
	MPI_Status status;
		
	FILE *fp;
	int L, N;
	int **board;
	double time;

	
	// read initial board from the input file
	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("failed to read input file: '%s'\n", filename);
		exit(2);
		
	} else {
		// read first line and compute board size
		fgets(buf, MAX_LINE_SIZE, fp);
		sscanf(buf,"%d", &L);
		N = L*L;
		
		// FIXME send L to slaves
		
		// initialize the board
		board = Board(N);
		getPuzzleFromFile(fp, board, N);		
	}
	
	fclose(fp);
	
	// FIXME malloc vector for receiving generated nodes
	
	// FIXME expand first board
	
	// FIXME loop send nodes / receive nodes
	
	
	time = omp_get_wtime();
	

	MPI_Comm_size(master_comm, &size);
	nslave = size - 1;
	while (nslave > 0) {
		MPI_Recv(buf, 256, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, master_comm, &status);
		switch (status.MPI_TAG) {
			case MSG_EXIT:
				nslave--;
				break;
				
			case MSG_PRINT_UNORDERED:
				fputs(buf, stdout);
				break;
				
			case MSG_PRINT_ORDERED:
				/* This lets us get any of the ordered messages first, and then
				start printing them.  There are other ways to do this
				(see related exercises) */
				firstmsg = status.MPI_SOURCE;
				for (i = 1; i<size; i++) {
					if (i == firstmsg) {
						fputs(buf, stdout);
						
					} else {
						MPI_Recv(buf2, 256, MPI_CHAR, i, MSG_PRINT_ORDERED, master_comm, &status);
						fputs(buf2, stdout);
					}
				}
				break;
		}
	}
	
	time = omp_get_wtime() -time;
	printf("Time: %f seconds\n",time);
	
	
	printBoard(board, N);
	
	// FIXME free structures
	freeBoard(board, N);
	return 0;
}


int slave(MPI_Comm master_comm, MPI_Comm new_comm) {
	char buf[256];
	int rank;
	
	
	// FIXME receive L and compute N
	
	// FIXME malloc matrix for receiving
	
	// FIXME malloc matrix vector for expanded nodes
	
	// FIXME loop receive matrix, generate nodes, check solution, send new nodes
	
	
	
	
	MPI_Comm_rank(new_comm, &rank);
	sprintf(buf, "Hello from slave %d\n", rank);
	MPI_Send(buf, strlen(buf) + 1, MPI_CHAR, 0, MSG_PRINT_UNORDERED, master_comm);
	
	sprintf(buf, "Goodbye from slave %d\n", rank);
	MPI_Send(buf, strlen(buf) + 1, MPI_CHAR, 0, MSG_PRINT_ORDERED, master_comm);

	sprintf(buf, "I'm exiting (%d)\n", rank);
	MPI_Send(buf, strlen(buf) + 1, MPI_CHAR, 0, MSG_PRINT_UNORDERED, master_comm);

	MPI_Send(buf, 0, MPI_CHAR, 0, MSG_EXIT, master_comm);
	
	return 0;
}



int main(int argc, char *argv[]) {
	int rank, size;
	MPI_Comm new_comm;
	
	
	if(argc < 2){
		printf("missing argument.\nusage: sudoku-serial <filename>\n");
		exit(1);
	}

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_split(MPI_COMM_WORLD, rank == 0, 0, &new_comm);
	
	if (rank == 0)
		master(MPI_COMM_WORLD, new_comm, argv[1]);
	else
		slave(MPI_COMM_WORLD, new_comm);
	
	MPI_Finalize();
	return 0;
}




/****************************************************************************/

int **Board(int SIZE) {
	int **board;
	
	board = (int**) malloc(SIZE * sizeof(int*));
	for (int i = 0; i < SIZE; i++) 
		board[i] = (int*) malloc(SIZE * sizeof(int));
	
	return board;
}

void getPuzzleFromFile(FILE *fp, int **board, int SIZE) {
	char line[MAX_LINE_SIZE];
	char* inputCell;
	
	for (int i = 0; i < SIZE; i++) {
		if (fgets(line, MAX_LINE_SIZE, fp) != NULL) {
			inputCell = strtok(line, " ");
			for (int j = 0; j < SIZE; j++) {
				board[i][j] = atoi(inputCell);					
				inputCell = strtok(NULL, " ");
			}
			
		} else {
			printf("ill formed file. \n");
			exit(3);
		}
	}
}

void freeBoard(int **board, int SIZE) {
	for (int i = 0; i < SIZE; i++)
		free(board[i]);  
	
	free(board);
}



void printBoard(int **board, int SIZE) {
	if (SIZE > 9) {
		for (int i = 0; i < SIZE; i++) {
			for (int j = 0; j < SIZE; j++) {
				printf("%2d ", board[i][j]);
			}
			printf("\n");
		}
		
	} else {
		for (int i = 0; i < SIZE; i++) {
			for (int j = 0; j < SIZE; j++) {
				printf("%d ", board[i][j]);
			}
			printf("\n");
		}
	}
	printf("\n");
}
