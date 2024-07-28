#include "dcdc.h"

pwr_adc_t adc;
pwr_data_t data={.tail=VOFA_TAIL};

void dcdc_init(){
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, SET);

    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc3, ADC_SINGLE_ENDED);
    

    LL_HRTIM_EnableOutput(HRTIM1, LL_HRTIM_OUTPUT_TA1);
    LL_HRTIM_EnableOutput(HRTIM1, LL_HRTIM_OUTPUT_TA2);

    LL_HRTIM_EnableOutput(HRTIM1, LL_HRTIM_OUTPUT_TB1);
    LL_HRTIM_EnableOutput(HRTIM1, LL_HRTIM_OUTPUT_TB2);

    LL_HRTIM_EnableOutput(HRTIM1, LL_HRTIM_OUTPUT_TE1);
    LL_HRTIM_EnableOutput(HRTIM1, LL_HRTIM_OUTPUT_TE2);

    LL_HRTIM_EnableOutput(HRTIM1, LL_HRTIM_OUTPUT_TF1);
    LL_HRTIM_EnableOutput(HRTIM1, LL_HRTIM_OUTPUT_TF2);

    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&(adc.i_motor), 2);
    HAL_ADC_Start_DMA(&hadc2, (uint32_t *)&(adc.i_dcdc), 3);
    HAL_ADC_Start_DMA(&hadc3, (uint32_t *)&(adc.i_fw2), 2);

    HAL_UART_Transmit_DMA(&huart4, (uint8_t *)&data, sizeof(data));

    dcdc_setphase(10.0f);
    dcdc_setduty(45.0f);

    HAL_Delay(100);

    
}

void dcdc_setphase(float percentage){
    int phaseshift=toCompareVal(percentage);

    int phaseA=checkCompVal(DCDC_COMPARE_MINVAL);
    int phaseAsh=checkCompVal((phaseA+phaseshift)%DCDC_PERIOD);
    int phaseB=checkCompVal(DCDC_COMPARE_MINVAL+DCDC_PERIOD/2);
    int phaseBsh=checkCompVal((phaseB+phaseshift)%DCDC_PERIOD);
    /*
    V1 -> E(3)
    C1 -> B(2)
    ---------
    V2 -> F(4)
    C2 -> A(1)
    */
   LL_HRTIM_TIM_SetCompare3(HRTIM1, LL_HRTIM_TIMER_MASTER, phaseA);
   LL_HRTIM_TIM_SetCompare2(HRTIM1, LL_HRTIM_TIMER_MASTER, phaseAsh);

   LL_HRTIM_TIM_SetCompare4(HRTIM1, LL_HRTIM_TIMER_MASTER, phaseB);
   LL_HRTIM_TIM_SetCompare1(HRTIM1, LL_HRTIM_TIMER_MASTER, phaseBsh);
    return;
}

void dcdc_setduty(float duty){
    float leftduty,rightduty;
    if(duty<0.1f){
        duty=0.1f;
    }else if(duty>100.0f){
        duty=100.0f;
    }

    if(duty<50.0f){
        leftduty=duty*2.0f;
        rightduty=100.0f;
    }else{
        leftduty=100.0f;
        rightduty=(100.0f-duty)*2.0f;
    }

    int comp_left=toCompareVal(leftduty);
    comp_left=checkCompVal(comp_left);

    int comp_right=toCompareVal(rightduty);
    comp_right=checkCompVal(comp_right);

    LL_HRTIM_TIM_SetCompare1(HRTIM1, LL_HRTIM_TIMER_E, comp_left);
    LL_HRTIM_TIM_SetCompare1(HRTIM1, LL_HRTIM_TIMER_F, comp_left);

    LL_HRTIM_TIM_SetCompare1(HRTIM1, LL_HRTIM_TIMER_A, comp_right);
    LL_HRTIM_TIM_SetCompare1(HRTIM1, LL_HRTIM_TIMER_B, comp_right);

    LL_HRTIM_EnableIT_REP(HRTIM1, LL_HRTIM_TIMER_MASTER);
}

int checkCompVal(int val){
    if(val<DCDC_COMPARE_MINVAL){
        return DCDC_COMPARE_MINVAL;
    }else if(val>DCDC_PERIOD-100){
        return DCDC_PERIOD-100;
    }else{
        return val;
    }
}

static void adc_value_conversion(void){
    float i_motor=(adc.i_motor*(V_REF/4095.f)-INA181_REF)*(IMOTOR_CAL*2000.0f/100.0f);
    data.i_motor=i_motor*IIR_C+data.i_motor*(1.0f-IIR_C);

    float i_dcdc=(adc.i_dcdc*(V_REF/4095.f)-INA181_REF)*(2000.0f/100.0f);
    data.i_dcdc=i_dcdc*IIR_C+data.i_dcdc*(1.0f-IIR_C);

    float i_tot=(adc.i_tot*(V_REF/4095.f)-INA181_REF)*(1000.0f/100.0f);
    data.i_tot=i_tot*IIR_C+data.i_tot*(1.0f-IIR_C);

    float v_bus=adc.v_bus*(11.0f*V_REF/4095.f);
    data.v_bus=v_bus*IIR_V+data.v_bus*(1.0f-IIR_V);

    float v_cap=adc.v_cap*(11.0f*V_REF/4095.f);
    data.v_cap=v_cap*IIR_V+data.v_cap*(1.0f-IIR_V);
    return;
}

void dcdc_mainISR(void){
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    adc_value_conversion();
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    return;
}
