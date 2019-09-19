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

#include "font.h"
#include "display.h"
#include "mandelbrot.h"

#define GREEN_LED 3
#define BLUE_LED 5
#define RED_LED 6

#define OLED_RESET	14
#define OLED_DC		15
#define OLED_CS		10


static char lipsum[] = "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.";

// Global Instance data for the PLIC
// for use by the PLIC Driver.
plic_instance_t g_plic;
// Structures for registering different interrupt handlers
// for different parts of the application.
typedef void (*interrupt_function_ptr_t) (void);

//array of function pointers which contains the PLIC
//interrupt handlers
interrupt_function_ptr_t g_ext_interrupt_handlers[PLIC_NUM_INTERRUPTS];


/*Entry Point for PLIC Interrupt Handler*/
void handle_m_ext_interrupt()
{
    plic_source int_num  = PLIC_claim_interrupt(&g_plic);
	if ((int_num >=1 ) && (int_num < PLIC_NUM_INTERRUPTS))
	{
		g_ext_interrupt_handlers[int_num]();
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

void handle_m_time_interrupt()
{
	clear_csr(mie, MIP_MTIP);
}

//default empty PLIC handler
void invalid_global_isr()
{
	printf("Unexpected global interrupt!\r\n");
}

int main (void)
{

	setPinOutput(RED_LED);
	setPin(RED_LED, 1);
	setPinOutput(BLUE_LED);
	setPin(BLUE_LED, 1);
	setPinOutput(GREEN_LED);
	setPin(GREEN_LED, 1);

	//setup default global interrupt handler
	for (int gisr = 0; gisr < PLIC_NUM_INTERRUPTS; gisr++){
		g_ext_interrupt_handlers[gisr] = invalid_global_isr;
	}

	// Enable Global (PLIC) interrupts.
	set_csr(mie, MIP_MEIP);

	// Enable all interrupts
	set_csr(mstatus, MSTATUS_MIE);

	uart_init();

	oled_init();

	puts("Now mainloop\r\n");
	printText("\n\n\npress ESC to exit\n");
	sleep(2000);
	uint8_t ch = 0;
	while (1) {
		cls();
		puts("Mandelbrot\r\n");
		mandelbrot();

		cls();
		puts("Textmode\r\n");
		printText("[ESC to exit]\n");
		while(!_getc(&ch))
			asm volatile ("nop");
		cls();
		while(ch != 27)
		{
			printChar(ch);
			while(!_getc(&ch))
				asm volatile ("nop");
		}

		cls();
		puts("LoremIpsum\r\n");
		unsigned lorem_pointer = 0;
		while(!_getc(&ch))
		{
			printChar(lipsum[lorem_pointer]);
			if(lorem_pointer % ((DISP_W / CHAR_W) * (DISP_H/8)) == ((DISP_W / CHAR_W) * (DISP_H/8))-1)
				sleep(100);
			lorem_pointer = lorem_pointer + 1 >= sizeof(lipsum) ? 0 : lorem_pointer + 1;
		}
	}
}
