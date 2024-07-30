#include "g474_fdcan.h"
#include "dcdc.h"
#include "cap_canmsg_protocal.h"

FDCAN_TxHeaderTypeDef TxHeader1;
FDCAN_TxHeaderTypeDef TxHeader2;
FDCAN_TxHeaderTypeDef TxHeader3;


extern pwr_data_t data;
extern int state;
extern float discharge_maxi;
//const uint8_t testdata[8]={0x00,0x66,0xCC,0xFF};
capcan_tx_t txmsg;
capcan_rx_t rxmsg;

volatile uint8_t recv_num=233;

FDCAN_FilterTypeDef sFilterConfig1;
FDCAN_FilterTypeDef sFilterConfig2;

/* USER CODE BEGIN 4 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs){
  static FDCAN_RxHeaderTypeDef rx_header;
  HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, (uint8_t *)&rxmsg);
  if(*(&rx_header.Identifier) == CAPCAN_TXMSG_ID){
    dcdc_update_power_limit(((float)rxmsg.power_target)/100.0f);
  }
}

void fdcan2_config(void)
{
  sFilterConfig1.IdType = FDCAN_STANDARD_ID;
  sFilterConfig1.FilterIndex = 0;
  sFilterConfig1.FilterType = FDCAN_FILTER_RANGE;
  sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  sFilterConfig1.FilterID1 = 0x00;
  sFilterConfig1.FilterID2 = 0x7FF;
  if (HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig1) != HAL_OK)
  {
    Error_Handler();
  }

  sFilterConfig2.IdType = FDCAN_EXTENDED_ID;
  sFilterConfig2.FilterIndex = 0;
  sFilterConfig2.FilterType = FDCAN_FILTER_RANGE;
  sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  sFilterConfig2.FilterID1 = 0x00;
  sFilterConfig2.FilterID2 = 0x1FFFFFFF;
  if (HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2) != HAL_OK)
  {
    Error_Handler();
  }

  /* Configure global filter on both FDCAN instances:
  Filter all remote frames with STD and EXT ID
  Reject non matching frames with STD ID and EXT ID */
  if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan2, FDCAN_REJECT, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE) != HAL_OK)
  {
    Error_Handler();
  }

  /* Activate Rx FIFO 0 new message notification on both FDCAN instances */
  if (HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_BUS_OFF, 0) != HAL_OK)
  {
    Error_Handler();
  }

  /* Configure and enable Tx Delay Compensation, required for BRS mode.
        TdcOffset default recommended value: DataTimeSeg1 * DataPrescaler
        TdcFilter default recommended value: 0 */
  HAL_FDCAN_ConfigTxDelayCompensation(&hfdcan2, hfdcan2.Init.DataPrescaler * hfdcan2.Init.DataTimeSeg1, 0);
  HAL_FDCAN_EnableTxDelayCompensation(&hfdcan2);

  HAL_FDCAN_Start(&hfdcan2);
  HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_NEW_MESSAGE,0);
}

typedef struct {
  uint8_t flag;
  FDCAN_TxHeaderTypeDef TxHeader;
  uint8_t TxData[64];
} FDCAN_SendFailTypeDef;

FDCAN_SendFailTypeDef fdcan1_send_fail = {0};
FDCAN_SendFailTypeDef fdcan2_send_fail = {0};
FDCAN_SendFailTypeDef fdcan3_send_fail = {0};

uint8_t dlc2len[]={0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64};
uint8_t can_dlc2len(uint32_t RxHeader_DataLength)
{
  return dlc2len[RxHeader_DataLength>>16];
}

void fdcan2_transmit(uint32_t can_id, uint32_t DataLength, uint8_t tx_data[])
{
  TxHeader2.Identifier = can_id;
  TxHeader2.IdType = FDCAN_EXTENDED_ID;
  if(can_id < 0x800) {  //exactly not right
    TxHeader2.IdType = FDCAN_STANDARD_ID;
  }
  TxHeader2.TxFrameType = FDCAN_DATA_FRAME;
  TxHeader2.DataLength = DataLength;
  TxHeader2.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  TxHeader2.BitRateSwitch = FDCAN_BRS_ON;
  TxHeader2.FDFormat = FDCAN_CLASSIC_CAN;
  TxHeader2.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  TxHeader2.MessageMarker = 0;	//marker++;	//Tx Event FIFO Use
  if(HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &TxHeader2, tx_data) != HAL_OK) {
    fdcan2_send_fail.flag = 1;
    memcpy(&fdcan2_send_fail.TxHeader, &TxHeader2, sizeof(FDCAN_TxHeaderTypeDef));
    memcpy(fdcan2_send_fail.TxData, tx_data, can_dlc2len(DataLength));
  } 
}

void send_capinfo(){
  txmsg.base_power=(int)(data.i_tot*data.v_bus*100.0f);
  txmsg.cap_state=state;

  float maxi=discharge_maxi<0.1f?0.05f:discharge_maxi;
  txmsg.max_discharge_power=(int)(maxi*data.v_bus*100.0f);

  float all_energy=(BAT_FULL_VOL*BAT_FULL_VOL-BAT_UVP_STARTUP_THRE*BAT_UVP_STARTUP_THRE);
  float remaining_energy=(data.v_cap*data.v_cap-BAT_UVP_STARTUP_THRE*BAT_UVP_STARTUP_THRE);
  txmsg.cap_energy_percentage=(int)(100.0f*remaining_energy/all_energy);
  
  fdcan2_transmit(CAPCAN_RXMSG_ID, FDCAN_DLC_BYTES_8, &txmsg);
}