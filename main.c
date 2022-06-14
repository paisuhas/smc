#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <libkern/OSCacheControl.h>

static __attribute__((__noinline__)) int do_increment(int n) {
	return n + 2;
}

int main() {
	long pagesize = sysconf(_SC_PAGESIZE);
	void *addr    = mmap((void*)1024, pagesize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_JIT | MAP_PRIVATE, -1, 0);
	int i = 0, j = 0, n = 1, m = 0;
	volatile int *n_p, *n_p2;
	int (*func_ptr)(int) = addr;
	void *addr2 = NULL;

	if (addr == MAP_FAILED) {
		printf("Failed to map a page!: %ld\n", pagesize);
		return -1;
	}

	printf("Mapped page for JIT code is at %lx\n", (unsigned long)addr);
	//pthread_jit_write_protect_np(0);

	n = do_increment(n);

	printf("After increment, n is %d\n", n);

	printf("Address of do_increment is 0x%lx\n", (unsigned long)&do_increment);

	(void)memcpy(addr, &do_increment, 0x20);

	n_p = (int *)addr;
	n_p += 1;
	n_p2 = n_p + 8;
	n = *n_p;
	printf("Before memcpy, n is %x\n", n);
	n += 0x10000;
	*n_p = n;

	printf("After memcpy, n is %x, n_p is %lx\n", n, (unsigned long)n_p);

	//pthread_jit_write_protect_np(1);
	//sys_icache_invalidate(addr, 0x20);

	n = (*func_ptr)(2);
	//pthread_jit_write_protect_np(0);

	//printf("After new increment, n is %d, n_p is %lx\n", n, (unsigned long)n_p);

	//for (i = 0; i < 1024 * 1024 * 1024; i++) {
	//for (i = 0; i < 8; i++) {
	for (j = 0; j < 1024 * 1024 * 16; j++) {
		//pthread_jit_write_protect_np(0);
		n_p = (int *)addr;
		n_p += 1;
		n_p2 = n_p + 8;
		func_ptr = addr;
		*n_p = 0x5d02478d;
		//pthread_jit_write_protect_np(1);
		//sys_icache_invalidate(addr, 0x20);
		for (i = 0; i < 1024 * 1024 * 16; i++) {
			if (i % 2 == 1) {
				*n_p2 = *n_p2 + 1;
			} else {
				*n_p2 = *n_p2 - 1;
			}
			/*
			   if (i % 512 == 0) {
			   addr2 = mmap((void*)2048, pagesize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_JIT | MAP_PRIVATE | MAP_FIXED, -1, 0);
			   }
			 */
			if ((i % (1024 * 1024)) == 0) {
				*n_p = *n_p + 0x10000;
				/*
				   }
				   if (i % (1024) == 0) {
				 */

				m = (*func_ptr)(m);

			}
		}
	}
	printf("After loop, n is %d\n", n);
	m = (*func_ptr)(m);

	printf("After new increment, m is %d, addr2 is %lx\n", m, (unsigned long)addr2);

	do {
	} while (1 > 0);

	return 0;
}
