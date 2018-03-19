	
/************************************************************************
 * 																		*
 * Parallel and Distributed Computing 									*
 * project part 1														*
 * 																		*
 * group 4, Alameda														*
 * 																		*
 * 75657 Paulo Gouveia													*
 * ##### ## ##															*
 * ##### ## ##															*
 * 																		*
 * Serial implementation												*
 * 																		*
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 8 spaces, 9 cells, '\0', '\n', possible '\r' on windows
unsigned int MAX_LINE_SIZE = 20; 

int charToInt(char c) {
	return c - '0';
}

void printBoard(int ***board, int N) {
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			printf("%d ", board[i][j][0]);
		}
		printf("\n");
	}
// 	printf("\n");
}


int solved(int ***board, int N) {
	// FIXME
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			if (board[i][j][0] == 0)
				return 0;
			
	return 1;
}



int** getBestCandidate(int ***board) {
	// FIXME
	return board[0][0];
}


void checkBlockLineColumn() {
	// FIXME
	
	return;
}


void uniqueCanditate() {
	// FIXME
	
	return;
}


void lineRemoval() {
	// FIXME
	
	return;
}


void blockRemoval() {
	// FIXME
	
	return;
}


void nakedSubset() {
	// FIXME
	
	return;
}


void hiddenSubset() {
	// FIXME
	
	return;
}


void xWing() {
	// FIXME
	
	return;
}


void swordfish() {
	// FIXME
	
	return;
}




int main(int argc, char *argv[]) {
	
	FILE *file;
	int l, n;
	int value;
	int*** board;
	char line[MAX_LINE_SIZE];
	char* cell;
	
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
		sscanf(line,"%d", &l);
		n = l * l;
// 		printf("l = %d, n = %d\n", l, n);
		
		board = (int***) malloc(n * sizeof(int**));
		for (int i = 0; i < n; i++) {
			board[i] = (int**) malloc(n * sizeof(int*));
			for (int j = 0; j < n; j++) {
				board[i][j] = (int*) malloc(n * sizeof(int));
			}
		}
		
		
		// initialize the board
		for (int i = 0; i < n ; i++) {
			if (fgets(line, MAX_LINE_SIZE, file) != NULL) {
				
				cell = strtok(line, " ");
				for (int j = 0; j < n; j++) {
					value = charToInt(*cell);
						
					if (value > 0)
						board[i][j][0] = value;
					
					cell = strtok(NULL, " ");
				}
				
			} else {
				printf("ill formed file '%s'\n", argv[1]);
				exit(3);
			}
		}
		
	    fclose(file);
	}
	
	
	// <FIXME> solve the thing here
	
	
	
	// </FIXME>
	
	
	// display solution
	printBoard(board, n);
	
	// free the memory
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			free(board[i][j]);
		}
		free(board[i]);
	}     
	free(board);

	
	return 0;
}

