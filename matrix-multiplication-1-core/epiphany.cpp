#include "prereqs.hpp"

#include <e-lib.h>

f32 a[2048] __attribute__ ((aligned(8), section (".data_bank1")));
f32 b[2048] __attribute__ ((aligned(8), section (".data_bank2")));
f32 c[2048] __attribute__ ((aligned(8), section (".data_bank3")));

int main(int argc, char** args) {
	(void) argc; (void) args;

	e_irq_global_mask(E_TRUE);

	const u32 w = *(u32*) 0x40;
	const u32 h = *(u32*) 0x44;

	f32* b_kh = b;
	f32* c_jh;

	// if (a != (const f32*) 0x2000 || b != (const f32*) 0x4000 || c != (const f32*) 0x6000) {
	// 	*(UserInterrupt*) 0x24 = UserInterrupt::Error;
	// 	return 0;
	// }

	e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
	e_ctimer_set(E_CTIMER_1, E_CTIMER_MAX);

	e_ctimer_start(E_CTIMER_0, E_CTIMER_FPU_INST);
	e_ctimer_start(E_CTIMER_1, E_CTIMER_CLK);

	for (u32 k = 0; k < w; k += 1) {
		c_jh = c;
		for (u32 j = 0; j < h; j += 1) {
			f32 a_jw_k = a[j*w + k];
			for (u32 i = 0; i < h;) {
				if (i + 1 < h) {
					c_jh[i    ] += a_jw_k * b_kh[i    ];
					c_jh[i + 1] += a_jw_k * b_kh[i + 1];
					i += 2;
				} else {
					c_jh[i    ] += a_jw_k * b_kh[i    ];
					i += 1;
				}
			}
			c_jh += h;
		}
		b_kh += h;
	}

// 	for (u32 k = 0; k < w; k += 1) { // this order of loops is faster than the others on epiphany
// 		for (u32 j = 0; j < h; j += 1) {
// 			for (u32 i = 0; i < h; i += 1) {
// 				c[j*h + i] += a[j*w + k] * b[k*h + i];
// 			}
// 		}
// 	}

	u32 cycles = (u32) e_ctimer_get(E_CTIMER_1);
	u32 fpops  = (u32) e_ctimer_get(E_CTIMER_0);

	*(u32*) 0x48 = E_CTIMER_MAX - cycles;
	*(u32*) 0x4c = E_CTIMER_MAX - fpops;

	*(UserInterrupt*) 0x24 = UserInterrupt::Done;

	return 0;
}
