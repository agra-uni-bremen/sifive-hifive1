#include "display.h"

#include <stdio.h>
#include <stdlib.h>
#include "platform.h"

#include "helpers.h"

/** SPI speed cannot exceed 10MHz for SSD1306 */
//#define MAX_SPI_FREQ (10000000)
#define MAX_SPI_FREQ   (10000000)

void spi_init(void)
{

	//Pins enable output
	GPIO_REG(GPIO_INPUT_EN)   &= ~(1 << mapPinToReg(OLED_SDIN) |
									1 << mapPinToReg(OLED_SCLK) |
									1 << mapPinToReg(OLED_CS));
	GPIO_REG(GPIO_OUTPUT_EN)  |= (1 << mapPinToReg(OLED_SDIN) |
									1 << mapPinToReg(OLED_SCLK) |
									1 << mapPinToReg(OLED_CS));

    // Select IOF SPI1.MOSI [SDIN] and SPI1.SCK [SLCK] and SPI1.SS0 [CS]
    GPIO_REG(GPIO_IOF_SEL)    &= ~((1 << mapPinToReg(OLED_SDIN)) |
    								(1 << mapPinToReg(OLED_SCLK)) |
									(1 << mapPinToReg(OLED_CS)));

    GPIO_REG(GPIO_IOF_EN )    |=  ((1 << mapPinToReg(OLED_SDIN)) |
    								(1 << mapPinToReg(OLED_SCLK)) |
    								(1 << mapPinToReg(OLED_CS)));

    GPIO_REG(GPIO_OUTPUT_VAL) |=  ((1 << mapPinToReg(OLED_SDIN)) |
    								(1 << mapPinToReg(OLED_SCLK)) |
									(1 << mapPinToReg(OLED_CS))); /* is this necessary? */


    // Set up SPI controller
    /** SPI clock divider: determines the speed of SPI
     * transfers. This cannot exceed 10Mhz for the SSD1306.
     * CPUfreq is set to 16Mhz in this demo.
     * The formula is CPU_FREQ/(1+SPI_SCKDIV)
     */
    SPI1_REG(SPI_REG_SCKDIV)    = (get_cpu_freq() / MAX_SPI_FREQ) - 1;
    SPI1_REG(SPI_REG_SCKMODE)   = 0; /* pol and pha both 0 - SCLK is active-high, */
    SPI1_REG(SPI_REG_CSID)      = OLED_CS_OFS; /* CS 0 */
    SPI1_REG(SPI_REG_CSDEF)     = 0xffff; /* CS is active-low */
    SPI1_REG(SPI_REG_CSMODE)    = SPI_CSMODE_HOLD; /* hold CS where possible */
    /* SPI1_REG(SPI_REG_DCSSCK)    = */
    /* SPI1_REG(SPI_REG_DSCKCS)    = */
    /* SPI1_REG(SPI_REG_DINTERCS)  = */
    /* SPI1_REG(SPI_REG_DINTERXFR) = */
    SPI1_REG(SPI_REG_FMT)       = SPI_FMT_PROTO(SPI_PROTO_S) | SPI_FMT_ENDIAN(SPI_ENDIAN_MSB) | SPI_FMT_DIR(SPI_DIR_TX) | SPI_FMT_LEN(8);
    SPI1_REG(SPI_REG_TXCTRL)    = 1; /* interrupt when <1 in tx fifo (completion) */
    /* SPI1_REG(SPI_REG_RXCTRL)    = */
    /* SPI1_REG(SPI_REG_IE)        = */
}

void spi(uint8_t data)
{
    while (SPI1_REG(SPI_REG_TXFIFO) & SPI_TXFIFO_FULL)
        sleep_u(1);
    SPI1_REG(SPI_REG_TXFIFO) = data;
    //sleep_u(100);
    //printf("%02x\n", data);
}

void spi_complete()
{
    // Wait for interrupt condition.
    // It would be more efficient to use an actual interrupt here.
    while (!(SPI1_REG(SPI_REG_IP) & SPI_IP_TXWM))
    	sleep_u(1000);
    sleep_u(1000);
}

void mode_data(void)
{
    spi_complete(); /* wait for SPI to complete before toggling */
    //SPI1_REG(SPI_REG_CSID)      = 1; /* CS 1 */
    setPin(OLED_DC, 1);
    sleep(10);
}

void mode_cmd(void)
{
    spi_complete(); /* wait for SPI to complete before toggling */
    //SPI1_REG(SPI_REG_CSID)      = 0; /* CS 0 */
    setPin(OLED_DC, 0);
    sleep(10);
}

void setDisplayOn(uint8_t on)
{
	spi(0xAE | (on & 1));
}

void setChargePumpVoltage(uint8_t voltage)
{
	 spi(0b00110000 | (voltage & 0b11));
}

void invertColor(uint8_t invert)
{
	spi(0b10100110 | (invert & 1));	//set 'normal' direction (1 = bright)
}

void setEntireDisplayOn(uint8_t allWhite)
{
	spi(0b10100100 | (allWhite & 1));
}

void setDisplayStartLine(uint8_t startline)
{
	spi(0b01000000 | (startline & 0b111111));
}

void setDisplayOffset(uint8_t something)
{
    spi(0b11010011);	//double byte to set display offset to (y)0;
    spi(0b00000000);	//double byte to set display offset to (y)0;
}

/** Initialize pmodoled module */
void oled_init()
{
    puts("OLED controller\r\n");
    spi_init();
    puts("spi initied\r\n");

    // Initial setup
    //
    //Enable RESET and D/C Pin
	GPIO_REG(GPIO_INPUT_EN)   &= ~(1 << mapPinToReg(OLED_RES) |
									1 << mapPinToReg(OLED_DC));
	GPIO_REG(GPIO_OUTPUT_EN)  |= (1 << mapPinToReg(OLED_RES) |
									1 << mapPinToReg(OLED_DC));

	//RESET
	setPin(OLED_RES,0);
	sleep_u(100);		//min 10us
	setPin(OLED_RES,1);
	sleep_u(200);		//'about 100ms'

    // Command mode
	mode_cmd();
    spi_complete();
    // Reset
    // Initialize display to desired operating mode.

    setChargePumpVoltage(0b10);
    invertColor(0);
    setDisplayOn(1);		//later on, do this after init

    // 4. Clear screen (entire memory)
    oled_clear();
    puts("display on\r\n");

}

void set_x(unsigned col)
{
    mode_cmd();
    spi(0x00 | ((col+3) & 0xf));
    //printf("set_x1: %02X\r\n", (col & 0xf));
    spi(0x10 | (((col+3) >> 4)&0xf));
    //printf("set_x2: %02X\r\n", 0x10 | ((col >> 4)&0xf));
    mode_data();
}
void set_y(unsigned row)
{
    mode_cmd();
    spi(0xb0 | (row & 0x7));
    //printf("set_y: %02X\r\n", 0xb0 | (row & 0x7));
    mode_data();
}

void oled_clear(void)
{
    set_x(0);
    set_y(0);

    for (unsigned x=0; x < DISP_W; ++x) {
    	for (unsigned y = 0; y < DISP_H/8; ++y) {
    		spi(0x00);
    	}
    }
}


