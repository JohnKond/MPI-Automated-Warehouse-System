all:
	mpicc -lm -o main main.c

clean:
	rm main
