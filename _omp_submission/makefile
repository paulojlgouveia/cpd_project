
FLAGS=-fopenmp -ansi -pedantic -Wall -Wno-unused-result -O3 -lm -std=c99

out=""


all: compile

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
	
	
run: compile
	@echo 'serial'; for f in input/*.in; do echo '\n'$$f; time ./sudoku-serial $$f; done
	@echo 'omp_2'; for f in input/*.in; do echo '\n'$$f; time ./sudoku-omp $$f 2; done
	@echo 'omp_4'; for f in input/*.in; do echo '\n'$$f; time ./sudoku-omp $$f 4; done
	@echo 'omp_8'; for f in input/*.in; do echo '\n'$$f; time ./sudoku-omp $$f 8; done
# 	@for file in input/*.in; do echo $$file; ./sudoku-mpi $$file; done
	@echo


test: compile
	@echo
	@./tests.sh


valgrind: compile
	valgrind -v --leak-check=full ./sudoku-serial input/ex2.in
	valgrind -v --leak-check=full ./sudoku-omp input/4x4.in 4
# 	valgrind -v --leak-check=full ./sudoku-mpi input/ex2.in
	
	
submission:
	zip omp_g04A.zip sudoku-serial.c sudoku-omp.c report-omp.pdf makefile
	
submission2:
	zip mpi_g04A.zip sudoku-serial.c sudoku-mpi.c report-mpi.pdf

