smc: main.c
	clang -Wall -O2 main.c -o x86_64_smc -arch x86_64

clean: x86_64_smc
	rm x86_64_smc
