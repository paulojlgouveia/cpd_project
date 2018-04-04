
/****************************************************************************
* 																			*
* Parallel and Distributed Computing 										*
* project part 1															*
* 																			*
* group 4, Alameda															*
* 																			*
* 75657 Paulo Gouveia														*
* 76213 Gonçalo Lopes														*
* 70969 Mario Reis															*
* 																			*
* OpenMP implementation														*
* 																			*
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

// 81 spaces, 162 cells (2 digits), '\n', '\0', possible '\r'
#define MAX_LINE_SIZE 246
// 81^2 + (81*80)/2 , board size + candidates per cell
#define MAX_STACK_SIZE 9801


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
void freePuzzle(puzzle *board);

void printSolution(puzzle *board);
void printBoard(puzzle *board);
void printStack(element* stack, int from, int to);

int valid(puzzle* board, int row, int column, int number);
int solved(puzzle* board);

int iterativeSolve(puzzle* board, int threadsN);


/****************************************************************************/

int main(int argc, char *argv[]) {

	puzzle *board;
	
	if(argc < 3){
		printf("missing argument.\nusage: sudoku-serial <filename> <thread_count>\n");
		exit(1);
	}
	
	board = getPuzzleFromFile(argv[1]);
	
	omp_set_dynamic(0);
	omp_set_num_threads(atoi(argv[2]));
	
	if (iterativeSolve(board, atoi(argv[2]))) {
		printBoard(board);
		
	} else {
		printf("No solution.\n");
	}
	
	
	freePuzzle(board);
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


/****************************************************************************/

void printSolution(puzzle *board) {
	for (int i = 0; i < board->N; i++) {
		for (int j = 0; j < board->N; j++) {
			printf("%d ", board->table[i][j]);
		}
		printf("\n");
	}
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

void printStack(element* stack, int from, int toInclusive) {
	for (int t = from; t <= toInclusive; t++)
		printf("%4d: (%d,%d) %2d\n", t, stack[t].x, stack[t].y, stack[t].value);
	
	printf("\n");
}


/****************************************************************************/

/*
int isValid(puzzle* board, int row, int column, int number) {
		
    int rowStart = (row/board->L) * board->L;
    int colStart = (column/board->L) * board->L;

    for(int i = 0; i < board->N; ++i) {
        if (board->table[row][i] == number)
			return 0;
		
        if (board->table[i][column] == number)
			return 0;
		
		int iFromBlock = rowStart + (i % board->L);
		int jFromBlock = colStart + (i / board->L);
		
        if (board->table[iFromBlock][jFromBlock] == number)
			return 0;
    }
    
    return 1;
}
*/


int isValid(puzzle* board, int row, int column, int number) {
		
    int rowStart = (row/board->L) * board->L;
    int colStart = (column/board->L) * board->L;
    int flag = 1;    	
    int iFromBlock;
    int jFromBlock;

    #pragma omp parallel
    for(int i = 0; i < board->N; ++i) {
        
        if (board->table[row][i] == number) {
            #pragma omp critical
            {
                flag = 0;
            }
            #pragma omp cancel parallel
        }
        
	if (board->table[i][column] == number) {
            #pragma omp critical
            {
                flag = 0;
            }
            #pragma omp cancel parallel
        }

        iFromBlock = rowStart + (i % board->L);
        jFromBlock = colStart + (i / board->L);
        
	if (board->table[iFromBlock][jFromBlock] == number) {
            #pragma omp critical
            {
                flag = 0;
            }
            #pragma omp cancel parallel
        }
    }
    return flag;
}



int solved(puzzle* board) {
	for (int i = board->N-1; i > -1; i--)
		for (int j = board->N-1; j > -1; j--)
			if (board->table[i][j] == 0)
				return 0;
		
	return 1;
}


/****************************************************************************/

int iterativeSolve(puzzle* board, int threadsN) {
	
#pragma omp single
{
// 	int threadsN = omp_get_num_threads();
	
	element globalStack[MAX_STACK_SIZE][MAX_STACK_SIZE];
	int globalStackPtr = -1;
	
	puzzle* boards[threadsN];
	element pathStack[threadsN][MAX_STACK_SIZE];
// 	int pathStackPtr[threadsN];
// 	int progress[threadsN];
	int pathStackPtr = -1;
	int progress = 0;
	
	int i, j, value;
	
	
	for(int t = 0; t < threadsN; t++) {
		boards[t] = Puzzle(board->L);
		memcpy(boards[t], board, sizeof(puzzle));
		memcpy(boards[t]->table, board->table, sizeof(board->N * sizeof(int*)));
		
		for (int i = 0; i < board->N; i++)
			memcpy(boards[t]->table[i], board->table[i], sizeof(board->N * sizeof(int)));
		
// 		pathStackPtr[t] = -1;
// 		progress[t] = 0;
		
		printf("- %d /%d -\n", t, threadsN);	
		printBoard(boards[t]);
	}
	
	
	
#pragma omp parallel firstprivate(pathStackPtr, progress) private(i, j, value)
{
	int tid = omp_get_thread_num();
	
// 	for (i = 0; i < boards[tid]->N; i++) {
// 		for (j = 0; j < boards[tid]->N; j++) {
// 			
// 			// if empty cell
// 			if (!boards[tid]->table[i][j]) {
// 				
// 				for (value = boards[tid]->N; value > 0; value--) {
// 					// add candidates to pathStack
// 					if (isValid(boards[tid], i, j, value)) {
// 						pathStackPtr++;
// 						pathStack[tid][pathStackPtr].x = i;
// 						pathStack[tid][pathStackPtr].y = j;
// 						pathStack[tid][pathStackPtr].value = value;
// 						pathStack[tid][pathStackPtr].expanded = 0;
// 						
// 						progress = 1;
// 					}
// 				}
// 				
// 				// if no candidates added, revert last branch of changes
// 				if (!progress) {
// 					while (pathStack[tid][pathStackPtr].expanded) {
// 						i = pathStack[tid][pathStackPtr].x;
// 						j = pathStack[tid][pathStackPtr].y;
// 						boards[tid]->table[i][j] = 0;
// 						pathStackPtr--;
// 					}
// 				}
// 				
// // 				printf("%d - %d,%d\n", omp_get_thread_num(), i, j);
// 				
// 				if (pathStackPtr >= 0) {
// 					// pick a candidate for the next iteration
// 					i = pathStack[tid][pathStackPtr].x;
// 					j = pathStack[tid][pathStackPtr].y;
// 					boards[tid]->table[i][j] = pathStack[tid][pathStackPtr].value;
// 					pathStack[tid][pathStackPtr].expanded = 1;
// 					
// 					progress = 0;
// 					
// 				} else {
// 					// nothing left to try, there is no solution
// // 					return 0;
// 				}
// 			}
// 		}
// 	}

// 	printf("- %d -\n", tid);	
// 	printBoard(boards[tid]);
}
}
	return solved(board);
}

