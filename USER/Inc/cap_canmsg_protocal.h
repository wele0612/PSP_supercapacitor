/**
 * @file cap_canmsg_protocal.h
 * @author The Great Lakes
 * @brief The protocal defination for super capacitor controller.
 * @version 1.0
 * @date 2024-07-29
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef __CAP_CANMSG_PROTOCAL_H
#define __CAP_CANMSG_PROTOCAL_H

#include <stdint.h>

/* Message send to capacitor module */
#define CAPCAN_RXMSG_ID 0x2C7  //CAN ID

//ALL UNITS IN watt, multiplies by 100

typedef struct capcan_rx_t{
    uint16_t power_target;
    uint16_t referee_power;
    uint16_t rsvd1; //Must be 0x2012
    uint16_t rsvd2; //Must be 0x0712
}capcan_rx_t;


/*Message come from capacitor module */
/*Expected message frequency = 100Hz */
#define CAPCAN_TXMSG_ID 0x2C8  //CAN ID

typedef struct capcan_tx_t{
    uint16_t max_discharge_power;
    uint16_t base_power;
    int16_t cap_energy_percentage;
    uint16_t cap_state;
}capcan_tx_t;

#endif