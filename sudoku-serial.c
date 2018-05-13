
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
* Serial implementation														*
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

int recursiveSolve(puzzle* board, int row, int column);
int iterativeSolve(puzzle* board);


/****************************************************************************/

int main(int argc, char *argv[]) {

	puzzle *board;
	double time;

	if(argc < 2){
		printf("missing argument.\nusage: sudoku-serial <filename>\n");
		exit(1);
	}
	
	board = getPuzzleFromFile(argv[1]);
	
	time = omp_get_wtime();
// 	if (recursiveSolve(board, 0, 0)) {
	if (iterativeSolve(board)) {
		printBoard(board);
		
	} else {
		printf("No solution.\n");
	}
	
	time = omp_get_wtime() -time;
	printf("Time: %f seconds\n",time);
	
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

int solved(puzzle* board) {
	for (int i = board->N-1; i > -1; i--)
		for (int j = board->N-1; j > -1; j--)
			if (board->table[i][j] == 0)
				return 0;
		
	return 1;
}


/****************************************************************************/

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
				if (isValid(board, row, column, i+1)) {
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
// 	element stack[board->N * board->N * board->N];
	element stack[MAX_STACK_SIZE];
	int stackPtr = -1;
	int progress = 0;
	
	for (int i = 0; i < board->N; i++) {
		for (int j = 0; j < board->N; j++) {
			
			// if empty cell
			if (!board->table[i][j]) {
				for (int value = board->N; value > 0; value--) {
					// add candidates to stack
					if (isValid(board, i, j, value)) {
						stackPtr++;
						stack[stackPtr].x = i;
						stack[stackPtr].y = j;
						stack[stackPtr].value = value;
						stack[stackPtr].expanded = 0;
						
						progress = 1;
					}
				}
				
				// if no candidates added, revert last branch of changes
				if (!progress) {
					while (stack[stackPtr].expanded) {
						i = stack[stackPtr].x;
						j = stack[stackPtr].y;
						board->table[i][j] = 0;
						stackPtr--;
					}
				}
				
				if (stackPtr >= 0) {
					// pick a candidate for the next iteration
					i = stack[stackPtr].x;
					j = stack[stackPtr].y;
					board->table[i][j] = stack[stackPtr].value;
					stack[stackPtr].expanded = 1;
					
					progress = 0;
					
				} else {
					// nothing left to try, there is no solution
					return 0;
				}
			}
		}
	}
	
	return solved(board);
}

