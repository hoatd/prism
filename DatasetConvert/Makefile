OBJS = datasetconvert.o
CC = g++
flags = -O2 -Wall

datasetconvert : $(OBJS) Makefile
	$(CC) $(flags) $(OBJS) -o datasetconvert

datasetconvert.o : datasetconvert.cpp
	$(CC) -c $(flags) datasetconvert.cpp

clean :
	rm -f $(OBJS) datasetconvert

