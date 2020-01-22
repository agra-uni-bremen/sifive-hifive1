// See LICENSE for license details.

// executing out of SPI Flash at 0x20400000.

#include "helpers.h"
#include "spi_can.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "platform.h"
#include "plic/plic_driver.h"
#include "encoding.h"	//For CSRs
#include "sifive/devices/spi.h"

#include "CAN_BUS_Shield/mcp_can.h"

#define GREEN_LED 3
#define BLUE_LED 5
#define RED_LED 6
#define BUTTON 10


// Global Instance data for the PLIC
// for use by the PLIC Driver.
plic_instance_t g_plic;
// Structures for registering different interrupt handlers
// for different parts of the application.
typedef void (*interrupt_function_ptr_t) (void);

//array of function pointers which contains the PLIC
//interrupt handlers
interrupt_function_ptr_t g_ext_interrupt_handlers[PLIC_NUM_INTERRUPTS];


void button_handler()
{
	//clear interrupt pending for Button
	GPIO_REG(GPIO_FALL_IP) |=  (1 << mapPinToReg(BUTTON));
	//disable Interrupt to ignore bouncing
	GPIO_REG(GPIO_FALL_IE) &= ~(1 << mapPinToReg(BUTTON));

	//blink some happy LEDs
	blue_led ^= 1;
	setPin(BLUE_LED, blue_led);
}

/*configures Button0 as a global gpio irq*/
void b0_irq_init()  {

    //disable hw io function
    GPIO_REG(GPIO_IOF_EN)    &= ~(1 << mapPinToReg(BUTTON));

    //set to input
    GPIO_REG(GPIO_INPUT_EN)   |= (1 << mapPinToReg(BUTTON));
    GPIO_REG(GPIO_PULLUP_EN)  |= (1 << mapPinToReg(BUTTON));

    //set to interrupt on edges
    GPIO_REG(GPIO_FALL_IE)    |= (1 << mapPinToReg(BUTTON));

    PLIC_init(&g_plic,
  	    PLIC_CTRL_ADDR,
  	    PLIC_NUM_INTERRUPTS,
  	    PLIC_NUM_PRIORITIES);

    PLIC_enable_interrupt (&g_plic, INT_GPIO_BASE + mapPinToReg(BUTTON));
    PLIC_set_priority(&g_plic, INT_GPIO_BASE + mapPinToReg(BUTTON), 2);
    g_ext_interrupt_handlers[INT_GPIO_BASE + mapPinToReg(BUTTON)] = button_handler;

    _puts("Inited button\r\n");
}


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

	blue_led ^= 1;
	setPin(BLUE_LED, blue_led);
}

//default empty PLIC handler
void invalid_global_isr()
{
	printf("Unexpected global interrupt!\r\n");
}


int main (void)
{
	_puts("Config String:\n\r");
	_puts(*((const char **) 0x100C));
	_puts("\n\r");

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
	b0_irq_init();

	// Enable Global (PLIC) interrupts.
	set_csr(mie, MIP_MEIP);

	// Enable all interrupts
	set_csr(mstatus, MSTATUS_MIE);

	uint8_t blue_led = 0;
	uint8_t green_led = 1;
	while(1)
	{
		sleep(500);
		setPin(GREEN_LED, green_led);
		setPin(BLUE_LED, green_led);
		green_led ^= 1;
		blue_led ^= 1;
	}
}
