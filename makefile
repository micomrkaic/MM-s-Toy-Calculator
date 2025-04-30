CC = gcc
CFLAGS = -g -Wall -Wextra -Werror -std=c11 
LIBS = -lgsl -lgslcblas -lm -lreadline -lhistory

OBJS = main.o lexer.o stack.o string_fun.o math_fun.o binary_fun.o unary_fun.o eval_fun.o stat_fun.o registers.o poly_fun.o

all: calculator

calculator: $(OBJS)
	$(CC) $(CFLAGS) -o calculator $(OBJS) $(LIBS)

main.o: main.c lexer.h stack.h
lexer.o: lexer.c lexer.h stack.h
stack.o: stack.c stack.h
string_fun.o: string_fun.c string_fun.h
math_fun.o: math_fun.c math_fun.h
binary_fun.o: binary_fun.c binary_fun.h
unry_fun.o: unary_fun.c unary_fun.h
eval_fun.o: eval_fun.c eval_fun.h
stat_fun.o: stat_fun.c stat_fun.h
poly_fun.o: poly_fun.c poly_fun.h
registers.o: registers.c registers.h

clean:
	rm -f *.o calculator
