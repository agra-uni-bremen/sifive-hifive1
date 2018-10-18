// See LICENSE for license details.

// executing out of SPI Flash at 0x20400000.

#include <stdint.h>
#include "platform.h"

static const char led_msg[] = "\a\n\r\n\r\
55555555555555555555555555555555555555555555555\n\r\
55555 Are able to press Buttons? [y/n]  5555555\n\r\
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
55555          9999999999999         55555\n\r\
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
               'led_fade' Demo \n\r\
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

static uint16_t segmentMapping[10] =
{
		0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0x70, 0x7F, 0x7B
};

static void displayNumber(uint8_t number, uint8_t dot)
{
	uint32_t reg = GPIO_REG(GPIO_OUTPUT_VAL) & ~(0xFF << 2);	//2 is pin offset, hardcoded

	if(number < 10)
		reg |= segmentMapping[number] << 2;
	else
		reg |= 0x01000000 << 2;

	if(dot)
		reg |= 0x10000000 << 2;

	GPIO_REG(GPIO_OUTPUT_VAL) |= reg;
}

static void sleep(uint32_t millis)
{
    volatile uint64_t *  now = (volatile uint64_t*)(CLINT_CTRL_ADDR + CLINT_MTIME);
    volatile uint64_t then = *now + millis;
    while (*now < then) { }
}


int main (void){

	// Make sure the HFROSC is on before the next line:
	PRCI_REG(PRCI_HFROSCCFG) |= ROSC_EN(1);
	// Run off 16 MHz Crystal for accuracy. Note that the
	// first line is
	PRCI_REG(PRCI_PLLCFG) = (PLL_REFSEL(1) | PLL_BYPASS(1));
	PRCI_REG(PRCI_PLLCFG) |= (PLL_SEL(1));
	// Turn off HFROSC to save power
	PRCI_REG(PRCI_HFROSCCFG) &= ~(ROSC_EN(1));

	// Configure UART to print
	GPIO_REG(GPIO_OUTPUT_VAL) |= IOF0_UART0_MASK;
	GPIO_REG(GPIO_OUTPUT_EN)  |= IOF0_UART0_MASK;
	GPIO_REG(GPIO_IOF_SEL)    &= ~IOF0_UART0_MASK;
	GPIO_REG(GPIO_IOF_EN)     |= IOF0_UART0_MASK;

	// 115200 Baud Rate
	UART0_REG(UART_REG_DIV) = 138;
	UART0_REG(UART_REG_TXCTRL) = UART_TXEN;
	UART0_REG(UART_REG_RXCTRL) = UART_RXEN;

	// Wait a bit to avoid corruption on the UART.
	// (In some cases, switching to the IOF can lead
	// to output glitches, so need to let the UART
	// reciever time out and resynchronize to the real
	// start of the stream.
	volatile int i=0;
	while(i < 10000){i++;}

	_puts(sifive_msg);
	_puts("Config String:\n\r");
	_puts(*((const char **) 0x100C));
	_puts("\n\r");
	_puts(led_msg);


	char c = 0;
	uint8_t counter = 0;

	for(uint8_t i = 2; i <= 9; i++)
	{
	  GPIO_REG(GPIO_INPUT_EN)  &= ~(1 << i);
	  GPIO_REG(GPIO_OUTPUT_EN) |=  1 << i;
	}


	while(1)
	{
		displayNumber(counter%10, counter%20 > 9);
		counter++;

		sleep(500);

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
	}
}
