#ifndef __DCDC__H
#define __DCDC__H

#include "adc.h"
#include "gpio.h"
#include "hrtim.h"
#include "usart.h"
#include <stdint.h>

#define DCDC_PERIOD 15542
#define DCDC_COMPARE_MINVAL (700)

#define toCompareVal(p) ((int)((p/100.0f)*(float)DCDC_PERIOD))

#define IIR_V 0.05f
#define IIR_C 0.2f
#define INA181_REF 1.25f
#define IMOTOR_CAL 0.83f

#define V_REF (3.00f)

typedef struct pwr_adc_t{
    uint16_t i_motor;   //ADC1_IN3 (PA2)
    uint16_t v_bus;     //ADC1_IN4 (PA3)
    uint16_t i_dcdc;    //ADC2_IN13(PA5)
    uint16_t i_tot;     //ADC2_IN5 (PC4)
    uint16_t v_cap;     //ADC2_IN12(PB2)
    uint16_t i_fw2;     //ADC3_IN12(PB0)
    uint16_t i_fw1;     //ADC3_IN1 (PB1)
}pwr_adc_t;

#define VOFA_TAIL {0x00, 0x00, 0x80, 0x7f}
typedef struct pwr_data_t
{
    float v_bus;
    float v_cap;
    float i_tot;
    float i_dcdc;
    float i_motor;
    unsigned char tail[4];
}pwr_data_t;

void dcdc_init(void);
void dcdc_setphase(float percentage);
void dcdc_setduty(float duty);
int checkCompVal(int val);

void dcdc_mainISR(void);




#endif