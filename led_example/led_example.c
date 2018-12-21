// See LICENSE for license details.

// executing out of SPI Flash at 0x20400000.

#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>

#include "encoding.h"	//For CSRs

typedef void (*interrupt_function_ptr_t) (void);
interrupt_function_ptr_t g_ext_interrupt_handlers[32];
//default empty PLIC handler
void invalid_global_isr()
{
	printf("Unexpected global interrupt!\r\n");
}
//default empty local handler
void invalid_local_isr() {
	printf ("Unexpected local interrupt!\n");
}

void handle_m_ext_interrupt()
{
	_exit(1337);
}


union DWord
{
	uint64_t as64;
	uint32_t as32[2];
};

int main (void)
{
    for (int gisr = 0; gisr < 32; gisr++){
            g_ext_interrupt_handlers[gisr] = invalid_global_isr;
    }



	union DWord cycle_pre, cycle_post;

	while(1)
	{
		asm volatile
		(
			"csrr %[pel], mcycle 	\n\t"
			"csrr %[peh], mcycleh	\n\t"
			"addi t0, t0, 1			\n\t"
			"csrr %[pol], mcycle 	\n\t"
			"csrr %[poh], mcycleh	\n\t"
			: [pel] "=r" (cycle_pre.as32[0]), [peh] "=r" (cycle_pre.as32[1]),
			  [pol] "=r" (cycle_post.as32[0]), [poh] "=r" (cycle_post.as32[1])
			:
			: "t0"
		);

		printf("addi t0, t0, 1: %lu Cycles\n", (uint32_t) (cycle_post.as64 - cycle_pre.as64));
		printf("\tpre  High: %lu, pre  Low: %lu\n", cycle_pre.as32[1], cycle_pre.as32[0]);
		printf("\tpost High: %lu, post Low: %lu\n\n", cycle_post.as32[1], cycle_post.as32[0]);
	}
}
