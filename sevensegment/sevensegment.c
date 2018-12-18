// See LICENSE for license details.

// executing out of SPI Flash at 0x20400000.

#include <stdint.h>
#include <stdio.h>
#include "platform.h"
#include "plic/plic_driver.h"
#include "encoding.h"	//For CSRs

static const char led_msg[] = "\a\n\r\n\r\
55555555555555555555555555555555555555555555555\n\r\
555555 Are able to press Buttons? [y/n]  555555\n\r\
55555555555555555555555555555555555555555555555\n\r";

static const char sifive_msg[] = "\n\r\
\n\r\
                SIFIVE, INC.\n\r\
\n\r\
         5555555555555555555555555\n\r\
        5555                   5555\n\r\
       5555                     5555\n\r\
      5555                       5555\n\r\
     5555       5555555555555555555555\n\r\
    5555       555555555555555555555555\n\r\
   5555                             5555\n\r\
  5555         yeeeeeeeeeeee         5555\n\r\
 5555                                 5555\n\r\
55555          9999999999999          55555\n\r\
 55555           555555555           55555\n\r\
   55555           55555           55555\n\r\
     55555           5           55555\n\r\
       55555                   55555\n\r\
         55555               55555\n\r\
           55555           55555\n\r\
             55555       55555\n\r\
               55555   55555\n\r\
                 555555555\n\r\
                   55555\n\r\
                     5\n\r\
\n\r\
               'sevensegment' Demo \n\r\
\n\r";

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

static uint16_t segmentMapping[10] =
{
		//abcdefg
		//0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0x70, 0x7F, 0x7B
		//gfedcba
		0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

static void sleep(uint32_t millis)
{
    volatile uint64_t *  now = (volatile uint64_t*)(CLINT_CTRL_ADDR + CLINT_MTIME);
    volatile uint64_t then = *now + millis*(RTC_FREQ / 1000);
    while (*now < then){}
}

static void bitprint(uint32_t val)
{
	for(uint8_t i = 0; i < 32; i++)
	{
		_putc(val & (1 << (32 - i)) ? '1' : '0');
	}
	_puts("\r\n");
}

static void printGPIOs()
{
	//bitprint(GPIO_REG(GPIO_INPUT_EN));
	//_puts("Output ENABLE ");
	//bitprint(GPIO_REG(GPIO_OUTPUT_EN));
	_puts("Output  VALUE ");
	bitprint(GPIO_REG(GPIO_OUTPUT_VAL));
	//_puts("Input   VALUE ");
	//bitprint(GPIO_REG(GPIO_INPUT_VAL));
}

static void displayNumber(uint8_t number, uint8_t dot)
{
	uint32_t reg = GPIO_REG(GPIO_OUTPUT_VAL) & ~(0xFF << PIN_0_OFFSET);
	if(number < 10)
	{
		for(uint8_t i = 0; i < 8; i++)
		{
			setPin(&reg, 2 + i, segmentMapping[number] & (1 << i));
		}
	}
	else
		reg |= 1 << 7;

	if(dot)
		setPin(&reg, 9, 1);

	//_puts("New OUTPUT_VALUE: \r\n");
	//bitprint(reg);
	GPIO_REG(GPIO_OUTPUT_VAL) = reg;
}

volatile int direction = 1;
volatile uint8_t directionChangePending = 0;

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
	//_puts("In Button handler\r\n");

	//only change when pressing down, small debounce
	if(!directionChangePending)
	{
		if(direction > 0)
		{
			direction = -1;
		}
		else
		{
			direction = 1;
		}
		directionChangePending = 1;
	}
	//clear irq - interrupt pending register is write 1 to clear
	GPIO_REG(GPIO_FALL_IP) |= (1 << mapPinToReg(10));
	_puts("button press handled\r\n");
}

/*configures Button0 as a global gpio irq*/
void b0_irq_init()  {

    //disable hw io function
    GPIO_REG(GPIO_IOF_EN )    &= ~(1 << mapPinToReg(10));

    //set to input
    GPIO_REG(GPIO_INPUT_EN)   |= (1 << mapPinToReg(10));
    GPIO_REG(GPIO_PULLUP_EN)  |= (1 << mapPinToReg(10));

    //set to interrupt on falling edge
    GPIO_REG(GPIO_FALL_IE)    |= (1 << mapPinToReg(10));

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
/*REQUIRED and called from bsp/env/ventry.s          */
void handle_sync_trap(uint32_t arg0) {
	uint32_t exception_code = read_csr(mcause);
	//printf("handling sync_trap %u\r\n", exception_code);
	//check for machine mode ecall
	if(exception_code == CAUSE_MACHINE_ECALL)
	{
		//reset ecall_countdown
		//ecall_countdown = 0;

		//ecall argument is stored in a0 prior to
		//ECALL instruction.
		printf("ecall from M-mode: %d\n",arg0);

		//on exceptions, mepc points to the instruction
		//which triggered the exception, in order to
		//return to the next instruction, increment
		//mepc
		unsigned long epc = read_csr(mepc);
		epc += 4; //return to next instruction
		write_csr(mepc, epc);

	}
	else
	{
		printf("vUnhandled Trap:\n");
		//_exit(1 + read_csr(mcause));
		while(1){};
	}
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
	_puts(sifive_msg);
	_puts("Config String:\n\r");
	_puts(*((const char **) 0x100C));
	_puts("\n\r");
	_puts(led_msg);


	char c = 0;
	uint8_t counter = 0;

	for(uint8_t i = 2; i <= 9; i++)
	{
		setPinOutput(i);
	}

	//setup default global interrupt handler
	for (int gisr = 0; gisr < PLIC_NUM_INTERRUPTS; gisr++){
		g_ext_interrupt_handlers[PLIC_NUM_INTERRUPTS] = invalid_global_isr;
	}
	b0_irq_init();

	// Set up machine timer interrupt.
	//set_timer();

	// Enable Global (PLIC) interrupts.
	set_csr(mie, MIP_MEIP);

	// Enable all interrupts
	set_csr(mstatus, MSTATUS_MIE);

	//  abcdefg.
	//0x11111111

	while(1)
	{
		displayNumber(counter%10, direction != 1);
		_puts("Number: ");
		_putc('0' + counter%10);
		_puts("\r\n");

		sleep(500);
		directionChangePending = 0;

		// Check for user input
		if (c == 0)
		{
			if (_getc(&c) != 0)
			{
				_putc(c);
				_puts("\n\r");
				if ((c == 'y') || (c == 'Y')){
				  _puts("You pressed a button. Good job.\n\r");
				} else{
				  _puts("This is not a 'Y'.\n\r");
				}
			}
		}
		counter += direction;
	}
}
