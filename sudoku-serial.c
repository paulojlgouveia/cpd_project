
	
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

// 8 spaces, 9 cells, '\n', '\0', possible '\r' on windows
unsigned int MAX_LINE_SIZE = 20; 

/***********************************************************************************/

typedef struct {
	int x, y;
} position;

typedef struct {
	int *candidates;
	int x, y;
	int value;
} cell;

// typedef struct {
// 	cell *cells;
// } set;


/***********************************************************************************/


cell* EmptyCell(int candidatesSize, int x, int y);
cell* Cell(int value, int x, int y);
cell** Set(int size);
cell*** Board(int size);

void freeCell(cell* cell);
void freeSet(cell** set);
void freeBoard(cell*** board, int size);

void printCell(cell* cell, int candidatesSize);
void printSet(cell** set, int size);
void printBoard(cell*** board, int size);

int validCell(int x, int y, int bounds);
int contains(cell** set, int value, int bounds);
int uniqueCanditate(cell* cell, int x, int y, int candidates);
int solved(cell*** board, int size);

int charToInt(char c);
cell* getCell(cell*** board, int x, int y, int size);
void rowToSet(cell** set, cell*** board, int x, int y, int width);
void columnToSet(cell** set, cell*** board, int x, int y, int hight);
void blockToSet(cell** set, cell*** board, int x, int y, int width);


/***********************************************************************************/


int main(int argc, char *argv[]) {
	
	FILE *file;
	int l, n;
	int value;
	cell*** board;
	cell** set;
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
		sscanf(line,"%d", &l);
		n = l * l;
// 		printf("l = %d, n = %d\n", l, n);
		
		board = Board(n);
		set = Set(n);
		
		// initialize the board
		for (int i = 0; i < n ; i++) {
			if (fgets(line, MAX_LINE_SIZE, file) != NULL) {
				
				inputCell = strtok(line, " ");
				for (int j = 0; j < n; j++) {
					value = charToInt(*inputCell);
						
					if (value > 0)
						board[i][j] = Cell(value, i, j);
					else
						board[i][j] = EmptyCell(n, i, j);
					
					inputCell = strtok(NULL, " ");
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
// 	char* s = (solved(board, n) != 0)? "True " : "False";
// 	printf("%s\n", s);
	
	
	
// 	printf("\nrow example for (1,2):\n");
// 	rowToSet(set, board, 1, 2, n);
// 	printSet(set, n);	
// 	
// 	printf("\ncolumn example for (1,2):\n");
// 	columnToSet(set, board, 1, 2, n);
// 	printSet(set, n);	
// 	
// 	printf("\nblock example for (1,2):\n");
// 	blockToSet(set, board, 1, 2, l);
// 	printSet(set, n);
	
	
	freeBoard(board, n);
	freeSet(set);
	
	return 0;
}

/***********************************************************************************/

cell* EmptyCell(int candidatesSize, int x, int y) {
	cell* cell = malloc(sizeof(cell));
	cell->x = x;
	cell->y = y;
	cell->candidates = malloc(candidatesSize * sizeof(int));
	cell->value = 0;
	
	return cell;
}

cell* Cell(int value, int x, int y) {
	cell* cell = malloc(sizeof(cell));
	cell->x = x;
	cell->y = y;
	cell->candidates = NULL;
	cell->value = value;
	
	return cell;
}

cell** Set(int size) {
	return malloc(size * sizeof(cell*));
}

cell*** Board(int size) {
	cell*** board = (cell***) malloc(size * sizeof(cell**));
	for (int i = 0; i < size; i++) 
		board[i] = (cell**) malloc(size * sizeof(cell*));
	
	return board;
}


/***********************************************************************************/

void freeCell(cell* cell) {
	if (cell->candidates != NULL)
		free(cell->candidates);
	
	free(cell);
}

void freeSet(cell** set) {
	free(set);
}

void freeBoard(cell*** board, int size) {
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			freeCell(board[i][j]);
		}
		
		free(board[i]);
	}   
	
	free(board);
}


/***********************************************************************************/

void printCell(cell* cell, int candidatesSize) {
// 	char* checked = (p->checked == 0)? "True " : "False";
// 	printf("(%d,%d) %s [", p->x, p->y, checked);
	
	printf("(%d,%d) %d [", cell->x, cell->y, cell->value);
	
	if (cell->candidates != NULL)
		for (int t = 0; t < candidatesSize; t++)
			printf(" %d", cell->candidates[t]);
	
	printf(" ]\n");
}

void printSet(cell** set, int size) {
	for (int t = 0; t < size; t++)
		printCell(set[t], size);
	
// 	printf("\n");
}

void printBoard(cell*** board, int size) {
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			printf("%d ", board[i][j]->value);
		}
		printf("\n");
	}
// 	printf("\n");
}


/***********************************************************************************/

int validCell(int x, int y, int bounds) {
	return !(x < 0 || x >= bounds || y < 0 || y >= bounds);
}

int contains(cell** set, int value, int bounds) {
	for (int t = 0; t < bounds; t++) {
		if (set[t]->value == value)
			return 1;
	}
	
	return 0;
}

int uniqueCanditate(cell* cell, int x, int y, int candidates) {
	int value = 0;
	// returns on second candidate found instead of looping over all
	for (int t = 0; t < candidates; t++) {	
		if (cell->candidates[t] != 0) {
			if (value != 0)
				return 0;
			else
				value = cell->candidates[t];
		}
	}
	
	// if unique, update cell and return != than 0
	cell->value = value;
	return value;
}

int solved(cell*** board, int size) {
	for (int i = 0; i < size; i++)
		for (int j = 0; j < size; j++)
			if (board[i][j]->value == 0)
				return 0;
			
	return 1;
}


/***********************************************************************************/

int charToInt(char c) {
	return c - '0';
}

cell* getCell(cell*** board, int x, int y, int size) {
	if (validCell(x, y, size))
		return board[x][y];
	else
		return NULL;
}

void rowToSet(cell** set, cell*** board, int x, int y, int width) {
	for (int t = 0; t < width; t++)
		set[t] = board[x][t];
}

void columnToSet(cell** set, cell*** board, int x, int y, int hight) {
	for (int t = 0; t < hight; t++)
		set[t] = board[t][y];
}

void blockToSet(cell** set, cell*** board, int x, int y, int width) {
	int xStart =  width * (int) (x / width);
	int xEnd   = xStart + width;
	int yStart = width * (int) (y / width);
	int yEnd   = yStart + width;

	int t = 0;
	for (int i = xStart; i < xEnd; i++) {
		for (int j = yStart; j < yEnd; j++) {
			set[t] = board[i][j];
			t++;
		}
	}
}


/***********************************************************************************/






