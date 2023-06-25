// Owner  :
//           Saar Gozlan ID: 204188403  Oren Danilov  ID: 203824545
// Project:  efm32_ARM_Engine_control
//=============================================================================
/////////////// include, define, signatures ///////////////
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_timer.h"
#include "em_gpio.h"
#include "em_usart.h"

#define OUT_FREQ 1000 // timer3(PWM) Desired frequency [Hz].
#define DUTY_CYCLE 75 // PWM Duty Cycle %
#define timer3_FREQ 28000000 // timer3 system frequency [Hz]

void Device_Init(void);

bool Button_0();
bool Button_1();
void Engine_driver_Control(uint8_t state);
uint8_t bluetooth_handler(uint8_t state);
void Move_forward();
void Move_back();
void Move_right();
void Move_left();
void Stop();
void PWN_Change_dutycycle(uint8_t speed_state);
uint8_t obstacle_detection(uint8_t state);

/////////////// main ///////////////////
int main() {
  CHIP_Init();
  Device_Init();
  uint8_t state,next_state ,speed_state ,speed_next_state ;
  state='S'; // initlize engines state to stop
  next_state='S';
  speed_state='0';
  speed_next_state='0';
  while (1){
      if(Button_0()){
        while(1){
            uint8_t tmp;
            tmp=bluetooth_handler(state); // USER INTERFACE CONTORL
            if(tmp >='0' && tmp <= '9')
              speed_next_state=tmp;
            else if(tmp >='A' && tmp <= 'Z')
              next_state=tmp;

          if(state!='S')//USE THE MACHINE SENESORS
              next_state=obstacle_detection(next_state);

            if(Button_1()){ // return to idle
              next_state='S';
              state=next_state;
              Engine_driver_Control(state);//FORCE STOP
              break;
            }
            if(next_state == 'R' || next_state == 'L')
              speed_next_state='2'; // use low speed for safety when turn right or left
            if((speed_next_state!=speed_state) && (speed_next_state!=0)){
              speed_state=speed_next_state;
              PWN_Change_dutycycle(speed_state);
            }

            if((next_state!=state) && (next_state!=0)){
              state=next_state;
              Engine_driver_Control(state);
            }
      }
    }
  }
}


////////////// functions //////////////
void PWN_Change_dutycycle(uint8_t speed_state){
  int dc;
  if(speed_state == '0')
    dc=75;
  else if(speed_state == '1')
    dc=85;
  else if(speed_state == '2')
    dc=95;
  else
    dc=75;

  uint32_t compare_val;
  compare_val = ((timer3_FREQ*1.0)/(OUT_FREQ*1.0) )*(dc/100.0);
  TIMER_CompareSet(TIMER3, 2, compare_val);    // set cc2 compare value 25 duty cycle.
  TIMER_CompareBufSet(TIMER3, 2, compare_val); // set cc2 compare buffer value 25 duty cycle.
}
void Move_forward(){
  for(int i=1;i<=7;i+=2)
     GPIO_PinOutSet(gpioPortD, i); // port Di = 1
   for(int i=0;i<=7;i+=2)
     GPIO_PinOutClear(gpioPortD, i); // port Di = 0
}
void Move_back(){
  for(int i=1;i<=7;i+=2)
      GPIO_PinOutClear(gpioPortD, i); // port Di = 0
  for(int i=0;i<=7;i+=2)
      GPIO_PinOutSet(gpioPortD, i); // port Di = 1
}
void Move_right(){
  // forward left engines [4,7]
  for(int i=5;i<=7;i+=2)
     GPIO_PinOutSet(gpioPortD, i); // port Di = 1
   for(int i=4;i<=7;i+=2)
     GPIO_PinOutClear(gpioPortD, i); // port Di = 0

    //revers right engines [0,3]
    for(int i=1;i<=3;i+=2)
        GPIO_PinOutClear(gpioPortD, i); // port Di = 0
    for(int i=0;i<=3;i+=2)
        GPIO_PinOutSet(gpioPortD, i); // port Di = 1
}
void Move_left(){
  // forward right engines [0,3]
  for(int i=1;i<=3;i+=2)
     GPIO_PinOutSet(gpioPortD, i); // port Di = 1
   for(int i=0;i<=3;i+=2)
     GPIO_PinOutClear(gpioPortD, i); // port Di = 0

  //revers left engines [4,7]
  for(int i=5;i<=7;i+=2)
      GPIO_PinOutClear(gpioPortD, i); // port Di = 0
  for(int i=4;i<=7;i+=2)
      GPIO_PinOutSet(gpioPortD, i); // port Di = 1
}
void Stop(){
  for(int i=0;i<=7;i++)
      GPIO_PinOutClear(gpioPortD, i); // port Di = 0
}
void Engine_driver_Control(uint8_t state){
  if(state=='S')//S=Stop
    Stop();
  else if(state=='F')//F=Forward
    Move_forward();
  else if(state=='L')//L=Left
    Move_left();
  else if(state=='R')//R=Right
    Move_right();
  else if(state=='V')//V=reVerse
    Move_back();
  else // for every other state we may can get by accident
    Stop();
}
uint8_t bluetooth_handler(uint8_t state){
  //return USART_Rx(USART1);
  if ((UART0->STATUS & UART_STATUS_RXDATAV)) { //change state
      state=(uint8_t)UART0->RXDATA;
  }
  if (state >= 'a' && state<= 'z')
      state-=32; // cast to capital letter
  return state;
}
uint8_t obstacle_detection(uint8_t state){
  //int state;
  //state=0;
  if((GPIO_PinInGet(gpioPortA, 14)==1) && (GPIO_PinInGet(gpioPortB, 15)==1))
    state = 'S'; //block from front and back
  else if(GPIO_PinInGet(gpioPortA, 14)==1)
    state = 'F'; // back _alarm
  else if (GPIO_PinInGet(gpioPortB, 15)==1)
    state = 'V'; // front _alarm
  return state;
}
bool Button_0(){
  if(GPIO_PinInGet(gpioPortA, 12))
    return true;
  else
    return false;

  //return GPIO_PinInGet(gpioPortA, 12);
}
bool Button_1(){
  if(GPIO_PinInGet(gpioPortA, 13))
    return true;
  else
    return false;
}
