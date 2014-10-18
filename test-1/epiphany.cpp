#include "prereqs.hpp"

#include <e-lib.h>

u8 a[5068] __attribute__ ((section (".data_bank0")));
u8 b[8192] __attribute__ ((section (".data_bank1")));
u8 c[8192] __attribute__ ((section (".data_bank2")));
u8 d[8192] __attribute__ ((section (".data_bank3")));

int main(int argc, char** args) {

	e_irq_mask(E_SYNC        , E_FALSE);
	e_irq_mask(E_SW_EXCEPTION, E_FALSE);
	e_irq_mask(E_MEM_FAULT   , E_FALSE);
	e_irq_mask(E_TIMER0_INT  , E_TRUE);
	e_irq_mask(E_TIMER1_INT  , E_FALSE);
	e_irq_mask(E_MESSAGE_INT , E_FALSE);
	e_irq_mask(E_DMA0_INT    , E_FALSE);
	e_irq_mask(E_DMA1_INT    , E_FALSE);
	e_irq_mask(E_USER_INT    , E_FALSE);

	e_coords_from_coreid(e_get_coreid(), (unsigned*) 0x44, (unsigned*) 0x40);

	*((u32*) 0x4c) = (u32) a;
	*((u32*) 0x50) = (u32) b;
	*((u32*) 0x54) = (u32) c;
	*((u32*) 0x58) = (u32) d;

	*((UserInterrupt*) 0x24) = StartedInterrupt;

	// e_wait(E_CTIMER_0, 600 * 1000 * 100);

	// this is manual e_waiting, kind of
	e_ctimer_set(E_CTIMER_0, 600 * 1000 * 100);
	e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);

	while ((*((u32*) 0x48) = e_ctimer_get(E_CTIMER_0)) > 0) {
	}

	*((UserInterrupt*) 0x24) = FinishInterrupt;

	return 0;
}
