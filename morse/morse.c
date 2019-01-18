// See LICENSE for license details.

// executing out of SPI Flash at 0x20400000.

#include "helpers.h"
#include "morse.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "platform.h"
#include "plic/plic_driver.h"
#include "encoding.h"	//For CSRs

#define GREEN_LED 3
#define BLUE_LED 5
#define RED_LED 6
#define BUTTON 10


static const uint16_t pulselen[] =
{
		 50,	// This acts as a debounce to High
		450,	// Threshold on which pulse is considered long
		100,	// debounce to LOW
		500,	//pause inbetween
};

static enum MorseState currentState = none;
static enum MorseState morseWord[MORSE_MAXLEN + 1];
static uint8_t currentMorsePos = 0;
uint8_t blue_led = 1;

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
	currentState = none;
	setTimer(pulselen[none]);
	//clear interrupt pending for Button
	GPIO_REG(GPIO_FALL_IP) |=  (1 << mapPinToReg(10));
	GPIO_REG(GPIO_FALL_IE) &= ~(1 << mapPinToReg(10));
	blue_led = 0;
	setPin(BLUE_LED, blue_led);
}

/*configures Button0 as a global gpio irq*/
void b0_irq_init()  {

    //disable hw io function
    GPIO_REG(GPIO_IOF_EN)    &= ~(1 << mapPinToReg(10));

    //set to input
    GPIO_REG(GPIO_INPUT_EN)   |= (1 << mapPinToReg(10));
    GPIO_REG(GPIO_PULLUP_EN)  |= (1 << mapPinToReg(10));

    //set to interrupt on edges
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

void resetMorseWord()
{
	memset(morseWord, 0, sizeof(enum MorseState) * (MORSE_MAXLEN + 1));
	currentMorsePos = 0;
}

void insertIntoMorseWord(enum MorseState state)
{
	if(currentMorsePos >= MORSE_MAXLEN)
	{
		puts("Word too long.\r\n");
		resetMorseWord();
	}
	else
	{
		morseWord[currentMorsePos++] = state;
	}
}

void handle_m_time_interrupt()
{
	clear_csr(mie, MIP_MTIP);
	uint8_t buttonStillPressed = !getPin(BUTTON);
	switch(currentState)
	{
	case none:
		if(!buttonStillPressed)
		{
			//printf("Too short\r\n");
			GPIO_REG(GPIO_FALL_IE) |= (1 << mapPinToReg(10));
			break;
		}
		//printf("min short\r\n");
		currentState = shortt;
		setTimer(pulselen[currentState]);
		break;
	case shortt:
		if(!buttonStillPressed)
		{
			_putc('.');
			fflush(stdout);
			insertIntoMorseWord(shortt);
			GPIO_REG(GPIO_FALL_IE) |= (1 << mapPinToReg(10));
			currentState = pause;
			setTimer(pulselen[currentState]);
			break;
		}
		_putc('-');
		fflush(stdout);
		insertIntoMorseWord(longg);
		currentState = longg;
		setTimer(pulselen[currentState]);
		break;
	case longg:
		if(!buttonStillPressed)
		{
			GPIO_REG(GPIO_FALL_IE) |= (1 << mapPinToReg(10));
			currentState = pause;
			setTimer(pulselen[currentState]);
			break;
		}
		//Hey, let go of this Button!
		setTimer(pulselen[currentState]);
		break;
	case pause:
		//If we reach this, the Button Interrupt was not fired in between, and we have a pause
		_putc(findChar(morseWord));
		resetMorseWord();
		currentState = none;
		_puts("\r\n");
		break;
	}

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

	resetMorseWord();

	// Enable all interrupts
	set_csr(mstatus, MSTATUS_MIE);

	uint8_t green_led = 1;
	while(1)
	{
		sleep(500);
		setPin(GREEN_LED, green_led);
		green_led ^= 1;
	}
}
