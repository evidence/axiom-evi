PROGRAM=nbody
KERNEL=kernel
PREFIX=.

CC = gcc
KC = nvcc

CFLAGS = -g #-k --ompss --instrument --cluster --ld=mpicc
CFLAGS_P =
CFLAGS_I = 
CFLAGS_D =

NVCFLAGS = -O3

LIBS  = -lm -lrt
INCS  = -I.

EXTRA = -O3 -Wall -Wno-unused 
OBJECTS=

all: $(PROGRAM) gen-solar-system

$(PROGRAM): $(PROGRAM).o $(KERNEL).o $(OBJECTS)
	$(CC) $(CFLAGS) $(EXTRA) -o $@ $< $(KERNEL).o $(OBJECTS) $(LIBS)

$(PROGRAM).o: $(PROGRAM).c $(PROGRAM).h $(KERNEL).h
	$(CC) $(CFLAGS) $(EXTRA) $(INCS) -o $@ -c $<

$(KERNEL).o: $(KERNEL).c $(PROGRAM).h $(KERNEL).h
	$(CC) $(CFLAGS) $(EXTRA) $(INCS) -o $@ -c $< 

$(KERNEL).x: $(KERNEL).cu $(KERNEL).h
	$(KC) $(INCS) -o $@ -c $< 

gen-solar-system: gen-solar-system.o
	$(CC) -O -o gen-solar-system gen-solar-system.o -lm

gen-solar-system.o: gen-solar-system.c
	$(CC) -O -c gen-solar-system.c

.c.o:
	$(CC) --no-openmp $(EXTRA) -c $<

check: $(PROGRAM)
	@NX_SMP_WORKERS=1 ./$(PROGRAM) 4096 10 1
	@NX_SMP_WORKERS=2 ./$(PROGRAM) 4096 10 1

clean:
	rm -f $(PROGRAM) 
	rm -f *.o 
	rm -f $(CC)_* 
	rm -f *~ 
	rm -f *.out

