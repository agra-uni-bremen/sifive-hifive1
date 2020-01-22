// See LICENSE for license details.

// executing out of SPI Flash at 0x20400000.

#include "helpers.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "platform.h"
#include "plic/plic_driver.h"
#include "encoding.h"	//For CSRs
#include "sifive/devices/spi.h"

#include "oled_shield.h"
#include "font.h"
#include "display.h"


const uint8_t buttons[] =
{
	BUTTON_D,
	BUTTON_CTR,
	BUTTON_R,
	BUTTON_L,
	BUTTON_U,
	BUTTON_A,
	BUTTON_B
};


// Global Instance data for the PLIC
// for use by the PLIC Driver.
plic_instance_t g_plic;
// Structures for registering different interrupt handlers
// for different parts of the application.
typedef void (*interrupt_function_ptr_t) (plic_source);

//array of function pointers which contains the PLIC
//interrupt handlers
interrupt_function_ptr_t g_ext_interrupt_handlers[PLIC_NUM_INTERRUPTS];


/*Entry Point for PLIC Interrupt Handler*/
void handle_m_ext_interrupt()
{
    plic_source int_num  = PLIC_claim_interrupt(&g_plic);
	if ((int_num >=1 ) && (int_num < PLIC_NUM_INTERRUPTS))
	{
		g_ext_interrupt_handlers[int_num](int_num);
	}
	else
	{
		//exit(1 + (uintptr_t) int_num);
		printf("unhandled Interrupt %u", int_num);
		while(1){};
	}
	PLIC_complete_interrupt(&g_plic, int_num);
	//puts("completed interrupt\r\n");
}

void button_handler(plic_source int_num)
{
	int_num -= INT_GPIO_BASE;
	if(int_num ==  mapPinToReg(BUTTON_D))
	{
		puts("BUTTON D!\r\n");
	}
	else if(int_num == mapPinToReg(BUTTON_U))
	{
		puts("BUTTON U!\r\n");
	}
	else
	{
		puts("Some button.\r\n");
	}
}

//default empty PLIC handler
void invalid_global_isr(plic_source int_num)
{
	printf("Unexpected global interrupt %d!\r\n", int_num);
}

void setup_button_irq()
{
    PLIC_init(&g_plic,
            PLIC_CTRL_ADDR,
            PLIC_NUM_INTERRUPTS,
            PLIC_NUM_PRIORITIES);


	for(unsigned i = 0; i < sizeof(buttons); i++)
	{
		setPinInputPullup(buttons[i], 1);
		enableInterrupt(buttons[i], 0);	//rise necessary?
		enableInterrupt(buttons[i], 1);	//fall
	    PLIC_enable_interrupt (&g_plic, INT_GPIO_BASE + mapPinToReg(buttons[i]));
	    PLIC_set_priority(&g_plic, INT_GPIO_BASE + mapPinToReg(buttons[i]), 2+i);
	    g_ext_interrupt_handlers[INT_GPIO_BASE + mapPinToReg(buttons[i])] = button_handler;

	}
}

void handle_m_time_interrupt()
{
	clear_csr(mie, MIP_MTIP);
}

int main (void)
{

	//setup default global interrupt handler
	for (int gisr = 0; gisr < PLIC_NUM_INTERRUPTS; gisr++){
		g_ext_interrupt_handlers[gisr] = invalid_global_isr;
	}
	setup_button_irq();


	// Enable Global (PLIC) interrupts.
	set_csr(mie, MIP_MEIP);

	// Enable all interrupts
	set_csr(mstatus, MSTATUS_MIE);

	uart_init();

	oled_init();

	puts("Now mainloop\r\n");
	printText("\n\n\n  Press any button\n      to exit\n");
	sleep(2000);
	while (1) {
		sleep(1000);
	}
}
