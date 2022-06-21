#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

static __attribute__((__noinline__)) int do_increment(int n) {
	return n + 2;
}

static __attribute__((__noinline__)) int gen_jit_function(void *func_addr, int *ptr, int n) {
	int i, j;
	int (*func_ptr)(int) = func_addr;

	//for (i = 0; i < 1024 * 1024 * 16; i++) {
	for (i = 0; i < 1024 * 8; i++) {
		if (i % 2 == 1) {
			*ptr = *ptr + 1;
		} else {
			*ptr = *ptr - 1;
		}
		if ((i % (1024 * 1024)) == 0) {
			*ptr = *ptr + 0x10000;
			for (j = 0; j < 1024; j++)
				n = (*func_ptr)(n);
		}
	}

	return n;
}

int main() {
	long pagesize = sysconf(_SC_PAGESIZE);
	void *addr    = mmap((void*)1024, pagesize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_JIT | MAP_PRIVATE, -1, 0);
	int         j = 0, n = 1, m = 0;
	int *n_p, *n_p2;
	int (*func_ptr1)(int) = addr;
	int (*func_ptr2)(void *, int *, int) = addr + 3968;

	if (addr == MAP_FAILED) {
		printf("Failed to map a page!: %ld\n", pagesize);
		return -1;
	}

	printf("Mapped JIT page is at %lx of size %ld\n", (unsigned long)addr, pagesize);

	n = do_increment(n);

	printf("After increment, n is %d\n", n);

	(void)memcpy(addr, &do_increment, 0x20);
	(void)memcpy(addr + 3968, &gen_jit_function, 0x75);

	printf("Address of do_increment is 0x%lx\n", (unsigned long)&do_increment);

	n_p = (int *)addr;
	n_p2 = n_p + 1023;
	n_p += 1;

	for (j = 0; j < 256; j++) {
		*n_p = 0x5d02478d;
		m = (*func_ptr2)(func_ptr1, n_p2, m);
	}

	printf("After new increment, m is %d\n", m);

	return 0;
}
