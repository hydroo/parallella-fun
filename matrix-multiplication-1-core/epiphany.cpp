#include "prereqs.hpp"

#include <e-lib.h>

f32 reserveBank1[2048] __attribute__ ((section (".data_bank1")));
f32 reserveBank2[2048] __attribute__ ((section (".data_bank2")));
f32 reserveBank3[2048] __attribute__ ((section (".data_bank3")));

int main(int argc, char** args) {
	(void) argc; (void) args;

	e_irq_global_mask(E_TRUE);

	int width  = *(int*) 0x40;
	int height = *(int*) 0x44;

	int aw = width;
	int ah = height;
	int bw = ah;
	int bh = aw; (void) bh;
	int cw = ah;
	int ch = ah;

	f32* a = (f32*) 0x2000;
	f32* b = (f32*) 0x4000;
	f32* c = (f32*) 0x6000;

	e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
	e_ctimer_set(E_CTIMER_1, E_CTIMER_MAX);

	e_ctimer_start(E_CTIMER_0, E_CTIMER_FPU_INST);
	e_ctimer_start(E_CTIMER_1, E_CTIMER_CLK);

	for (int i = 0; i < cw; i += 1) {
		for (int j = 0; j < ch; j += 1) {
			for (int k = 0; k < aw; k += 1) {
				c[j*cw + i] += a[j*aw + k] * b[k*bw + i];
			}
		}
	}

	u32 cycles = (u32) e_ctimer_get(E_CTIMER_1);
	u32 fpops  = (u32) e_ctimer_get(E_CTIMER_0);

	*(u32*) 0x48 = E_CTIMER_MAX - cycles;
	*(u32*) 0x4c = E_CTIMER_MAX - fpops;

	*(UserInterrupt*) 0x24 = UserInterrupt::Done;

	return 0;
}
