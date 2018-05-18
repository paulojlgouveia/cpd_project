
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

// respectively
// 81 spaces, 162 cells (2 digits), '\n', '\0', possible '\r'
// 9^2=81 => 81^2 + (81*80)/2 , board size + candidates per cell
// MAX_LINE_SIZE * 81
#define MAX_LINE_SIZE 246
#define MAX_STACK_SIZE 9801
#define MAX_BUFFER_SIZE 19926

#define size(n)			n*n
#define table			&(board[0][0])
#define node(ptr)		&(stack[ptr][0][0])

#define KILL 0
#define SOLVED 1
#define REQUEST 2
#define NEW_NODE 3
#define TRY_AGAIN 4




/****************************************************************************/

void getPuzzleFromFile(FILE *fp, int **board, int SIZE);

int **Board(int SIZE);
int ***Stack(int BOARD_SIZE, int DEPTH);
void freeBoard(int **board, int SIZE);
void freeStack(int ***stack, int BOARD_SIZE, int DEPTH);

int master(MPI_Comm master_comm, MPI_Comm new_comm, char *filename);
int slave(MPI_Comm master_comm, MPI_Comm new_comm);

int expandNode(int **board, int BLOCK_SIZE, int BOARD_SIZE, int ***stack, int STACK_SIZE);
int isValid(int row, int column, int number, int **board, int BLOCK_SIZE, int BOARD_SIZE);
int isSolved(int **board, int SIZE);

void copyBoard(int **srcBoard, int **dstBoard, int BOARD_SIZE);
void printBoard(int **board, int SIZE);
void printStack(int ***stack, int SIZE, int STACK_SIZE);

int allWaiting(int *array, int SIZE);

/****************************************************************************/

int master(MPI_Comm master_comm, MPI_Comm new_comm, char *filename) {
	
	FILE *fp;
	char buffer[64];
	
	int L, N;
	int **board;
	
	int stackPtr = 0;
	int ***stack;

	
	double time;
	int solved = 0, *tryAgains;
	int t = 0;

	MPI_Status status;
	int processID, totalProcesses;
	
	
	MPI_Comm_size(master_comm, &totalProcesses);
	MPI_Comm_rank(new_comm, &processID);
	
	
	// discount master and create a vector for slaves waiting for work
	totalProcesses--;
	tryAgains = malloc(totalProcesses * sizeof(int));	
	for (t = 0; t < totalProcesses; t++)
		tryAgains[t] = 0;
	
	// read initial board from the input file
	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("failed to read input file: '%s'\n", filename);
		exit(2);
		
	} else {
		// read first line and compute board size
		fgets(buffer, 64, fp);
		sscanf(buffer,"%d", &L);
		N = L*L;
		
		MPI_Bcast(&L, 1, MPI_INT, processID, MPI_COMM_WORLD);
		
		
		// initialize the board
		board = Board(N);
		getPuzzleFromFile(fp, board, N);		
	}
	
	fclose(fp);

	
	stack = Stack(N, MAX_STACK_SIZE);
	stackPtr = expandNode(board, L, N, stack, MAX_STACK_SIZE);

	
	time = omp_get_wtime();
	
	while(totalProcesses > 0) {
		MPI_Recv(node(stackPtr), size(N), MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, master_comm, &status);
		
		if (!solved) {
			switch (status.MPI_TAG) {
				case SOLVED:
					copyBoard(stack[stackPtr], board, N);
					solved = 1;
// 					printf("solved by %d.\n", status.MPI_SOURCE);
					break;
					
				case REQUEST:
					if (stackPtr) {
						stackPtr--;
						MPI_Send(node(stackPtr), size(N), MPI_INT, status.MPI_SOURCE, NEW_NODE, master_comm);
						
					} else {
						MPI_Send(node(stackPtr), size(N), MPI_INT, status.MPI_SOURCE, TRY_AGAIN, master_comm);
						
						// take note of processes waiting for work
						tryAgains[status.MPI_SOURCE-1]++;
						if (allWaiting(tryAgains, totalProcesses))
							solved = 1;
					}
					break;
					
				case NEW_NODE:
					stackPtr++;
					for (t = 0; t < totalProcesses; t++)
						tryAgains[t] = 0;
					
					break;
			}
			
		} else {
			MPI_Send(table, size(N), MPI_INT, totalProcesses, SOLVED, master_comm);
			totalProcesses--;
		}
	}

	time = omp_get_wtime() -time;
	printf("Time: %f seconds\n", time);
	
	if(isSolved(board, N))
		printBoard(board, N);
	else
		printf("No solution.\n");	
	
	free(tryAgains);
	freeBoard(board, N);
	freeStack(stack, N, MAX_STACK_SIZE);
	
	return 0;
}


int slave(MPI_Comm master_comm, MPI_Comm new_comm) {
	int processID;
	MPI_Status status;
	
	int L, N;
	int **board;
	int ***stack;
	int stackPtr = 0;
	
	int solved = 0;
	

	MPI_Comm_rank(MPI_COMM_WORLD, &processID);

	// get block size and compute board size
	MPI_Bcast(&L, 1, MPI_INT, 0, MPI_COMM_WORLD);
	N = L*L;
	
	board = Board(N);
	stack = Stack(N, N);

	while(!solved) {
		MPI_Send(node(stackPtr), size(N), MPI_INT, 0, REQUEST, master_comm);
		MPI_Recv(table, size(N), MPI_INT, 0, MPI_ANY_TAG, master_comm, &status);
		
		switch (status.MPI_TAG) {
			case TRY_AGAIN:
				break;
				
			case SOLVED:
				solved = 1;
				break;
				
			case NEW_NODE:
				stackPtr = expandNode(board, L, N, stack, N);
				while(stackPtr) {
					stackPtr--;
					if (isSolved(stack[stackPtr], N))
						MPI_Send(node(stackPtr), size(N), MPI_INT, 0, SOLVED, master_comm);
					else
						MPI_Send(node(stackPtr), size(N), MPI_INT, 0, NEW_NODE, master_comm);
				}
				break;
		}
	}
	
	freeBoard(board, N);
	freeStack(stack, N, N);
	
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
		
	
	MPI_Comm_free(&new_comm);
	MPI_Finalize();
	
	return 0;
}




/****************************************************************************/

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


int **Board(int SIZE) {
	int **board, *data;
	data = (int*) malloc(SIZE*SIZE * sizeof(int));
	board = (int**) malloc(SIZE * sizeof(int*));
	for (int i = 0; i < SIZE; i++) 
		board[i] = &(data[SIZE*i]);
	
	return board;
}

int ***Stack(int BOARD_SIZE, int DEPTH) {
	int ***stack = (int***) malloc(DEPTH * sizeof(int**));
	for (int t = 0; t < DEPTH; t++)
		stack[t] = Board(BOARD_SIZE);
	
	return stack;
}

void freeBoard(int **board, int SIZE) {
	free(board[0]);
	free(board);
}

void freeStack(int ***stack, int BOARD_SIZE, int DEPTH) {
	for (int t = 0; t < DEPTH; t++)
		freeBoard(stack[t], BOARD_SIZE);
	
	free(stack);
}


/****************************************************************************/

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
						
						stackPtr++;
					}
				}
				
				return stackPtr;
			}
		}
	}
	
	// allows for check if new nodes where generated
	return stackPtr;	
}

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

int isSolved(int **board, int SIZE) {
	// checks start from lower right corner, board is filled from the upper left
	for (int i = SIZE-1; i >= 0; i--)
		for (int j = SIZE-1; j >= 0; j--)
			if (board[i][j] == 0)
				return 0;
		
	return 1;
}


/****************************************************************************/

void copyBoard(int **srcBoard, int **dstBoard, int BOARD_SIZE) {
	for (int i = 0; i < BOARD_SIZE; i++)
		for (int j = 0; j < BOARD_SIZE; j++)
			dstBoard[i][j] = srcBoard[i][j];
}

void printBoard(int **board, int SIZE) {
	int bPtr = 0;
	char buffer[MAX_BUFFER_SIZE];
	
	if (SIZE < 10) {
		for (int i = 0; i < SIZE; i++) {
			for (int j = 0; j < SIZE; j++)
				bPtr += sprintf(buffer+bPtr, "%d ", board[i][j]);
			
			bPtr += sprintf(buffer+bPtr, "\n");
		}
		
	} else {
		for (int i = 0; i < SIZE; i++) {
			for (int j = 0; j < SIZE; j++)
				bPtr += sprintf(buffer+bPtr, "%2d ", board[i][j]);
			
			bPtr += sprintf(buffer+bPtr, "\n");
		}
	}
	
	printf("%s\n", buffer);
}

void printStack(int ***stack, int SIZE, int STACK_SIZE) {
	int bPtr = 0;
	char buffer[MAX_BUFFER_SIZE];
	
	bPtr += sprintf(buffer+bPtr, "<stack> (size: %d)\n", STACK_SIZE);
	if (SIZE < 10) {
		for (int t = STACK_SIZE-1; t >= 0; t--) {
			for (int i = 0; i < SIZE; i++) {
				bPtr += sprintf(buffer+bPtr, "    ");
				
				for (int j = 0; j < SIZE; j++) {
					bPtr += sprintf(buffer+bPtr, "%d ", stack[t][i][j]);
				}
				bPtr += sprintf(buffer+bPtr, "\n");
			}
			bPtr += sprintf(buffer+bPtr, "\n");
		}
		
	} else {
		for (int t = STACK_SIZE-1; t >= 0; t--) {
			for (int i = 0; i < SIZE; i++) {
				bPtr += sprintf(buffer+bPtr, "    ");
				
				for (int j = 0; j < SIZE; j++) {
					bPtr += sprintf(buffer+bPtr, "%2d ", stack[t][i][j]);
				}
				bPtr += sprintf(buffer+bPtr, "\n");
			}
			bPtr += sprintf(buffer+bPtr, "\n");
		}
	}
	
	printf("%s</stack>\n\n", buffer);
}

int allWaiting(int *array, int SIZE) {
	for (int t = 0; t < SIZE; t++)
		if (array[t] < 2)
			return 0;
	
	return 1;
}

