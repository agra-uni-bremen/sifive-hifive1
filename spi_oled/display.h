// Copyright (c) 2017 Wladimir J. van der Laan
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef H_DISPLAY

#include <stdint.h>

/** Display width in pixels */
#define DISP_W 128
#define DISP_MEM_W 132
#define DISP_W_OFFS ((DISP_MEM_W-DISP_W)/2)
/** Display height in pixels */
#define DISP_H 64

/********* Wiring **********
 *
 * GPIO Pin PPin Oled
 * 23   7   1    CS    chip select if low
 * 3    11  2    SDIN
 * 5    13  4    SCLK
 * 2    10  7    D/C   command if low, data if high
 * 0    8   8    RES   reset if low
 * 1    9   9    Vbatc power control display; active-low
 * 4    12  10   Vddc  power control logic; active-low
 * -    Gnd 6/11 GND   ground
 * -    3v3 7/12 VCC   power supply
 *
 * GPIO is the SoC GPIO number
 * Pin is the pin on the HiFive board.
 * PPin is the pin on the PMOD connector. These are laid out as
 *
 *  \/
 * +-------------------+
 * | 1  2  3  4  5  6  |
 * | 7  8  9  10 11 12 |
 * +-------------------+
 *
 * Oled is the name of the pin at the side of the PMODoled module
 *
 ** Hard constraints on wiring:
 *
 * To use SPI controller instead of bitbanging,
 *   Pin 10 should be connected to 7/DC
 *   Pin 11 should be connected to 2/SDIN
 *   Pin 13 should be connected to 4/SDLK
 *
 ** SPI Setup
 *
 * SPI_REG_SCKDIV
 *   Fsck = Fin/(2*(div+1))   Fin=tlclk=cpuclk=16 MHz
 *   So at least 1, which puts the clock rate at 8 MHz (of max 10 MHz)
 *
 * SPI_REG_CSMODE
 *   mode
 *     3 OFF Disable hardware control of the CS pin
 * SPI_REG_SCKMODE
 *   pol
 *     0 Inactive state of SCLK is logical 0
 *   pha
 *     0 Value of SDIN is sampled at SCLK's rising edge so shift out a new bit at falling edge
 * SPI_REG_FMT
 *   proto
 *     0 single
 *   endian
 *     0 Data is clocked from bit 7 (MSB) to bit 0 (LSB)
 *   dir
 *     1 Tx: the receive fifo is not populated
 *   len
 *     8 bits per frame
 */
#define OLED_CS	   10
#define OLED_CS_OFS 0	//Chip select offset -  0 = pin 10(2), 1 = ?, 2 = 15(9), 3 = 16(10)
#define OLED_SDIN  11
#define OLED_SCLK  13
#define OLED_RES   16
#define OLED_DC    15


/** Initialize pmodoled module */
void oled_init();
/** Initialize SPI */
void spi_init(void);
/** write a byte to OLED spi */
void spi(uint8_t data);
/** wait for completion of queued spi bytes */
void spi_complete(void);
/** set mode to data */
void mode_data(void);
/** set mode to commands */
void mode_cmd(void);

void setDisplayOn(uint8_t on);
void setChargePumpVoltage(uint8_t voltage);
void invertColor(uint8_t invert);
void setEntireDisplayOn(uint8_t allWhite);
void setDisplayStartLine(uint8_t startline);
void setDisplayOffset(uint8_t something);
void flipDisplay(uint8_t flip);
void setContrast(uint8_t contrast);
void fadeIn(uint64_t millis);
void fadeOut(uint64_t millis);

void set_x(unsigned col);
void set_y(unsigned row);
void set_xy(unsigned col, unsigned row);
/** clear (visible portion of) screen, reset pointers */
void oled_clear(void);

#endif
