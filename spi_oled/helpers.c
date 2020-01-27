// See LICENSE for license details.

// executing out of SPI Flash at 0x20400000.

#include "helpers.h"
#include "encoding.h"
#include "platform.h"
#include "plic/plic_driver.h"

void uart_init()
{
    // Configure UART GPIO pins
    GPIO_REG(GPIO_OUTPUT_VAL) |= IOF0_UART0_MASK;
    GPIO_REG(GPIO_OUTPUT_EN)  |= IOF0_UART0_MASK;
    GPIO_REG(GPIO_IOF_SEL)    &= ~IOF0_UART0_MASK;
    GPIO_REG(GPIO_IOF_EN)     |= IOF0_UART0_MASK;
    // RX and TX enable
    UART0_REG(UART_REG_TXCTRL) = UART_TXEN;
    UART0_REG(UART_REG_RXCTRL) = UART_RXEN;
}

void _putc(char c) {
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


void _puts(const char * s) {
  while (*s != '\0'){
    _putc(*s++);
  }
}

void bitprint(uint32_t val)
{
	for(uint8_t i = 0; i < 32; i++)
	{
		_putc(val & (1 << (32 - i)) ? '1' : '0');
	}
	_puts("\r\n");
}


uint32_t mapPinToReg(uint8_t pin)
{
    if(pin < 8)
    {
        return pin + PIN_0_OFFSET;
    }
    if(pin >= 8 && pin < 14)
    {
        return pin - 8;
    }
    //ignoring non-wired pin 14 <==> 8
    if(pin > 14 && pin < 20)
    {
        return pin - 6;
    }
    return 0;
}

void setPinOutput(uint8_t pin)
{
	GPIO_REG(GPIO_INPUT_EN) &= ~(1 << mapPinToReg(pin));
	GPIO_REG(GPIO_OUTPUT_EN) |= 1 << mapPinToReg(pin);
	GPIO_REG(GPIO_OUTPUT_VAL) &= ~(1 << mapPinToReg(pin));
}

void setPinInput(uint8_t pin)
{
	setPinInputPullup(pin, 0);
}

void setPinInputPullup(uint8_t pin, uint8_t pullup_enable)
{
	GPIO_REG(GPIO_IOF_EN )    &= ~(1 << mapPinToReg(pin));
	GPIO_REG(GPIO_OUTPUT_EN)  &= ~(1 << mapPinToReg(pin));
	if(pullup_enable)
		GPIO_REG(GPIO_PULLUP_EN)  |= (1 << mapPinToReg(pin));
	else
		GPIO_REG(GPIO_PULLUP_EN)  &= ~(1 << mapPinToReg(pin));
	GPIO_REG(GPIO_INPUT_EN)   |= 1 << mapPinToReg(pin);
}


/**
 * @param mode 0: rise, 1: fall, 2: high, 3: low
 */
void enableInterrupt(uint8_t pin, uint8_t mode)
{
	switch(mode)
	{
	case 0:
		GPIO_REG(GPIO_RISE_IE)    |= (1 << mapPinToReg(pin));
		break;
	case 1:
		GPIO_REG(GPIO_FALL_IE)    |= (1 << mapPinToReg(pin));
		break;
	case 2:
		GPIO_REG(GPIO_HIGH_IE)    |= (1 << mapPinToReg(pin));
		break;
	case 3:
		GPIO_REG(GPIO_LOW_IE)    |= (1 << mapPinToReg(pin));
		break;
	default:
		_puts("Invalid interrupt mode");
	}
}

void setPin(uint8_t pin, uint8_t val)
{
	if(val)
	{
		GPIO_REG(GPIO_OUTPUT_VAL) |= 1 << mapPinToReg(pin);
	}
	else
	{
		GPIO_REG(GPIO_OUTPUT_VAL) &= ~(1 << mapPinToReg(pin));
	}
}

uint8_t getPin(uint8_t pin)
{
	return (GPIO_REG(GPIO_INPUT_VAL) & (1 << mapPinToReg(pin))) ? 1 : 0;
}

void printGPIOs()
{
	_puts("Output ENABLE ");
	bitprint(GPIO_REG(GPIO_OUTPUT_EN));
	_puts("Output  VALUE ");
	bitprint(GPIO_REG(GPIO_OUTPUT_VAL));
	_puts("Input  ENABLE ");
	bitprint(GPIO_REG(GPIO_INPUT_EN));
	_puts("Input   VALUE ");
	bitprint(GPIO_REG(GPIO_INPUT_VAL));
}

void sleep_u(uint64_t micros)
{
    volatile uint64_t *  now = (volatile uint64_t*)(CLINT_CTRL_ADDR + CLINT_MTIME);
    volatile uint64_t then = *now + ((micros * RTC_FREQ) / (1000 * 1000));
    while (*now < then){}
}

void sleep(uint64_t millis)
{
    sleep_u(millis * 1000);
}

void setTimer(uint32_t millis)
{
	//printf("Timer in %u ms\n", millis);
	volatile uint64_t * mtime       = (uint64_t*) (CLINT_CTRL_ADDR + CLINT_MTIME);
	volatile uint64_t * mtimecmp    = (uint64_t*) (CLINT_CTRL_ADDR + CLINT_MTIMECMP);
	uint64_t now = *mtime;
	uint64_t then = now + ((millis * RTC_FREQ) / 1000);
	*mtimecmp = then;

	// Enable the timer interrupt.
	set_csr(mie, MIP_MTIP);
}
