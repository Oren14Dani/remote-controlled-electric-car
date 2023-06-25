// Owner  :  Oren Danilov  ID: 203824545
//           Saar Gozlan   ID: 204188403
// Project:  project_middle
//=========================================================
//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "general.h"

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
uint32_t delay_end=0;
uint32_t button0_pressed=0;
uint32_t button1_pressed=0;
uint32_t in_proccess=0;

//-----------------------------------------------------------------------------
// SI_SEGMENT_VARIABLE and SiLabs_Startup() Routine
//-----------------------------------------------------------------------------
SI_SEGMENT_VARIABLE(line[DISP_BUF_SIZE], uint8_t, RENDER_LINE_SEG);

void SiLabs_Startup (void)
{
  // Disable the watchdog here
}

//-----------------------------------------------------------------------------
// main
// ----------------------------------------------------------------------------
int main(void)
{
    uint32_t distance_mesauered_b,distance_mesauered_fr;
    uint32_t new_distance_b,new_distance_fr;
    uint32_t ton_state_f,ton_state_b;
    bool b_change,f_change;
    bool emeregency_b,emeregency_f;
    enter_DefaultMode_from_RESET (); // Call hardware initialization routine
    PCA0CN0_CCF0=0;
    PCA0CN0_CCF1=0;
    distance_mesauered_b=0;
    distance_mesauered_fr=0;
    emeregency_b=false;
    emeregency_f=false;
    b_change=false;
    f_change=false;
    ton_state_f=0;
    ton_state_b=0;
    while(1)
    {
        while(button0_pressed==1 || (delay_end && in_proccess)==1 )
        {
            button0_pressed = 0;
            delay_end=0;
            new_distance_b = (triggerNcapture_back() / 58); // calc distance in [cm]
            new_distance_fr = (triggerNcapture_front() / 58); // calc distance in [cm]
            b_change=check_distance_change(new_distance_b, distance_mesauered_b);
            f_change=check_distance_change(new_distance_fr, distance_mesauered_fr);

            if(b_change || f_change)
            {
                if(b_change){
                    distance_mesauered_b = new_distance_b;
                    emeregency_b=alaram_state(distance_mesauered_b);
                    ton_state_b=buzzer_ton_state(distance_mesauered_b);
                }
                if (f_change){
                    distance_mesauered_fr = new_distance_fr;
                    emeregency_f=alaram_state(distance_mesauered_fr);
                    ton_state_f=buzzer_ton_state(distance_mesauered_fr);
                }

                alaram_func(emeregency_b, emeregency_f);// send SOS signsl & turn on buzzer OR turn off buzzer
            }
            if (button1_pressed == 1) // stop the process and avoid the waiting loop
            {
                alert_signal_b = 0;
                alert_signal_f = 0;
                buzzer = 0;
                button1_pressed = 0;
                break;
            }
            if(ton_state_f>ton_state_b)
                waitingNbuzzerTon(ton_state_f);
            else
                waitingNbuzzerTon(ton_state_b);
        }

        if (button1_pressed == 1) //in case the you outside the loop
        {
            buzzer = 0;
            button1_pressed = 0; //avoid false button1_pressed when it will start a new process
        }
    }
}

//-----------------------------------------------------------------------------
// functions
// ----------------------------------------------------------------------------
uint32_t power_of2(uint32_t val) //  2^ton_state
{
  uint32_t x=1;
  while(val!=0){
      x*=2;
      val--;
  }
  return x;
}

void waitingNbuzzerTon(uint32_t ton_state)
{
  uint32_t overflow_counter; // use Tbit_counter to count delay of 0.5 sec
  uint32_t buzz_cnt; // use buzz_cnt to change the buzzer frequncy
  uint32_t scale;
  uint32_t baude_rate_percent;
  if(ton_state>0)
    scale = power_of2(ton_state); // = 2^(ton_state)
  else
    scale=-1;
  buzz_cnt=0;
  overflow_counter=0;
  TCON_TR1=1; // start timer 1

  baude_rate_percent=0.125*BAUD_RATE;
  while(overflow_counter<=baude_rate_percent)//wait 0.0625sec
  {
      if(ton_state>=1 && ton_state<=4){
        if(buzz_cnt == (baude_rate_percent/scale)){ //toggle every 0.5sec/scale
            toggle_buzz();
            buzz_cnt=0;
        }
      }
      while(TCON_TF1==0); // wait for timer1 overflow (52u sec)
      TCON_TF1 = 0; // clear overflow flag
      overflow_counter ++;
      buzz_cnt++;
  }

  TCON_TR1=0; // stop timer 1
  delay_end=1;
}
/*
void distance_int_to_text(uint32_t distance_mesauered, char* str)
{
  uint32_t temp;
  uint32_t temp_inv;
  uint32_t cnt;
  uint32_t i;
  strcpy(str,"000");

  temp = distance_mesauered;
  temp_inv = 0;
  cnt=0;


  if(distance_mesauered >= 300){
      str="300";
      return;
  }
  else if (distance_mesauered <= 3){
      str="003";
      return;
  }

  while (temp != 0) // 145 ---> 541 , 12 ---> 21
    {
      temp_inv = temp_inv*10 + (temp % 10);
      temp /= 10;
      cnt++; // count digit of the number
    }
  i = (3-cnt);
  for(; temp_inv>0; i++) //if cnt = 3 ---> i=0 , if cnt = 3
    {
      str[i] = temp_inv % 10 +'0';
      temp_inv /= 10;
    }
}
*/
void alaram_func(bool emeregency_b, bool emeregency_f)
{
  uint32_t i=0;
  if (emeregency_b == true || emeregency_f == true) // send SOS signsl & turn on buzzer
  {
      if(emeregency_b&&emeregency_f){
          alert_signal_b = 1;
          alert_signal_f = 1;
      }

      else if (emeregency_b == true ){
          alert_signal_b = 1;
          alert_signal_f=0;
      }
      else if (emeregency_f == true){
          alert_signal_b = 0;
          alert_signal_f = 1;
      }
      buzzer = 1;

      //use UART0 to send alarm to the user:
      SCON0_TI=0;
      TCON_TR1=1;
      for(;i<4;i++)
      {
          SBUF0=ALARM_STRING[i];
          while(SCON0_TI==0);
          SCON0_TI=0;
      }
      TCON_TR1=0;
  }

  else{ //turn off buzzer
      buzzer = 0;
      alert_signal_b = 0;
      alert_signal_f = 0;
  }
}

uint32_t triggerNcapture_back()
{
    uint32_t REG_PCA_RISE=0;
    uint32_t REG_PCA_FALL=0;
    uint32_t delta_reg=0;
    uint32_t i=0;

    // ---- trigger ----

    // manual reload value
    TH0 =0xff;
    TL0 =0xf1;

    P3_B0 = 1; // turn on one tact before start timer to ensure 10us pulse minimum
    for(;i<=1;i++){
        TCON_TR0 = 1; // start timer 0
        while (TCON_TF0 == 0); // wait for 10usec
        TCON_TF0 = 0; // clear overflow flag
    }
    P3_B0=0;
    TCON_TF0 = 0; // clear overflow flag
    TCON_TR0= 0; // stop


    // ---- ECHO capture ----

    // reset pca0 register value
    //get ready to catch echo signal rise
    PCA0CPM0=PCA0CPM0_CAPP__ENABLED;

    if(PCA0CN0_CF)
        PCA0CN0_CF=0;
    if(PCA0CN0_CCF0)
        PCA0CN0_CCF0=0;
    PCA0CN0_CR = 1; // start PCA0
    while(PCA0CN0_CCF0==0);//waiting echo rise
    REG_PCA_RISE=PCA0CPH0*0x100 + PCA0CPL0;
    PCA0CN0_CCF0=0;//clear the flag
    PCA0CN0_CF=0;
    //get ready to catch echo signal fall
    PCA0CPM0=PCA0CPM0_CAPN__ENABLED;
    while(PCA0CN0_CCF0==0);//waiting echo fall, delay_end is for security

    REG_PCA_FALL = PCA0CPH0*0x100 + PCA0CPL0; // save how many clocks the ECHO signal run
    PCA0CN0_CR = 0; // stop PCA0
    PCA0CN0_CCF0 = 0;
    if(PCA0CN0_CF)
        PCA0CN0_CF=0;

    // PCA0 clock period is 0.5usec then it can count for about (Oxffff)*0.5usec = 32.7msec
    // we know that the max echo time is about 36ms >  32.7msec => the pca clock start again
    if(REG_PCA_FALL<REG_PCA_RISE)
        delta_reg= 0xffff - REG_PCA_RISE + REG_PCA_FALL;
    else
        delta_reg= REG_PCA_FALL- REG_PCA_RISE;

    // PCA0 clock period is 0.5usec
    delta_reg *= 0.5; //echo_time[usec]
    return delta_reg;
}

uint32_t triggerNcapture_front()
{
    uint32_t REG_PCA_RISE=0;
    uint32_t REG_PCA_FALL=0;
    uint32_t delta_reg=0;
    uint32_t i=0;

    // ---- trigger ----

    // manual reload value
    TH0 =0xff;
    TL0 =0xf1;

    trigger_signal_b = 1; // turn on one tact before start timer to ensure 10us pulse minimum
    for(;i<=1;i++){
        TCON_TR0 = 1; // start timer 0
        while (TCON_TF0 == 0); // wait for 10usec
        TCON_TF0 = 0; // clear overflow flag
    }
    trigger_signal_b=0;
    TCON_TF0 = 0; // clear overflow flag
    TCON_TR0= 0; // stop


    // ---- ECHO capture ----

    // reset pca0 register value
    //get ready to catch echo signal rise
    PCA0CPM1=PCA0CPM1_CAPP__ENABLED;

    if(PCA0CN0_CF)
        PCA0CN0_CF=0;
    if(PCA0CN0_CCF1)
        PCA0CN0_CCF1=0;
    PCA0CN0_CR = 1; // start PCA0
    while(PCA0CN0_CCF1==0);//waiting echo rise
    REG_PCA_RISE=PCA0CPH1*0x100 + PCA0CPL1;
    PCA0CN0_CCF1=0;//clear the flag
    PCA0CN0_CF=0;
    //get ready to catch echo signal fall
    PCA0CPM1=PCA0CPM1_CAPN__ENABLED;
    while(PCA0CN0_CCF1==0);//waiting echo fall, delay_end is for security

    REG_PCA_FALL = PCA0CPH1*0x100 + PCA0CPL1; // save how many clocks the ECHO signal run
    PCA0CN0_CR = 0; // stop PCA0
    PCA0CN0_CCF1 = 0;
    if(PCA0CN0_CF)
        PCA0CN0_CF=0;

    // PCA0 clock period is 0.5usec then it can count for about (Oxffff)*0.5usec = 32.7msec
    // we know that the max echo time is about 36ms >  32.7msec => the pca clock start again
    if(REG_PCA_FALL<REG_PCA_RISE)
        delta_reg= 0xffff - REG_PCA_RISE + REG_PCA_FALL;
    else
        delta_reg= REG_PCA_FALL- REG_PCA_RISE;

    // PCA0 clock period is 0.5usec
    delta_reg *= 0.5; //echo_time[usec]
    return delta_reg;
}

bool check_distance_change(uint32_t new_val, uint32_t old_val){
  if(new_val!=old_val)
      return true;
  return false;
}

void toggle_buzz() // turn on and of the
{
  buzzer=!buzzer;
}

uint32_t buzzer_ton_state(uint32_t distance_mesauered){
  uint32_t state;
  if(distance_mesauered>=30)
    state=0;
  else if (distance_mesauered<30 && distance_mesauered>=20)
    state=1;
  else if(distance_mesauered<20 && distance_mesauered>=15)
    state=2;
  else if(distance_mesauered<15 && distance_mesauered>=10)
    state=3;
  else if(distance_mesauered<10 && distance_mesauered>=5)
    state=4;
  else if(distance_mesauered<5)
    state=5;
  else // dont care
    state=0;
  return state;
}

bool alaram_state(uint32_t distance_mesauered)
{
  if(distance_mesauered<MINIMAL_DISTANCE)
      return true;
  return false;
}
