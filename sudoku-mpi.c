
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

#define size(n)		((n+1)*(n+1)+5)
// #define size(n)		(n*n)
#define min(x, y)	((x < y)? (x) : (y))

#define KILL 0
#define SOLVED 1
#define REQUEST 2
#define NEW_NODE 3
#define TRY_AGAIN 4



// #define CLOSED_BRACH 1
// #define NEW_NODES 2
// #define SOLVED 3
// 
// #define L_FROM_FILE 4
// #define MATRIX 5



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



/****************************************************************************/

int master(MPI_Comm master_comm, MPI_Comm new_comm, char *filename) {
	
	FILE *fp;
	char buffer[64];
	
	int L, N;
	int **board;
	int ***stack;
	
	int p = 0;
	int stackPtr = 0;
	
	double time;
	int solved = 0;

	MPI_Status status;
	int processID, totalProcesses;

	
	
	MPI_Comm_size(master_comm, &totalProcesses);
	MPI_Comm_rank(new_comm, &processID);

	
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

	
	// FIXME generate at least 1 node per process?
// 	// send initial nodes; // 0 is master
// 	for (p = 1; p < totalProcesses && stackPtr > 0; p++) {
// 		stackPtr--;
// 		MPI_Send(&(stack[stackPtr][0][0]), size(N), MPI_INT, p, NEW_NODE, master_comm);
// 	}
	
	
	
	time = omp_get_wtime();
	
	
// 	printf("stackPtr = %d\n", stackPtr);
	while(!solved) {
// 		MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, master_comm, &status);		
		MPI_Recv(&(stack[stackPtr][0][0]), size(N), MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, master_comm, &status);

		
// 		printf("received:\n");
// 		printBoard(stack[stackPtr], N); // FIXME remove print
		
		switch (status.MPI_TAG) {
			case SOLVED:
				for (p = 1; p < totalProcesses; p++)
					MPI_Send(&(stack[stackPtr][0][0]), size(N), MPI_INT, p, SOLVED, master_comm);
				solved = 1;
				break;
				
			case REQUEST:
				if (stackPtr) {
// 				printf("request %d\n", size(N));
					stackPtr--;
					MPI_Send(&(stack[stackPtr])[0][0], size(N), MPI_INT, status.MPI_SOURCE, NEW_NODE, master_comm);
					
				} else {
					MPI_Send(&(stack[stackPtr])[0][0], size(N), MPI_INT, status.MPI_SOURCE, TRY_AGAIN, master_comm);
				}
				
// 				printf("sent:\n");
// 				printBoard(stack[stackPtr], N); // FIXME remove print
				
				break;
				
			case NEW_NODE:
// 				MPI_Recv(&(stack[stackPtr][0][0]), size(N), MPI_INT, status.MPI_SOURCE, NEW_NODE, master_comm, &status);
				stackPtr++;
				break;
		}
		
		
// 		getchar();
	}

	
	time = omp_get_wtime() -time;
	printf("solved: %d\nTime: %f seconds\n", isSolved(stack[stackPtr], N), time);
	printBoard(stack[stackPtr], N);
	
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
	
	
	MPI_Comm_rank(new_comm, &processID);
	
	// get block size and compute board size
	MPI_Bcast(&L, 1, MPI_INT, 0, MPI_COMM_WORLD);
	N = L*L;
	
	board = Board(N);
	stack = Stack(N, N);

	while(!solved) {
		MPI_Send(&(stack[stackPtr][0][0]), size(N), MPI_INT, 0, REQUEST, master_comm);
		MPI_Recv(&board[0][0], size(N), MPI_INT, 0, MPI_ANY_TAG, master_comm, &status);
		
// 		printf("received:\n");
// 		printBoard(board, N); // FIXME remove print
		
		switch (status.MPI_TAG) {
			case TRY_AGAIN:
				break;
				
			case SOLVED:
				solved = 1;
				break;
				
			case NEW_NODE:
				stackPtr = expandNode(board, L, N, stack, N);
				
// 				printStack(stack, N, stackPtr);
				
				while(stackPtr) {
					stackPtr--;
					if (isSolved(stack[stackPtr], N))
						MPI_Send(&(stack[stackPtr][0][0]), size(N), MPI_INT, 0, SOLVED, master_comm);
					else
						MPI_Send(&(stack[stackPtr][0][0]), size(N), MPI_INT, 0, NEW_NODE, master_comm);
				}
				break;
		}
		
		
// 		MPI_Send(&(stack[stackPtr][0][0]), size(N), MPI_INT, 0, SOLVED, master_comm);
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
	int **board;
	
	board = (int**) malloc(SIZE * sizeof(int*));
	for (int i = 0; i < SIZE; i++) 
		board[i] = (int*) malloc(SIZE * sizeof(int));
	
	return board;
}

int ***Stack(int BOARD_SIZE, int DEPTH) {
	int ***stack;
	
	stack = (int***) malloc(DEPTH * sizeof(int**));
	for (int t = 0; t < DEPTH; t++) {
		stack[t] = (int**) malloc(BOARD_SIZE * sizeof(int*));
		for (int i = 0; i < BOARD_SIZE; i++)
			stack[t][i] = (int*) malloc(BOARD_SIZE * sizeof(int));
	}
	
	return stack;
}

void freeBoard(int **board, int SIZE) {
	for (int i = 0; i < SIZE; i++)
		free(board[i]);  
	
	free(board);
}

void freeStack(int ***stack, int BOARD_SIZE, int DEPTH) {
	for (int t = 0; t < DEPTH; t++) {
		for (int i = 0; i < BOARD_SIZE; i++)
			free(stack[t][i]);
		
		free(stack[t]);
	}
	
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
	for (int i = SIZE-1; i > -1; i--)
		for (int j = SIZE-1; j > -1; j--)
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
	char buffer[MAX_LINE_SIZE];
	
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
	
	printf("%s</stack>\n\n", buffer);
}
