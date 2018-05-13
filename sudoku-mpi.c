
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

#define FAILED_BRACH 1
#define NEW_NODES 2
#define SOLVED 3

#define MSG_EXIT 1
#define MSG_PRINT_ORDERED 2
#define MSG_PRINT_UNORDERED 3


/****************************************************************************/

typedef struct puzzle {
	int L, N;
	int **table;
} puzzle;

typedef struct element {
	int x, y;
	int value;
	int expanded;
} element;

/****************************************************************************/

puzzle* Puzzle(int modSize);
puzzle* getPuzzleFromFile(char *inputFile);

int master(MPI_Comm master_comm, MPI_Comm new_comm, char *filename);
int slave(MPI_Comm master_comm, MPI_Comm new_comm);
void freePuzzle(puzzle *board);

void printBoard(puzzle *board);


/****************************************************************************/

int master(MPI_Comm master_comm, MPI_Comm new_comm, char *filename) {
	int i,j;
	int size, nslave, firstmsg;
	char buf[256], buf2[256];
	MPI_Status status;
	
	
	puzzle *board;
	double time;

	
	board = getPuzzleFromFile(filename);
	
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
	
	printBoard(board);
	
	freePuzzle(board);
	return 0;}


int slave(MPI_Comm master_comm, MPI_Comm new_comm) {
	char buf[256];
	int rank;
	
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

puzzle* Puzzle(int modSize) {
	puzzle* board = malloc(sizeof(puzzle));
	
	board->L = modSize;
	board->N = modSize * modSize;
	
	board->table = (int**) malloc(board->N * sizeof(int*));
	for (int i = 0; i < board->N; i++) 
		board->table[i] = (int*) malloc(board->N * sizeof(int));
	
	return board;
}

puzzle* getPuzzleFromFile(char *inputFile) {
	
	FILE *file;
	int modSize;
	char line[MAX_LINE_SIZE];
	char* inputCell;
	
	puzzle* board;
	
	// read initial board from the input file
	file = fopen(inputFile, "r");
	if (file == NULL) {	
		printf("failed to read input file: '%s'\n", inputFile);
		exit(2);
	
	} else {
		// read first line (board size)
		fgets(line, MAX_LINE_SIZE, file);
		sscanf(line,"%d", &modSize);
		
		// initialize the board
		board = Puzzle(modSize);
		
		for (int i = 0; i < board->N; i++) {
			if (fgets(line, MAX_LINE_SIZE, file) != NULL) {
				inputCell = strtok(line, " ");
				for (int j = 0; j < board->N; j++) {
					board->table[i][j] = atoi(inputCell);					
					inputCell = strtok(NULL, " ");
				}
				
			} else {
				printf("ill formed file '%s'\n", inputFile);
				exit(3);
			}
		}
		
		fclose(file);
	}
	
	return board;
}

void freePuzzle(puzzle *board) {
	for (int i = 0; i < board->N; i++) {
		free(board->table[i]);
	}  
	
	free(board);
}



void printBoard(puzzle *board) {
	if (board->N > 9) {
		for (int i = 0; i < board->N; i++) {
			for (int j = 0; j < board->N; j++) {
				printf("%2d ", board->table[i][j]);
			}
			printf("\n");
		}
		
	} else {
		for (int i = 0; i < board->N; i++) {
			for (int j = 0; j < board->N; j++) {
				printf("%d ", board->table[i][j]);
			}
			printf("\n");
		}
	}
	printf("\n");
}
