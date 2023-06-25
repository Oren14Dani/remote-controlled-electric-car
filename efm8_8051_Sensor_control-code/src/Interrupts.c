// Owner  :  Oren Danilov  ID: 203824545
//           Saar Gozlan   ID: 204188403
// Project:  project_middle
//=========================================================
#include "general.h"
extern uint32_t button0_pressed;
extern uint32_t button1_pressed;
extern uint32_t in_proccess;
//-----------------------------------------------------------------------------
// INT0_ISR
//-----------------------------------------------------------------------------
// INT0 ISR Content goes here. Remember to clear flag bits:
// TCON::IE0 (External Interrupt 0)
//-----------------------------------------------------------------------------
SI_INTERRUPT (INT0_ISR, INT0_IRQn)
  {
    TCON_IE0 = 0;
    button0_pressed = 1;
    button1_pressed = 0;
    in_proccess = 1; // to indicate start periodic proccess
    button0_out=1;
    button1_out=0;
  }

//-----------------------------------------------------------------------------
// INT1_ISR
//-----------------------------------------------------------------------------
// INT1 ISR Content goes here. Remember to clear flag bits:
// TCON::IE1 (External Interrupt 1)
//-----------------------------------------------------------------------------
SI_INTERRUPT (INT1_ISR, INT1_IRQn)// button1 - stop priodic mesure process
  {
    TCON_IE1 = 0;
    button1_pressed = 1;
    button0_pressed = 0;
    in_proccess = 0; // to indicate end periodic proccess
    button0_out=0;
    button1_out=1;
  }


