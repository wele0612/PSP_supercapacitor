#include "dcdc.h"
#include "cap_canmsg_protocal.h"

pwr_adc_t adc;
pwr_data_t data={.tail=VOFA_TAIL};

int state=CAP_OFF;
uint32_t protection_triggered=0;

float set_current=2.5f;  //current to be achieved by PID
float target_current=1.0f; //dcdc-current required due to power limit
float total_allow_current=3.0f; //total-current required due to power limit
float power_limit=30.0f; //power limit

float discharge_maxi;

PID_t _i={
    .p=0.0f,
    .i=30000.0f,
    .d=0.0f,
    .i_max=0.01f,
    .errm1=0.0f,
    .err_i=0.0f,
};

/**
 * @brief Turn on the MOS driver circuit, enable DCDC. 
 * 开启DCDC电路。
 */
void dcdc_on(){
    HAL_GPIO_WritePin(DRV_EN_GPIO_Port, DRV_EN_Pin, SET);
}
/**
 * @brief Turn off the MOS driver circuit, disable DCDC. 
 * 关闭DCDC电路。
 */
void dcdc_off(){
    HAL_GPIO_WritePin(DRV_EN_GPIO_Port, DRV_EN_Pin, RESET);
}


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

    HAL_Delay(10);

    LL_HRTIM_EnableIT_REP(HRTIM1, LL_HRTIM_TIMER_MASTER);

    state=CAP_READY;
}

__STATIC_INLINE int checkCompVal(int val){
    if(val<DCDC_COMPARE_MINVAL){
        return DCDC_COMPARE_MINVAL;
    }else if(val>DCDC_PERIOD-100){
        return DCDC_PERIOD-100;
    }else{
        return val;
    }
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
        leftduty=duty*2.05f;
        rightduty=100.0f;
    }else{
        leftduty=100.0f;
        rightduty=(100.0f-duty)*2.05f;
    }

    int comp_left=toCompareVal(leftduty);
    comp_left=checkCompVal(comp_left);

    int comp_right=toCompareVal(rightduty);
    comp_right=checkCompVal(comp_right);

    LL_HRTIM_TIM_SetCompare1(HRTIM1, LL_HRTIM_TIMER_E, comp_left);
    LL_HRTIM_TIM_SetCompare1(HRTIM1, LL_HRTIM_TIMER_F, comp_left);

    LL_HRTIM_TIM_SetCompare1(HRTIM1, LL_HRTIM_TIMER_A, comp_right);
    LL_HRTIM_TIM_SetCompare1(HRTIM1, LL_HRTIM_TIMER_B, comp_right);
}

/**
 * @brief Reset integral term of DCDC. 
 * 重置积分项。
 */
__STATIC_INLINE void pid_reset(){
    _i.err_i= 0;
}

/**
 * @brief Calculate PID for next period.
 * @note This PID controls 
 */
__STATIC_INLINE void update_pid(){

    /// P and D disabled for now

    float i_err=set_current-data.i_dcdc;
    _i.err_i += i_err*DT;
    // float err_d=(i_err-_i.errm1)*(1.0f/DT);
    // _i.errm1=i_err;

    if(_i.err_i > _i.i_max) _i.err_i=_i.i_max;
    if(_i.err_i < -_i.i_max) _i.err_i=-_i.i_max;
    
    //dcdc_setduty(i_err*_i.p + _i.err_i*_i.i + err_d*_i.d);
    dcdc_setduty(_i.err_i*_i.i);
}

/**
 * @brief State machine implementation of DCDC module.
 * 
 */
static void cap_state_machine(){
    //data.state=(float)state*10;

    if(data.v_bus > BUS_OVP_THRE){ //BUS over-voltage protection
        state=VBUS_OVP;
        protection_triggered=HAL_GetTick();
    }else if(data.v_bus < BUS_UVP_THRE){ //BUS under-voltage halt
        state=VBUS_UVP;
        protection_triggered=HAL_GetTick();
    }else if(data.v_cap > BAT_OVP_THRE){ //BAT over-voltage protection
        state=VBAT_OVP;
        //data.testval=data.v_cap;// debug display output
        protection_triggered=HAL_GetTick();
    }else if(state==VBUS_UVP && data.v_bus > BUS_UVP_THRE \
        && protection_triggered < HAL_GetTick() - 50){ 
        //recovery from BUS UVP
        pid_reset();
        protection_triggered=HAL_GetTick();
        state=CAP_READY;
    }

    //state-transition of DCDC FSM
    switch (state)
    {
    case CAP_ON:
        update_pid();
        break;
    case CAP_READY:
        dcdc_on();
        state=CAP_ON;
        break;
    case CAP_OFF: //本行疑似bug。暂时似乎没有影响。
    case VBUS_OVP:
    case VBAT_OVP:
    //进入保护后，系统会每隔 PROTECTION_RECOVERY_TIME 毫秒尝试重启。
        if(HAL_GetTick() > protection_triggered + PROTECTION_RECOVERY_TIME){
            pid_reset();
            state=CAP_READY;
        }
    case VBUS_UVP:
        dcdc_off();
        break;
    default:
        state=CAP_OFF;
    }
}

__STATIC_INLINE void adc_value_conversion(void){
    float i_motor=(adc.i_motor*(V_REF/4095.f)-INA181_REF)*(IMOTOR_CAL*2000.0f/100.0f);
    data.i_motor=i_motor*IIR_C+data.i_motor*(1.0f-IIR_C);

    float i_tot=(adc.i_tot*(V_REF/4095.f)-INA181_REF)*(ITOT_CAL*1000.0f/100.0f);
    data.i_tot=i_tot*IIR_C+data.i_tot*(1.0f-IIR_C);

    // float i_dcdc=(adc.i_dcdc*(V_REF/4095.f)-INA181_REF)*(2000.0f/100.0f);
    // data.i_dcdc=i_dcdc*IIR_C+data.i_dcdc*(1.0f-IIR_C);
    data.i_dcdc=data.i_tot-data.i_motor;

    float v_bus=adc.v_bus*(11.0f*V_REF/4095.f);
    data.v_bus=v_bus*IIR_V+data.v_bus*(1.0f-IIR_V);

    float v_cap=adc.v_cap*(11.0f*V_REF/4095.f);
    data.v_cap=v_cap*IIR_V+data.v_cap*(1.0f-IIR_V);
    return;
}

void dcdc_update_power_limit(float limit){
    if(limit<POWER_LIMIT_MINIMUM){
        limit=POWER_LIMIT_MINIMUM;
    }else if(limit>POWER_LIMIT_MAXIMUM){
        limit=POWER_LIMIT_MAXIMUM;
    }
    power_limit=limit;
}

void dcdc_mainISR(void){
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    adc_value_conversion();

    total_allow_current=power_limit/data.v_bus;
    data.i_allow=total_allow_current;
    target_current=total_allow_current-data.i_motor;
    //calculate target current

    float lim_judge=CAP_MAX_CURRENT*(data.v_cap/data.v_bus);
    float lim_capfull=(BAT_FULL_VOL-data.v_cap)*7.0f;
    float lim_caplow=(data.v_cap-BAT_UVP_STARTUP_THRE)*5.0f;
    
    float charge_maxi=lim_judge<lim_capfull?lim_judge:lim_capfull;
    discharge_maxi=lim_judge<lim_caplow?lim_judge:lim_caplow;
    if(charge_maxi < 0.0f)charge_maxi=0.0f;
    if(discharge_maxi < -0.25f)discharge_maxi=-0.25f; 

    set_current=target_current;
    if(set_current>charge_maxi){
        set_current=charge_maxi;
    }else if(set_current<(-discharge_maxi)){
        set_current=-discharge_maxi;
    }

    HAL_IWDG_Refresh(&hiwdg);

    cap_state_machine();

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    return;
}
