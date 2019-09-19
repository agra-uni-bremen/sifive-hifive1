#ifndef HELPERS_H
#define HELPERS_H

#include <stdint.h>

void uart_init();
void _putc(char c);
int _getc(char * c);
void _puts(const char * s);

uint32_t mapPinToReg(uint8_t pin);

void setPinOutput(uint8_t pin);
void setPinInput(uint8_t pin);

void setPin(uint8_t pin, uint8_t val);
uint8_t getPin(uint8_t pin);

void sleep_u(uint64_t micros);
void sleep(uint64_t millis);

void setTimer(uint32_t millis);

#endif
