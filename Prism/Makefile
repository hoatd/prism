OBJS = main.o prism.o prism_common.o prism_simple.o prism_dataset.o prism_encoding.o prism_extensions.o prism_tables.o
CC = g++
flags = -O2 -Wall

prism : $(OBJS) Makefile
	$(CC) $(flags) $(OBJS) -o prism

main.o : main.cpp prism_tables.h prism.h
	$(CC) -c $(flags) main.cpp

prism.o : prism.cpp prism_tables.h prism.h
	$(CC) -c $(flags) prism.cpp

prism_common.o : prism_common.cpp prism_tables.h prism.h
	$(CC) -c $(flags) prism_common.cpp

prism_simple.o : prism_simple.cpp prism_tables.h prism.h
	$(CC) -c $(flags) prism_simple.cpp

prism_dataset.o : prism_dataset.cpp prism_tables.h prism.h
	$(CC) -c $(flags) prism_dataset.cpp

prism_encoding.o : prism_encoding.cpp prism_tables.h prism.h
	$(CC) -c $(flags) prism_encoding.cpp

prism_extensions.o : prism.cpp prism_tables.h prism.h
	$(CC) -c $(flags) prism_extensions.cpp

prism_tables.o : prism_tables.cpp prism_tables.h prism.h
	$(CC) -c $(flags) prism_tables.cpp

clean :
	rm -f $(OBJS) prism

