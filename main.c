#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

static __attribute__((__noinline__)) int do_increment(int n) {
	return n + 2;
}

int main() {
	long pagesize = sysconf(_SC_PAGESIZE);
	void *addr    = mmap((void*)1024, pagesize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_JIT | MAP_PRIVATE, -1, 0);
	int i = 0, j = 0, k = 0, n = 1, m = 0;
	volatile int *n_p, *n_p2;
	int (*func_ptr)(int) = addr;

	if (addr == MAP_FAILED) {
		printf("Failed to map a page!: %ld\n", pagesize);
		return -1;
	}

	printf("Mapped JIT page is at %lx of size %ld\n", (unsigned long)addr, pagesize);

	n = do_increment(n);

	printf("After increment, n is %d\n", n);

	(void)memcpy(addr, &do_increment, 0x20);

	printf("Address of do_increment is 0x%lx\n", (unsigned long)&do_increment);

	n_p = (int *)addr;
	n_p += 1;
	n_p2 = n_p + 8;

	for (j = 0; j < 256; j++) {
		*n_p = 0x5d02478d;
		for (i = 0; i < 1024 * 1024 * 16; i++) {
			if (i % 2 == 1) {
				*n_p2 = *n_p2 + 1;
			} else {
				*n_p2 = *n_p2 - 1;
			}
			if ((i % (1024 * 1024)) == 0) {
				*n_p = *n_p + 0x10000;
				m = (*func_ptr)(m);
				k++;
			}
		}
	}

	printf("After new increment, m is %d\n", m);
	printf("Total number of calls = %d\n", k);

	return 0;
}
