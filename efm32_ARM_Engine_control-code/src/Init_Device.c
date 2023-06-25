
/////////////// init device //////////////////////////

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
void Set_Clock_Gpio(void);
void PWM_Init(void);
void UART_Init(void);
void Engine_Init(void);
void Stop();


void Device_Init(void) {
   Set_Clock_Gpio();
   Engine_Init();
   UART_Init();
}
void Set_Clock_Gpio(void){
  CMU_HFRCOBandSet(cmuHFRCOBand_28MHz); // Set HFRCOCTRL (High Freq. RCOsc.) to 1 MHz
  CMU_ClockEnable(cmuClock_GPIO, true); // Enable GPIO peripheral clock
}
void PWM_Init(void){
  CMU_ClockEnable(cmuClock_TIMER3, true);   // enable Timer3 clock.
  GPIO_PinModeSet(gpioPortE, 2, gpioModePushPull, 0); // Configure port E pin 2 as digital output (push-pull)

  uint32_t top;
  uint32_t compare_val;
  top=timer3_FREQ/OUT_FREQ;
  compare_val = ((timer3_FREQ*1.0)/(OUT_FREQ*1.0) )*(DUTY_CYCLE/100.0);

  TIMER_TopSet (TIMER3,top);                   // set top timer 3.
  TIMER_CounterSet(TIMER3, 0);                 // start counter from 0 up-count mode.
  TIMER_CompareSet(TIMER3, 2, compare_val);    // set cc2 compare value 25 duty cycle.
  TIMER_CompareBufSet(TIMER3, 2, compare_val); // set cc2 compare buffer value 25 duty cycle.
  TIMER3->ROUTE = (1 << 16)| (1 << 2) ;   // connect pwm output to port E pin 2.

  // setup timer channel configuration for pwm.
  TIMER_InitCC_TypeDef timerCCInit =
      {
        .eventCtrl=timerEventEveryEdge,
        .edge=timerEdgeNone,
        .prsSel=timerPRSSELCh0,
        .cufoa=timerOutputActionNone,
        .cofoa=timerOutputActionSet,
        .cmoa=timerOutputActionClear,
        .mode=timerCCModePWM,
        .filter=false,
        .prsInput= false,
        .coist=false,
        .outInvert=false,
      };
  //configuration for timer3 on channel 2.
  TIMER_InitCC(TIMER3, 2, &timerCCInit);
    // setup timer configuration for pwm.
    TIMER_Init_TypeDef timerPWMInit =
    {
      .enable= true,
      .debugRun=true,
      .prescale=timerPrescale1,
      .clkSel=timerClkSelHFPerClk,
      .fallAction=timerInputActionNone,
      .riseAction=timerInputActionNone,
      .mode=timerModeUp,
      .dmaClrAct=false,
      .quadModeX4=false,
      .oneShot=false,
      .sync=false,
    };
   // pwm configuration for timer3.
    TIMER_Init(TIMER3, &timerPWMInit);
}
void UART_Init(void){
  CMU_ClockEnable(cmuClock_GPIO, true);
  CMU_ClockEnable(cmuClock_UART0, true);

  GPIO_PinModeSet(gpioPortE, 0, gpioModePushPull, 1); // U0_TX  is push pull
  GPIO_PinModeSet(gpioPortE, 1, gpioModeInput, 1);    // U0_RX  is input

  // Start with default config, then modify as necessary
  USART_InitAsync_TypeDef config = USART_INITASYNC_DEFAULT;

  config.baudrate = 9600; // CLK freq is 1 MHz
  config.autoCsEnable = true; // CS pin controlled by hardware, not firmware

  config.enable = usartDisable; // Make sure to keep USART disabled until it's all set up
  USART_InitAsync(UART0, &config);

  // Set and enable USART pin locations
  UART0->ROUTE = UART_ROUTE_CLKPEN | UART_ROUTE_CSPEN | UART_ROUTE_TXPEN | UART_ROUTE_RXPEN | UART_ROUTE_LOCATION_LOC1;

  USART_Enable(UART0, usartEnable);
}
void Engine_Init(void){
  PWM_Init();
  for(int i=0;i<=7;i++)
    GPIO_PinModeSet(gpioPortD, i, gpioModePushPull, 0); // Configure port D pin i as digital output (push-pull)

  /* Configure port A pin 12 as button 0
     Configure port A pin 13 as button 1
     Configure port A pin 14 as alarm_Front */
  for(int i=12;i<=14;i++)
    GPIO_PinModeSet(gpioPortA, i, gpioModeInput, 0);

  // Configure port B pin 15 as alarm_back
  GPIO_PinModeSet(gpioPortB, 15, gpioModeInput, 0);

  Stop();
}
