#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdint.h>

/* Configures the clock to use HSE (external oscillator) and the PLL
	to get SysClk == AHB == APB1 == APB2 == 36MHz */
void configClock(void);

/* Configures USART2 to the specified baud rate */
void configUSART2(uint32_t baud);

#endif /* _UTILS_H_ */
