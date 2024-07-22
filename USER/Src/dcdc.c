#include "dcdc.h"

void dcdc_init(){
    HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1);
    HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA2);

    HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TB1);
    HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TB2);

    HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TE1);
    HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TE2);

    HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TF1);
    HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TF2);

    dcdc_setphase(0.0f);
    dcdc_setduty(20.0f);
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
    __HAL_HRTIM_SetCompare(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_3, phaseA);
    __HAL_HRTIM_SetCompare(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_2, phaseAsh);

    __HAL_HRTIM_SetCompare(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_4, phaseB);
    __HAL_HRTIM_SetCompare(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_1, phaseBsh);
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

    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_E, HRTIM_COMPAREUNIT_1,comp_left);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_F, HRTIM_COMPAREUNIT_1,comp_left);

    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1,comp_right);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_1,comp_right);
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

void dcdc_mainISR(void){
    return;
}
