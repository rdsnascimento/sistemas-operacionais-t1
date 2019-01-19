CC=gcc

CFLAGS=-Wextra -Werror -g -std=c11 -pthread
LDFLAGS=-lm

all: grade gradet

aluno: roteamentot.c aluno.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o aluno roteamentot.c aluno.c -lm

test: roteamento.c test.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o test roteamento.c test.c -lm 

testt: roteamentot.c test.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o testt roteamentot.c test.c -lm

teste_aluno: aluno
	./aluno

grade: test
	./test

gradet: testt
	./testt

clean:
	rm -rf *.o test testt aluno
