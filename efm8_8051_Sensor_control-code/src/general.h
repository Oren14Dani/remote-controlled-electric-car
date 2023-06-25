// Owner  :  Oren Danilov  ID: 203824545
//           Saar Gozlan   ID: 204188403
// Project:  efm8_8051_Sensor_control
//=========================================================

#ifndef GENERAL_H
#define GENERAL_H
//-----------------------------------------------------------------------------
// Includes and defines
//-----------------------------------------------------------------------------
#include <SI_EFM8UB2_Register_Enums.h>   // SFR declarations
#include "InitDevice.h"
#include "bsp.h"
#include "disp.h"
#include "tick.h"
#include "render.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//-----------------------------------------------------------------------------
// defines
//-----------------------------------------------------------------------------
#define TRIGGER_DELAY 0.5 // between trigger pulses [m sec]
#define BAUD_RATE 9600 // UART frequency [1/sec]
#define MINIMAL_DISTANCE 15 // [cm]
#define ALARM_STRING "SOS!"

//-----------------------------------------------------------------------------
// pins definition
// ----------------------------------------------------------------------------
SI_SBIT (button0            , SFR_P0, 2); // P0.2
SI_SBIT (button1            , SFR_P0, 3); // P0.3
SI_SBIT (button0_out        , SFR_P2, 4); // P2.4
SI_SBIT (button1_out        , SFR_P2, 5); // P2.5
SI_SBIT (trigger_signal     , SFR_P3, 0); // P3.0
SI_SBIT (echo_signal        , SFR_P3, 1); // P3.1
SI_SBIT (trigger_signal_b   , SFR_P3, 2); // P3.2
SI_SBIT (echo_signal_b      , SFR_P3, 3); // P3.3
SI_SBIT (alert_signal_b     , SFR_P3, 4); // P3.4
SI_SBIT (alert_signal_f     , SFR_P3, 5); // P3.5
SI_SBIT (buzzer             , SFR_P1, 6); // P1.6
SI_SBIT (UARTtx_BTrx        , SFR_P0, 4); // P0.4

/* SPI:
       P0.1 - CS (Active High)
       P0.6 - SCK
       P1.0 - MOSI
       P1.4 - Display enable
*/

//-----------------------------------------------------------------------------
// functions signature
// ----------------------------------------------------------------------------
uint32_t triggerNcapture_back();
uint32_t triggerNcapture_front();
void waitingNbuzzerTon(uint32_t ton_state);
bool check_distance_change(uint32_t new_val, uint32_t old_val);
void alaram_func(bool emeregency_f, bool emeregency_b);
bool alaram_state(uint32_t distance_mesauered);
uint32_t buzzer_ton_state(uint32_t distance_mesauered);
void toggle_buzz();
uint32_t power_in2(uint32_t val);
//void distance_int_to_text(uint32_t distance_mesauered,char* str);
//void print_LCD(uint32_t distance_mesauered_f,uint32_t distance_mesauered_b);
#endif
