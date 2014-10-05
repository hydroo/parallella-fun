#include "prereqs.hpp"

#include <cstdlib>
#include <cstdio>

#define N 16
#define A(i,j) a_mat[j*N+i]
#define B(i,j) b_mat[j*N+i]
#define C(i,j) c_mat[j*N+i]

f32 a_mat[N*N] __attribute__ ((section (".data_bank1"))); // result matrix
f32 b_mat[N*N] __attribute__ ((section (".data_bank2"))); // result matrix
f32 c_mat[N*N] __attribute__ ((section (".data_bank3"))); // result matrix

static f32 sfrand();

static unsigned int mirand =1;

u32 e_test_init() {
	u32 *pass = (u32*) 0x24; // use user interrupt entry for now
	*pass = 0x00000000;      // initialize as "started"

	// resetting mask register
	__asm__ __volatile__ ("MOVTS IMASK, %0" : : "r" (0x0));

	// get the coreID using assembly
	register u32 coreid_in_reg asm("r0");
	__asm__ __volatile__ ("MOVFS %0, COREID" : : "r" (coreid_in_reg));
	coreid_in_reg = coreid_in_reg << 20;
	return coreid_in_reg;
}

int e_test_finish(int status) {
	u32 *pass = (u32*) 0x24; // overwrite the sync ivt entry on exit
	if (status == 1) {
		*pass = 0x12345678;
	} else {
		*pass = 0xDEADBEEF;
	}
	while(1);
}

void e_write_ack(u32* addr) {
	u32 probe_data;
	probe_data = *(addr);                // read old data
	probe_data = probe_data ^ 0xFFFFFFFF;// toggle old data
	(*(addr)) = probe_data;              // write new toggled data
	while(probe_data != *(addr)){        // keep reading until match is met
	}
}

int main(int argc, char** args) {
	int  i, j, k;
	f32  sum = 0.0f;
	int  status = 1;
	u32  coreID;

	// test init
	coreID = e_test_init();

	// fill input matrices with a constant
	for (i = 0; i < N; i += 1) {
		for (j = 0; j < N; j += 1) {
			A(i,j) = sfrand();
			B(i,j) = sfrand();
		}
	}

	// run matrix multiplication
	for (i = 0; i < N; i+= 1) {
		for (j = 0; j < N; j += 1) {
			C(i,j) = 0;
			for (k = 0; k < N; k += 1) {
				C(i,j) += A(i,k) * B(k,j);
			}
		}
	}

	// sum up the C matrix
	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			sum += C(i,j);
		}
	}

	// compare to expected result
	// printf("sum=%f\n",sum);

	if(sum != -9.114673f) { // testing for bit exact sum from Epiphany golden chip(s)
		status=0;         // fail
	}

	// finish test
	return e_test_finish(status);
}

// weird and cool random floating point generator from www.rgba.org/articles/sfrand/sfrand.htm
f32 sfrand() {
	u32 a;
	mirand *= 16807;
	a = (mirand & 0x007fffff) | 0x40000000;
	return *((f32*)&a) - 3.0f;
}
