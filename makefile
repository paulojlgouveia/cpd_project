
FLAGS=-fopenmp -ansi -pedantic -Wall -Wno-unused-result -O3 -lm -std=c99

in=input/
out=output/

all: compile run

compile: clean serial omp mpi


clean:
# 	@clear
	@rm -f *.o
	@rm -f sudoku-serial sudoku-omp sudoku-mpi
	@rm -f output/*
	@rm -f *.zip
	@rm -f massif.out.*

	
serial:
	@gcc $(FLAGS) sudoku-serial.c -o sudoku-serial
	
omp:
	@gcc $(FLAGS) sudoku-omp.c -o sudoku-omp
	
mpi:
# 	@gcc $(FLAGS) sudoku-mpi.c -o sudoku-mpi
	
	
run:
	./sudoku-serial input/ex1.in
# 	./sudoku-omp input/ex1.in
# 	./sudoku-mpi input/ex1.in
	

valgrind: compile
	valgrind -v --leak-check=full ./sudoku-serial
# 	valgrind -v --leak-check=full ./sudoku-omp
# 	valgrind -v --leak-check=full ./sudoku-mpi
	
	
submission:
	zip omp_g04A.zip sudoku-serial.c sudoku-omp.c report-omp.pdf
	
submission2:
	zip mpi_g04A.zip sudoku-serial.c sudoku-mpi.c report-mpi.pdf

