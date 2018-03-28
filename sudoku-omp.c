
	
/************************************************************************
* 																		*
* Parallel and Distributed Computing 									*
* project part 1														*
* 																		*
* group 4, Alameda														*
* 																		*
* 75657 Paulo Gouveia													*
* 76213 Gonçalo Lopes													*
* ##### ## ##															*
* 																		*
* Serial implementation												*
* 																		*
************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include<omp.h>

// 9 spaces, 16 cells (2 digits), '\n', '\0', possible '\r'
#define MAX_LINE_SIZE 44
#define MAX_STACK_SIZE 44


/***********************************************************************************/

typedef struct puzzle puzzle;
typedef struct stack stack;
typedef struct element element;


void push(stack stack, element* element);

puzzle* Puzzle(int modSize);
puzzle* getPuzzleFromFile(char *inputFile);
void freePuzzle(puzzle *board);

void printBoard(puzzle *board);

int valid(puzzle* board, int row, int column, int number);

int recursiveSolve(puzzle* board, int row, int column);
int iterativeSolve(puzzle* board);


/***********************************************************************************/

int main(int argc, char *argv[]) {

	puzzle *board;
	
	if(argc < 2){
		printf("missing argument.\nusage: sudoku-serial <filename>\n");
		exit(1);
	}
	
	board = getPuzzleFromFile(argv[1]);
	
	
	
	if (recursiveSolve(board, 0, 0)) {
		printBoard(board);
		
	} else {
		printf("No solution.\n");
	}
	
	
	freePuzzle(board);
	
	return 0;
}


/***********************************************************************************/


struct puzzle {
	int L, N;
	int **table;
};

struct element {
	int x, y;
	int value;
	element* next;
};

struct stack {
	element* first;
	element* last;
};


/***********************************************************************************/

void push(stack stack, element* element) {
	
}


/***********************************************************************************/

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


/***********************************************************************************/

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
// 	printf("\n");
}


/***********************************************************************************/

int valid(puzzle* board, int row, int column, int number) {
		
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


/***********************************************************************************/

int recursiveSolve(puzzle* board, int row, int column) {
	
	// if not reached the end of the board
	if (row < board->N && column < board->N) {
		
		if (board->table[row][column]) {
			if (column+1 < board->N) {
				return recursiveSolve(board, row, column+1);
				
			} else if (row+1 < board->N) {
				return recursiveSolve(board, row+1, 0);
				
			} else
				return 1;
			
		} else {
			for(int i = 0; i < board->N; ++i) {
				if (valid(board, row, column, i+1)) {
					board->table[row][column] = i+1;
					if (column+1 < board->N) {
						if (recursiveSolve(board, row, column+1))
							return 1;
						else
							board->table[row][column] = 0;
						
					} else if (row+1 < board->N) {
						if (recursiveSolve(board, row+1, 0))
							return 1;
						else
							board->table[row][column] = 0;
						
					} else {
						return 1;
					}
				}
			}
		}
		
		return 0;
		
	} else {
		return 1;
	}
}


int iterativeSolve(puzzle* board) {
	
	
// 	// if not reached the end of the board
// 	if (row < board->N && column < board->N) {
// 		
// 		if (board->table[row][column] != 0) {
// 			if (column+1 < board->N) {
// 				return solve(board, row, column+1);
// 				
// 			} else if (row+1 < board->N) {
// 				return solve(board, row+1, 0);
// 				
// 			} else
// 				return 1;
// 			
// 		} else {
// 			for(int i = 0; i < board->N; ++i) {
// 				if (valid(board, row, column, i+1)) {
// 					board->table[row][column] = i+1;
// 					if (column+1 < board->N) {
// 						if (solve(board, row, column+1))
// 							return 1;
// 						else
// 							board->table[row][column] = 0;
// 						
// 					} else if (row+1 < board->N) {
// 						if (solve(board, row+1, 0))
// 							return 1;
// 						else
// 							board->table[row][column] = 0;
// 						
// 					} else {
// 						return 1;
// 					}
// 				}
// 			}
// 		}
// 		
// 		return 0;
// 		
// 	} else {
// 		return 1;
// 	}
	
	return 0;
}

