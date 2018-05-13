#!/bin/bash

bold='\e[1m'
italic='\e[3m'
underline='\e[4m'
red='\e[31m'
green='\e[32m'
end='\e[0m'


BINARIES="sudoku-serial sudoku-omp sudoku-mpi"
FILES=$(find expected/*.out)
INPUTS=$(find input/*.in)


for bin in ${BINARIES}
do
# start=`date +%s`
	for input in ${INPUTS}
	do
		output=${input//'input/'/'output/'}
		output=${output//'.in'/'.out'}
		
		$(./${bin} ${input} > ${output})
	done
	
	
	INPUTS=${FILES//'expected/'/'input/'}
	INPUTS=${INPUTS//'.out'/'.in'}
	
	total=0
	correct=0

	for expected in ${FILES}
	do
		total=$((total+1))
		outputed=${expected//'expected/'/'output/'}
		name=${expected//'expected/'/''}
		
		
		if [ ${#name} -le 8 ]; then
			name="${name}    "
		fi
		
		
		#ignore blank spaces at the end of line
		diff -Z ${expected} ${outputed} > /dev/null
		
		if [ $? == 0 ]; then
			correct=$((correct+1))
			echo -e "${name}\t-> ${green}Passed${end}"
			
		elif [ $? == 1 ]; then
			echo -e "${name}\t-> ${red}Failed${end}"
			
		else
			echo -e "${name}\t-> ${underline}Error${end}"
		fi
			
	done
	
# 	end=`date +%s`
# 	runtime=$((end-start))
	
	echo -e "${bold}${italic}${bin} : ${correct}/${total} tests.${end}\n"
done

