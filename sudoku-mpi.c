
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

#define L_FROM_FILE 4
#define MATRIX 5



/****************************************************************************/

int ***Stack(int SIZE, int DEPTH);
int **Board(int SIZE);
void getPuzzleFromFile(FILE *fp, int **board, int SIZE);
void freeBoard(int **board, int SIZE);

int master(MPI_Comm master_comm, MPI_Comm new_comm, char *filename);
int slave(MPI_Comm master_comm, MPI_Comm new_comm);

int isValid(int row, int column, int number, int **board, int BLOCK_SIZE, int BOARD_SIZE);
int expandNode(int **board, int BLOCK_SIZE, int BOARD_SIZE, int ***stack, int STACK_SIZE);
int solved(int **board, int SIZE);

int copyBoard(int **srcBoard, int **dstBoard, int BOARD_SIZE);
void printBoard(int **board, int SIZE);



/****************************************************************************/

int master(MPI_Comm master_comm, MPI_Comm new_comm, char *filename) {
	
	FILE *fp;
	int L, N;
	int **board;
	int ***stack;
	
	int i = 0, j = 0;
	int stackPtr = 0;
	
	double time;

	MPI_Status status;
	int processID;
	int totalProcesses, slaves, firstmsg;

	char buf[256], buf2[256];
	
	
	MPI_Comm_size(master_comm, &totalProcesses);
	MPI_Comm_rank(new_comm, &processID);

	
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
		
		MPI_Bcast(&L, 1, MPI_INT, processID, MPI_COMM_WORLD);
		
		
		// initialize the board
		board = Board(N);
		getPuzzleFromFile(fp, board, N);		
	}
	
	fclose(fp);
	
	
	
	// FIXME malloc vector for receiving generated nodes
	stack = Stack(N, MAX_STACK_SIZE);
	
	
	// FIXME expand first board
	stackPtr = expandNode(board, L, N, stack, N);
		
	
	// send initial nodes; // 0 is master
	if (totalProcesses < stackPtr) {
		for (i = 1; i < totalProcesses; i++) {
			stackPtr--;
			MPI_Send(&(stack[stackPtr][0][0]), (N+1)*(N+1), MPI_INT, i, NEW_NODES, MPI_COMM_WORLD);
		}
		
	} else {
		while(stackPtr) {
			stackPtr--;
			MPI_Send(&(stack[stackPtr][0][0]), (N+1)*(N+1), MPI_INT, stackPtr, NEW_NODES, MPI_COMM_WORLD);
		}
	}
	
	
// 	for (; stackPtr >= 0 || )
	
	
	time = omp_get_wtime();
	// FIXME loop send nodes / receive nodes
// 	while(!solved) {
// 		while(stackPtr >= 0) {
// 			MPI_Send(&stack[stackPtr][0][0], N*N, MPI_INT, 1, NEW_NODES, MPI_COMM_WORLD);
// 			stackPtr--;
// 		}
// 		
// // 		MPI_Recv(buf, 256, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, master_comm, &status);
// // 		switch (status.MPI_TAG) {
// // 			case SOLVED:
// // 				slaves--;
// // 				break;
// // 				
// // 			case CLOSED_BRACH:
// // 				fputs(buf, stdout);
// // 				break;
// // 				
// // 			case NEW_NODES:
// // 				MPI_Recv(&(stack[0][0][stackPtr]), N*N, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
// // 				stackPtr++;
// // 				
// // 				break;
// // 		}
// 		
// 		
// 		stackPtr = expandNode(stack[stackPtr], L, N, stack, N);
// 	}
	
	
	

	slaves = totalProcesses - 1;
	while (slaves > 0) {
		MPI_Recv(buf, 256, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, master_comm, &status);
		switch (status.MPI_TAG) {
			case SOLVED:
				slaves--;
				break;
				
			case CLOSED_BRACH:
				fputs(buf, stdout);
				break;
				
			case NEW_NODES:
// 				MPI_Recv(&(stack[0][0][stackPtr]), N*N, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				stackPtr++;
				
				
				/* This lets us get any of the ordered messages first, and then
				start printing them.  There are other ways to do this
				(see related exercises) */
				firstmsg = status.MPI_SOURCE;
				for (i = 1; i<totalProcesses; i++) {
					if (i == firstmsg) {
						fputs(buf, stdout);
						
					} else {
						MPI_Recv(buf2, 256, MPI_CHAR, i, NEW_NODES, master_comm, &status);
						fputs(buf2, stdout);
					}
				}
				break;
		}
	}
	
	time = omp_get_wtime() -time;
	printf("solved: %d\nTime: %f seconds\n", solved(board, N), time);
	printBoard(board, N);
	
	// FIXME free structures
	freeBoard(board, N);
	return 0;
}


int slave(MPI_Comm master_comm, MPI_Comm new_comm) {
	char buf[256];
	int processID;
	MPI_Status status;
	
	int L, N;
	int **board;
	int ***stack;
	int stackSize;
	
	int solved = 0;
	
	
	MPI_Comm_rank(new_comm, &processID);
	
	
	MPI_Bcast(&L, 1, MPI_INT, 0, MPI_COMM_WORLD);
	N = L*L;
	
	board = Board(N);
	// FIXME malloc matrix vector for expanded nodes
	stack = Stack(N, N);
	
	
	MPI_Recv(&(board[0][0]), (N+1)*(N+1), MPI_INT, 0, NEW_NODES, MPI_COMM_WORLD, &status);
	
	
	printBoard(board, N);
	printf("\n");




// 	// FIXME loop receive matrix, generate nodes, check solution, send new nodes
// 	while (!solved) {
// 		printf("slave - 1\n");
// 		MPI_Recv(&(board[0][0]), N*N, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
// 		printf("slave - 2\n");
// 		
// 		switch (status.MPI_TAG) {
// 			case SOLVED:
// // 				return 0;
// 				solved = 1;
// 				break;
// 				
// 			case NEW_NODES:
// 				if ((stackSize = expandNode(board, L, N, stack, N))) {
// 					// FIXME send new nodes to master
// 				}
// 				break;
// 		}
// 	}
	
	
	
	
	sprintf(buf, "Hello from slave %d\n", processID);
	MPI_Send(buf, strlen(buf) + 1, MPI_CHAR, 0, NEW_NODES, master_comm);
	
	sprintf(buf, "Goodbye from slave %d\n", processID);
	MPI_Send(buf, strlen(buf) + 1, MPI_CHAR, 0, NEW_NODES, master_comm);

	sprintf(buf, "I'm exiting (%d)\n", processID);
	MPI_Send(buf, strlen(buf) + 1, MPI_CHAR, 0, CLOSED_BRACH, master_comm);

	MPI_Send(buf, 0, MPI_CHAR, 0, SOLVED, master_comm);
	
	return 0;
}



int main(int argc, char *argv[]) {
	int processID;
	MPI_Comm new_comm;
	
	
	if(argc < 2){
		printf("missing argument.\nusage: sudoku-serial <filename>\n");
		exit(1);
	}

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &processID);
	MPI_Comm_split(MPI_COMM_WORLD, processID == 0, 0, &new_comm);
	
	if (processID == 0)
		master(MPI_COMM_WORLD, new_comm, argv[1]);
	else
		slave(MPI_COMM_WORLD, new_comm);
	
	MPI_Finalize();
	return 0;
}




/****************************************************************************/

int ***Stack(int SIZE, int DEPTH) {
	int ***stack;
	
	stack = (int***) malloc(DEPTH * sizeof(int**));
	for (int i = 0; i < SIZE; i++) {
		stack[i] = (int**) malloc(SIZE * sizeof(int*));
		for (int j = 0; j < SIZE; j++)
			stack[i][j] = (int*) malloc(SIZE * sizeof(int));
	}
	
	return stack;
}

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


/****************************************************************************/

int isValid(int row, int column, int number, int **board, int BLOCK_SIZE, int BOARD_SIZE) {
	int rowStart = (row/BLOCK_SIZE) * BLOCK_SIZE;
    int colStart = (column/BLOCK_SIZE) * BLOCK_SIZE;

    for(int i = 0; i < BOARD_SIZE; ++i) {
        if (board[row][i] == number)
			return 0;
		
        if (board[i][column] == number)
			return 0;
		
		int iFromBlock = rowStart + (i % BLOCK_SIZE);
		int jFromBlock = colStart + (i / BLOCK_SIZE);
		
        if (board[iFromBlock][jFromBlock] == number)
			return 0;
    }
    
    return 1;
}

int expandNode(int **board, int BLOCK_SIZE, int BOARD_SIZE, int ***stack, int STACK_SIZE) {

	int stackPtr = 0;
	
	int i = 0, j = 0;
	int value = BOARD_SIZE;
	
	for (i = 0; i < BOARD_SIZE; i++) {
		for (j = 0; j < BOARD_SIZE; j++) {
			// if empty cell
			if (!board[i][j]) {
				for (value = BOARD_SIZE; value > 0; value--) {
					if (isValid(i, j, value, board, BLOCK_SIZE, BOARD_SIZE)) {
						copyBoard(board, stack[stackPtr], BOARD_SIZE);
						stack[stackPtr][i][j] = value;
						
						printf("%d\n\n", value);
						
						stackPtr++;
					}
				}
			}
			
			return stackPtr;
		}
	}
	
	// allows for check if new nodes where generated
	return stackPtr;	
}


int solved(int **board, int SIZE) {
	// checks start from lower right corner, board is filled from the upper left
	for (int i = SIZE-1; i > -1; i--)
		for (int j = SIZE-1; j > -1; j--)
			if (board[i][j] == 0)
				return 0;
		
	return 1;
}


int copyBoard(int **srcBoard, int **dstBoard, int BOARD_SIZE) {
	for (int i = 0; i < BOARD_SIZE; i++)
		for (int j = 0; j < BOARD_SIZE; j++)
			dstBoard[i][j] = srcBoard[i][j];
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
