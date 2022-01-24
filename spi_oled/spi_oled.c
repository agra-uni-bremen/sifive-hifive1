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

#define BUTTON_D   3
#define BUTTON_CTR 4
#define BUTTON_R   5
#define BUTTON_L   6
#define BUTTON_U   7
#define BUTTON_A   18
#define BUTTON_B   19

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

static char lipsum[] = "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.";
enum State {MANDELBROT = 0, TEXTMODE, LOREM, STATE_num};
volatile enum State state;

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

void handle_m_time_interrupt()
{
	clear_csr(mie, MIP_MTIP);
}

void button_handler(plic_source int_num)
{
	unsigned state_changed = 0;
	int_num -= INT_GPIO_BASE;
	if(int_num ==  mapPinToReg(BUTTON_D))
	{
		puts("BUTTON D!\r\n");
	}
	else if(int_num == mapPinToReg(BUTTON_U))
	{
		puts("BUTTON U!\r\n");
	}
	else if(int_num == mapPinToReg(BUTTON_L))
	{
		puts("BUTTON L!\r\n");
		state = (state+STATE_num-1)%STATE_num;
		state_changed = 1;
	}
	else if(int_num == mapPinToReg(BUTTON_R))
	{
		puts("BUTTON R!\r\n");
		state = (state+1)%STATE_num;
		state_changed = 1;
	}
	else if(int_num == mapPinToReg(BUTTON_CTR))
	{
		puts("BUTTON C!\r\n");
	}
	else
	{
		puts("Some button.\r\n");
	}

	//transitions
	if(state_changed)
	{
		switch (state)
		{
		case MANDELBROT:
			puts("Mandelbrot\r\n");
			break;
		case TEXTMODE:
			puts("Textmode\r\n");
			break;
		case LOREM:
			puts("Lorem ipsum\r\n");
			break;
		default:
			printText("Invalid mode\n");
			sleep(1000);
		}
	}

	GPIO_REG(GPIO_FALL_IP) |= (1 << int_num);
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
		enableInterrupt(buttons[i], 1);	//fall
	    PLIC_enable_interrupt (&g_plic, INT_GPIO_BASE + mapPinToReg(buttons[i]));
	    PLIC_set_priority(&g_plic, INT_GPIO_BASE + mapPinToReg(buttons[i]), 2+i);
	    g_ext_interrupt_handlers[INT_GPIO_BASE + mapPinToReg(buttons[i])] = button_handler;
	    printf("Inited button %d\r\n", buttons[i]);
	}
}

uint8_t wait_condition()
{
	uint8_t ch;
	//printGPIOs();
	if(state == MANDELBROT)
		return 1;
	if(!_getc(&ch))
		return 0;
	return ch;
}

int main (void)
{
	_init();
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
	state = MANDELBROT;
	uint8_t ch = 0;

	mandelbrot(wait_condition);	//this may block a while
	cls();
	
	for(unsigned i = 0; i < sizeof(lipsum); i++){
		printChar(lipsum[i]);
		if(i % ((DISP_W / CHAR_W) * (DISP_H/8)) == ((DISP_W / CHAR_W) * (DISP_H/8))-1)
			sleep(100);
	}
}
