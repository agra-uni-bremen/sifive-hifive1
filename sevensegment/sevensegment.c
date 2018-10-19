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

volatile int direction = 1;

static void sleep(uint32_t millis)
{
    volatile uint64_t *  now = (volatile uint64_t*)(CLINT_CTRL_ADDR + CLINT_MTIME);
    volatile uint64_t then = *now + millis*(RTC_FREQ / 1000);
    int prevDirection = direction;
    while (*now < then)
    {
    	if(prevDirection == direction && !getPin(10))
    	{
    		direction = direction > 0 ? -1 : 1;
    		while(!getPin(10)){};	//wow, even more active Waits :O
    	}
    }
}

static void bitprint(uint32_t val)
{
	for(uint8_t i = 0; i < 32; i++)
	{
		_putc(val & (1 << i) ? '1' : '0');
	}
	_putc('\r\n');
}

static void printGPIOs()
{
	//bitprint(GPIO_REG(GPIO_INPUT_EN));
	//_puts("Output ENABLE ");
	//bitprint(GPIO_REG(GPIO_OUTPUT_EN));
	_puts("Output  VALUE ");
	bitprint(GPIO_REG(GPIO_OUTPUT_VAL));
	_puts("Input   VALUE ");
	bitprint(GPIO_REG(GPIO_INPUT_VAL));
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

	GPIO_REG(GPIO_OUTPUT_VAL) = reg;
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
	while(i < 5000){i++;}

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

	setPinInput(10);

	//0123456789
	//  abcdefg.
	//0x11111111

	uint8_t j = 0;
	while(0)
	{
		_putc('a' + j);
		_puts("\r\n");
		setPin(&GPIO_REG(GPIO_OUTPUT_VAL), 2 + j, 1);
		while (_getc(&c) == 0){}
		setPin(&GPIO_REG(GPIO_OUTPUT_VAL), 2 + j, 0);
		j = j+1 <= 8 ? j+1 : 0;
	}

	while(1)
	{
		displayNumber(counter%10, direction != 1);
		_puts("Number: ");
		_putc('0' + counter%10);
		if(counter%20 > 9)
		{
			_putc('.');
		}
		_puts("\r\n");
		printGPIOs();

		sleep(250);


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
