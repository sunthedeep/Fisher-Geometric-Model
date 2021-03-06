CC=g++
CFLAGS=-lgsl -lgslcblas
DEPS = src/allele.h src/environment.h src/modelFunctions.h src/population.h src/randomv.h src/runningStat.h 

a:	src/tester.cpp src/allele.cpp src/environment.cpp src/modelFunctions.cpp src/population.cpp src/randomv.cpp src/runningStat.cpp src/stability.cpp
	$(CC) -O3 -o bin/FGM src/tester.cpp src/allele.cpp src/environment.cpp src/modelFunctions.cpp src/population.cpp src/randomv.cpp src/runningStat.cpp src/stability.cpp $(CFLAGS)
stability: src/computeStability.cpp src/stability.cpp
	$(CC) -O3 -o bin/stability src/computeStability.cpp src/stability.cpp $(CFLAGS)
clean: 
	rm -f bin/FGM bin/stability
