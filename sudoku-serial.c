
	
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

// 9 spaces, 16 cells (2 digits), '\n', '\0', possible '\r'
unsigned int MAX_LINE_SIZE = 44; 


/***********************************************************************************/

typedef struct {
	int fixed;
	int value;
// 	int *candidates;
} cell;


typedef struct {
	int L, N;
	int **table;
} puzzle;


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
		
        if (board->table[rowStart + (i%board->L)][colStart + (i/board->L)] == number)
			return 0;
    }
    
    return 1;
}


int solve(puzzle* board, int row, int column) {
	
	if (row < board->N && column < board->N) {
		if (board->table[row][column] != 0) {
			if (column+1 < board->N) {
				return solve(board, row, column+1);
				
			} else if (row+1 < board->N) {
				return solve(board, row+1, 0);
				
			} else
				return 1;
			
		} else {
			for(int i = 0; i < board->N; ++i) {
				if (valid(board, row, column, i+1)) {
					board->table[row][column] = i+1;
					if (column+1 < board->N) {
						if (solve(board, row, column+1))
							return 1;
						else
							board->table[row][column] = 0;
						
					} else if((row+1)<board->N) {
						if (solve(board, row+1, 0))
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



/***********************************************************************************/


int main(int argc, char *argv[]) {
	
	FILE *file;
	int modSize;
	puzzle *board;
	char line[MAX_LINE_SIZE];
	char* inputCell;
	
	if(argc < 2){
		printf("missing argument.\nusage: sudoku-serial <filename>\n");
		exit(1);
	}
	
	// read initial board from the input file
	file = fopen(argv[1], "r");
	if (file == NULL) {	
		printf("failed to read input file: '%s'\n", argv[1]);
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
				printf("ill formed file '%s'\n", argv[1]);
				exit(3);
			}
		}
		
		fclose(file);
	}
	

// 	printBoard(board);
// 	printf("\n");
	
	if (solve(board, 0, 0)) {
		printBoard(board);
		
	} else {
		printf("No solution.\n");
	}
	
	
	freePuzzle(board);
	
	return 0;
}


/***********************************************************************************/


