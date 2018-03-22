	
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

// 8 spaces, 9 cells, '\0', '\n', possible '\r' on windows
unsigned int MAX_LINE_SIZE = 20; 

typedef struct {
	int *candidates;
	int x, y;
	int checked;
} position;

void Position(position *p, int size, int x, int y) {
// 	position *p = malloc(sizeof(position));
	p->candidates = malloc(size * sizeof(int));
	p->x = x;
	p->y = y;
	p->checked = 0;
}

void printCell(position* p, int size) {
	char* checked = (p->checked == 0)? "True " : "False";
	
	printf("(%d,%d) %s [", p->x, p->y, checked);
	for (int t = 0; t < size; t++)
		printf(" %d", p->candidates[t]);
	
	printf(" ]\n");
}



int validatePosition(position p, int test);
int* arrayNAND(int* a1, int* a2);

int charToInt(char c) {
	return c - '0';
}

void printBoard(position **board, int N) {
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			printf("%d ", board[i][j].checked);
		}
		printf("\n");
	}
// 	printf("\n");
}

//Returns an the block index between 0 and l-1
//n received for arg validation
position getBlock(position cellPosition, int l, int n){
	position blockPosition;

	/*if(!validatePosition(cellPosition,n))
		return -1; <-- what do i return here */

	blockPosition.x = cellPosition.x / l;
	blockPosition.y = cellPosition.y / l;

	return blockPosition;
}

//returns the block's top left position, with this we can easily obtain
//the other positions of the block
position getBlockTopLeft(position blockPosition, int l){
	position topLeft;

	/*if(!validatePosition(blockPosition,l))
		return -1; <-- what do i return here */

	topLeft.x = blockPosition.x * l;
	topLeft.y = blockPosition.y * l;

	return topLeft;
}

int validatePosition(position p, int test){ return (p.x < 0 || p.x >= test || p.y < 0 || p.y >= test);}

int solved(int ***board, int N) {
	// FIXME
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			if (board[i][j][0] == 0)
				return 0;
			
	return 1;
}

/*

int** getBestCandidate(int ***board) {
	// FIXME
	return board[0][0];
}
*/

void checkBlockLineColumn(int*** board,int l,int n, position pos) {
	int i, removables[n];
	position blockPos = getBlockTopLeft(getBlock(pos,l,n),l);

	//Gather candidates to remove in row and column
	for(i = 0; i < n;i++){
		
		// check in row
		if(board[i][pos.y][0])
			removables[board[i][pos.y][0]] = board[i][pos.y][0];
		
		// check in column
		if(board[pos.x][i][0] != 0)
			removables[board[pos.x][i][0]] = board[pos.x][i][0];
		
		// check in block
		if(board[blockPos.x + (i % n)][blockPos.y + ((int) (i/n))][0])
			removables[board[blockPos.x + (i % n)][blockPos.y + ((int) (i/n))][0]] = board[blockPos.x + (i % n)][blockPos.y + ((int) (i/n))][0];
	}
	
	board[pos.x][pos.y] = arrayNAND(board[pos.x][pos.y], removables);
	
	return;
}

int* arrayNAND(int* a1, int* a2){
	// FIXME
	return a1;
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
	position** board;
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
		
		board = (position**) malloc(n * sizeof(position*));
		for (int i = 0; i < n; i++) {
			board[i] = (position*) malloc(n * sizeof(position));
			for (int j = 0; j < n; j++) {
				Position(&board[i][j], n, i, j);
			}
		}
		
		
		// initialize the board
		for (int i = 0; i < n ; i++) {
			if (fgets(line, MAX_LINE_SIZE, file) != NULL) {
				
				cell = strtok(line, " ");
				for (int j = 0; j < n; j++) {
					value = charToInt(*cell);
						
					if (value > 0)
						board[i][j].checked = value;
						
					else
						for (int t = 1; t <= n; t++)
							board[i][j].candidates[t-1] = t;
					
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
	
	

	// <scrapbook>

	// </scrapbook>

	// display solution
	printBoard(board, n);
	
	// free the memory
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			free(board[i][j].candidates);
		}
		free(board[i]);
	}     
	free(board);

	
	return 0;
}

