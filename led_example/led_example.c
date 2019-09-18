// See LICENSE for license details.

// executing out of SPI Flash at 0x20400000.

#include <stdint.h>
#include <stdio.h>
#include "platform.h"
#include "plic/plic_driver.h"
#include "encoding.h"	//For CSRs

#define GREEN_LED 3
#define BLUE_LED 5
#define RED_LED 6

static void _putc(char c) {
  while ((int32_t) UART0_REG(UART_REG_TXFIFO) < 0);
  UART0_REG(UART_REG_TXFIFO) = c;
}

int _getc(char * c){
  int32_t val = (int32_t) UART0_REG(UART_REG_RXFIFO);
  if (val > 0) {
    *c =  val & 0xFF;
    return 1;
  }
  return 0;
}


static void _puts(const char * s) {
  while (*s != '\0'){
    _putc(*s++);
  }
}

static uint32_t mapPinToReg(uint8_t pin)
{
	if(pin < 8)
	{
		return pin + PIN_0_OFFSET;
	}
	if(pin < 20)
	{
		return pin - 8;
	}
	return 0;	//also ignoring non-wired pin 14 <==> 8
}

static void setPinOutput(uint8_t pin)
{
	GPIO_REG(GPIO_INPUT_EN) &= ~(1 << mapPinToReg(pin));
	GPIO_REG(GPIO_OUTPUT_EN) |= 1 << mapPinToReg(pin);
	GPIO_REG(GPIO_OUTPUT_VAL) &= ~(1 << mapPinToReg(pin));
}

static void setPinInput(uint8_t pin)
{
	  GPIO_REG(GPIO_OUTPUT_EN)  &= ~(1 << mapPinToReg(pin));
	  GPIO_REG(GPIO_PULLUP_EN)  &= ~(1 << mapPinToReg(pin));
	  GPIO_REG(GPIO_INPUT_EN)   |= 1 << mapPinToReg(pin);
}

static void setPin(volatile uint32_t* reg, uint8_t pin, uint8_t val)
{
	if(val)
	{
		*reg |= 1 << mapPinToReg(pin);
	}
	else
	{
		*reg &= ~(1 << mapPinToReg(pin));
	}
}

static uint8_t getPin(uint8_t pin)
{
	return GPIO_REG(GPIO_INPUT_VAL) & (1 << mapPinToReg(pin));
}


static void sleep(uint32_t millis)
{
    volatile uint64_t *  now = (volatile uint64_t*)(CLINT_CTRL_ADDR + CLINT_MTIME);
    volatile uint64_t then = *now + millis*(RTC_FREQ / 1000);
    while (*now < then){}
}

// Global Instance data for the PLIC
// for use by the PLIC Driver.
plic_instance_t g_plic;
// Structures for registering different interrupt handlers
// for different parts of the application.
typedef void (*interrupt_function_ptr_t) (void);

//array of function pointers which contains the PLIC
//interrupt handlers
interrupt_function_ptr_t g_ext_interrupt_handlers[PLIC_NUM_INTERRUPTS];

void button_handler() {
	if(getPin(10))
	{	//rise
		setPin(&GPIO_REG(GPIO_OUTPUT_VAL), RED_LED, 1);
	}
	else
	{	//fall
		setPin(&GPIO_REG(GPIO_OUTPUT_VAL), RED_LED, 0);
	}
	//clear irq - interrupt pending register write 1 to clear
	GPIO_REG(GPIO_RISE_IP) |= (1 << mapPinToReg(10));
	GPIO_REG(GPIO_FALL_IP) |= (1 << mapPinToReg(10));

	_puts("button handled\r\n");
}

/*configures Button0 as a global gpio irq*/
void b0_irq_init()  {

    //disable hw io function
    GPIO_REG(GPIO_IOF_EN )    &= ~(1 << mapPinToReg(10));

    //set to input
    GPIO_REG(GPIO_INPUT_EN)   |= (1 << mapPinToReg(10));
    GPIO_REG(GPIO_PULLUP_EN)  |= (1 << mapPinToReg(10));

    //set to interrupt on edges
    GPIO_REG(GPIO_FALL_IE)    |= (1 << mapPinToReg(10));
    GPIO_REG(GPIO_RISE_IE)    |= (1 << mapPinToReg(10));

    PLIC_init(&g_plic,
  	    PLIC_CTRL_ADDR,
  	    PLIC_NUM_INTERRUPTS,
  	    PLIC_NUM_PRIORITIES);

    PLIC_enable_interrupt (&g_plic, INT_GPIO_BASE + mapPinToReg(10));
    PLIC_set_priority(&g_plic, INT_GPIO_BASE + mapPinToReg(10), 2);
    g_ext_interrupt_handlers[INT_GPIO_BASE + mapPinToReg(10)] = button_handler;

    _puts("Inited button\r\n");
}

/*Synchronous Trap Handler*/
/*REQUIRED and called from bsp/env/ventry.s */
void handle_sync_trap(uint32_t arg0) {
	printf("vUnhandled Trap:\n");
	_exit(1 + read_csr(mcause));
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


//default empty PLIC handler
void invalid_global_isr()
{
	printf("Unexpected global interrupt!\r\n");
}
//default empty local handler
void invalid_local_isr() {
  printf ("Unexpected local interrupt!\n");
}

int main (void)
{
	_puts("Config String:\n\r");
	_puts(*((const char **) 0x100C));
	_puts("\n\r");

	setPinOutput(RED_LED);
	setPin(&GPIO_REG(GPIO_OUTPUT_VAL), RED_LED, 1);
	setPinOutput(BLUE_LED);
	setPin(&GPIO_REG(GPIO_OUTPUT_VAL), BLUE_LED, 1);
	setPinOutput(GREEN_LED);
	setPin(&GPIO_REG(GPIO_OUTPUT_VAL), GREEN_LED, 1);

	//setup default global interrupt handler
	for (int gisr = 0; gisr < PLIC_NUM_INTERRUPTS; gisr++){
		g_ext_interrupt_handlers[gisr] = invalid_global_isr;
	}
	b0_irq_init();

	// Enable Global (PLIC) interrupts.
	set_csr(mie, MIP_MEIP);

	// Enable all interrupts
	set_csr(mstatus, MSTATUS_MIE);

	uint8_t green_led = 1;
	while(1)
	{
		sleep(500);
		setPin(&GPIO_REG(GPIO_OUTPUT_VAL), GREEN_LED, green_led);
		green_led ^= 1;
	}
}
