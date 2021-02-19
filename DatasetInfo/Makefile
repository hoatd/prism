OBJS = datasetinfo.o
CC = g++
flags = -O2 -Wall

datasetinfo : $(OBJS) Makefile
	$(CC) $(flags) $(OBJS) -o datasetinfo

datasetinfo.o : datasetinfo.cpp
	$(CC) -c $(flags) datasetinfo.cpp

clean :
	rm -f $(OBJS) datasetinfo

