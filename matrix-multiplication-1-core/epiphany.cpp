#include "prereqs.hpp"

#define N 16
#define A(i,j) a_mat[j*N+i]
#define B(i,j) b_mat[j*N+i]
#define C(i,j) c_mat[j*N+i]

f32 a_mat[N*N] __attribute__ ((section (".data_bank1")));
f32 b_mat[N*N] __attribute__ ((section (".data_bank2")));
f32 c_mat[N*N] __attribute__ ((section (".data_bank3")));

static f32 sfrand();

u32 e_test_init() {
	u32 *pass = (u32*) 0x24; // use user interrupt entry for now
	*pass = 0x00000000;      // initialize as "started"

	// resetting mask register
	__asm__ __volatile__ ("MOVTS IMASK, %0" : : "r" (0x0));

	// get the coreIdentifier using assembly
	register u32 coreIdentifier asm("r0");
	__asm__ __volatile__ ("MOVFS %0, COREID" : : "r" (coreIdentifier));
	coreIdentifier <<= 20;
	return coreIdentifier;
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
	probe_data = *(addr);         // read old data
	probe_data ^= 0xFFFFFFFF;     // toggle old data
	(*(addr)) = probe_data;       // write new toggled data
	while(probe_data != *(addr)){ // keep reading until match is met
	}
}

int main(int argc, char** args) {
	f32  sum = 0.0f;
	int  status = 1;
	u32  coreIdentifier;

	// test init
	coreIdentifier = e_test_init();

	// fill input matrices with a constant
	for (int i = 0; i < N; i += 1) {
		for (int j = 0; j < N; j += 1) {
			A(i,j) = sfrand();
			B(i,j) = sfrand();
			C(i,j) = 0;
		}
	}

	// run matrix multiplication
	for (int i = 0; i < N; i+= 1) {
		for (int j = 0; j < N; j += 1) {
			for (int k = 0; k < N; k += 1) {
				C(i,j) += A(i,k) * B(k,j);
			}
		}
	}

	// sum up the C matrix
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
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
	static u32 mirand = 1;
	u32 a;
	mirand *= 16807;
	a = (mirand & 0x007fffff) | 0x40000000;
	return *((f32*)&a) - 3.0f;
}
