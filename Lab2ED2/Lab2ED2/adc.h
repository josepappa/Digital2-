/*
 * adc.h
 *
 * Created: 1/22/2026 7:58:01 PM
 *  Author: jargu
 */ 


#ifndef ADC_H_
#define ADC_H_


#include <stdint.h>

void ADC_init(void);
uint16_t ADC_read(uint8_t channel);



#endif /* ADC_H_ */