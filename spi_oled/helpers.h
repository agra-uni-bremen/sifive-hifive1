#ifndef HELPERS_H
#define HELPERS_H

#include <stdint.h>

void uart_init();
void _putc(char c);
int _getc(char * c);
void _puts(const char * s);
void bitprint(uint32_t val);

uint32_t mapPinToReg(uint8_t pin);

void setPinOutput(uint8_t pin);
void setPinInput(uint8_t pin);
void setPinInputPullup(uint8_t pin, uint8_t pullup_enable);
/**
 * @param mode 0: rise, 1: fall, 2: high, 3: low
 */
void enableInterrupt(uint8_t pin, uint8_t mode);

void setPin(uint8_t pin, uint8_t val);
uint8_t getPin(uint8_t pin);

void printGPIOs();

void sleep_u(uint64_t micros);
void sleep(uint64_t millis);
uint64_t getTime_ms();

void setTimer(uint32_t millis);

#endif
