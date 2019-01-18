// See LICENSE for license details.

// executing out of SPI Flash at 0x20400000.

#include "helpers.h"
#include "encoding.h"
#include "platform.h"
#include "plic/plic_driver.h"

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

uint32_t mapPinToReg(uint8_t pin)
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

void setPinOutput(uint8_t pin)
{
	GPIO_REG(GPIO_INPUT_EN) &= ~(1 << mapPinToReg(pin));
	GPIO_REG(GPIO_OUTPUT_EN) |= 1 << mapPinToReg(pin);
	GPIO_REG(GPIO_OUTPUT_VAL) &= ~(1 << mapPinToReg(pin));
}

void setPinInput(uint8_t pin)
{
	  GPIO_REG(GPIO_OUTPUT_EN)  &= ~(1 << mapPinToReg(pin));
	  GPIO_REG(GPIO_PULLUP_EN)  &= ~(1 << mapPinToReg(pin));
	  GPIO_REG(GPIO_INPUT_EN)   |= 1 << mapPinToReg(pin);
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
	return GPIO_REG(GPIO_INPUT_VAL) & (1 << mapPinToReg(pin));
}


void sleep(uint32_t millis)
{
    volatile uint64_t *  now = (volatile uint64_t*)(CLINT_CTRL_ADDR + CLINT_MTIME);
    volatile uint64_t then = *now + millis * (RTC_FREQ / 1000);
    while (*now < then){}
}

void setTimer(uint32_t millis)
{
	//printf("Timer in %u ms\n", millis);
	volatile uint64_t * mtime       = (uint64_t*) (CLINT_CTRL_ADDR + CLINT_MTIME);
	volatile uint64_t * mtimecmp    = (uint64_t*) (CLINT_CTRL_ADDR + CLINT_MTIMECMP);
	uint64_t now = *mtime;
	uint64_t then = now + millis * (RTC_FREQ / 1000);
	*mtimecmp = then;

	// Enable the timer interrupt.
	set_csr(mie, MIP_MTIP);
}
