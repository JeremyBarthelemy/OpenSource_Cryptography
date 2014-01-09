/*This is the controller,
 *Seems good
*/

#include "io430.h"
#include "io430g2553.h"

#define UART_TXD   0x02                     // TXD on P1.1 (Timer0_A.OUT0)
#define UART_RXD   0x04                     // RXD on P1.2 (Timer0_A.CCI1A)  RX/TX IS FROM TERMINAL SIDE (AKA HOOKUP UART_RXD to ZIGBEE/FTDICABLE RX

//------------------------------------------------------------------------------
// Conditions for 9600 Baud SW UART, SMCLK = 1MHz
//------------------------------------------------------------------------------
#define UART_TBIT_DIV_2     (1000000 / (9600 * 2))
#define UART_TBIT           (1000000 / 9600)

//------------------------------------------------------------------------------
// Global variables used for full-duplex UART communication
//------------------------------------------------------------------------------
unsigned int txData;                        // UART internal variable for TX
unsigned char rxBuffer;                     // Received UART character

unsigned int i = 0;
unsigned int sound_Detected = 0;
unsigned int motion_Left_Detected = 0;
unsigned int motion_Center_Detected = 0;
unsigned int motion_Right_Detected = 0;
unsigned int CurDirection = 1; //Direction of the camera!  0 = Left, 1 = Center, 2 = Right!!!

void IOPoll();
void init_PWM();
void goToLeft();
void goToRight();
void goToCenter();
void pan();

int main( void )
{   
   // Stop watchdog timer to prevent time out reset
   WDTCTL = WDTPW + WDTHOLD;  
   
   init_PWM();
  
   // configure the CPU clock (MCLK) 
   // to run from DCO @ 16MHz and SMCLK = DCO / 4
   BCSCTL1 = CALBC1_1MHZ; // Set DCO
   DCOCTL = CALDCO_1MHZ;
   //BCSCTL2= DIVS_2 + DIVM_0; // divider=4 for SMCLK and 1 for MCLK

   // set the pin mode 3 for pins 1 & 2 of port 1 (Uart mode)
   P1SEL |= BIT1 + BIT2; // low bit = 1 for pin 1 and 2
   P1SEL2 |= BIT1 + BIT2; // high bit = 1 for pin 1 and 2
   
   UCA0CTL1 = UCSWRST;      //RESET UART
  
   // configure the UART
   UCA0CTL0 = 0; //UART mode, No parity, LSB first, 8 data, 1 stop
   UCA0CTL1 |= UCSSEL_2; //use SCLK
   UCA0BR0 = 104; //lower byte of UCBR0. xoxoxo26dec  
                   //(4MHz / 9600 baud)  see table 15-5
   UCA0BR1 = 0x0; //upper byte of UCBR0.set to 0
   UCA0MCTL = UCBRS_0; //sets UCBRFx to 1,
                                   // UCBRSx to 0 , UCOS16=1
   UCA0CTL1 &= ~UCSWRST; // **Initialize USCI **
   UC0IE |= UCA0RXIE; // Enable USCI_A1 RX interrupt
   IE2 |= UCA0RXIE;
   
   P1SEL |= BIT6;       // P1.6 to TA0.1 for PWM timer

   P1DIR |= 0x43;	//Debug LED output p1.0
                        //UART Tx to Command through zigbee is output p1.1
                        //UART Rx from Command through zigbee is input p1.2
                        //Motion sensor left input P1.3
                        //Motion sensor center input p1.4
                        //Motion sensor right input p1.5
                        //PWM Servo output p1.6
                        //Sound sensor input p1.7
                        
//   P1OUT |= 0x01;       //Debug led output p1.0 **ON**

//   P1REN |= 0x38;       //pull up resistor enabled for p1.3/4/5 Motion sensors
   //Abe added start
     P1REN |= 0x38;       //pull up resistor enabled for p1.3/4/5 Motion sensors
     P1OUT &= ~0x38;
   //Abe added end
//   P1OUT |= 0x38;
  
    __bis_SR_register(GIE); // interrupts enabled
     
   while (1)
   {
     
      IOPoll();
  
      if(sound_Detected == 1)                              //SOUND TRIGGER
         UCA0TXBUF = 's';  //transmit an s
      if(motion_Right_Detected == 1)                             //MOTION TRIGGER RIGHT
      {
        UCA0TXBUF = 'o';
        for(i = 0; i < 64000; i++){}
        for(i = 0; i< 64000 ; i++){}
        
      }
      else if(motion_Left_Detected == 1)                             //MOTION TRIGGER LEFT
         UCA0TXBUF = 'm';
      else if(motion_Center_Detected == 1)                             //MOTION TRIGGER CENTER
      {
        UCA0TXBUF = 'n';
        for(i = 0; i < 64000; i++){}
        for(i = 0; i< 64000 ; i++){}
      }
      else{}
      
      for(i=0; i< 20000/16; i++);                //Delay
      
 //moved send large chars to RX UART interrupt see bottom
      
      //Check if we receive S, M, N, O, X, etc...
   }
}

void IOPoll()
{
    //Polling
    //SOUND SENSOR
    if((P1IN & 0x80) == 0x80)
    {
      sound_Detected = 1;
    }
    else
    {
      sound_Detected = 0;
    }
/*    //MOTION SENSOR LEFT
    if((P1IN & 0x08) == 0x08)
    {
        motion_Left_Detected= 1;
    }
    else
    {
        motion_Left_Detected = 0;
    }
    */
    //MOTION SENSOR CENTER
    if((P1IN & 0x20) == 0x20)
    {
      motion_Center_Detected = 1;
    }
    else
    {
        motion_Center_Detected = 0;
    }
    //MOTION SENSOR RIGHT
    if((P1IN & 0x10) == 0x10)
    {
      motion_Right_Detected = 1;
    }
    else
    {
        motion_Right_Detected = 0;
    }
}

void init_PWM()
{
/*  CCR0 = 19276-1; // PWM Period        //compare 0
  // CCR0 = 1000-1; // PWM Period
  CCTL1 = OUTMOD_7; // CCR1 reset/set  // compare control 1
  CCR1 = 1900; // CCR1 PWM duty cycle //compare 1
  TACTL = TASSEL_2 + MC_1; // SMCLK, up mode
*/
  //Abe added 12/5/12 START
  P1SEL |= ~BIT6;       // P1.6 to TA0.1 for PWM timer
  //Abe added 12/5/12 END
  CCR0 = 20000-1; // PWM Period        //compare 0
  // CCR0 = 1000-1; // PWM Period
  CCTL1 = OUTMOD_7; // CCR1 reset/set  // compare control 1
  CCR1 = 1500-1; // CCR1 PWM duty cycle //compare 1
  TACTL = TASSEL_2 + MC_0; // SMCLK, up mode
}

void goToLeft()
{
  
  //Abe added 12/5/12 START
  TACTL = TASSEL_2 + MC_1;    //SET PIN TO TIMER FUNC -> ON
  //Abe added 12/5/12 END
  CCR1 = 800;
  //CCR1 = 1000; // CCR1 PWM duty cycle //compare 1
  CurDirection = 0;
  //Abe added 12/5/12 START                             
  for(i=0; i< 32000; i++);                //Delay
  TACTL = TASSEL_2 + MC_0;       // P1.6 to TA0.1 for PWM timer  --SET PIN TO TIMER FUNC -> OFF
  //Abe added 12/5/12 END

}

void goToRight()
{
  
  //Abe added 12/5/12 START
  TACTL = TASSEL_2 + MC_1;       //SET PIN TO TIMER FUNC -> ON
  //Abe added 12/5/12 END
  CCR1 = 2000; // CCR1 PWM duty cycle //compare 1
  CurDirection = 2;
  //Abe added 12/5/12 START                             
  for(i=0; i< 32000; i++);                //Delay
  TACTL = TASSEL_2 + MC_0; //SET PIN TO TIMER FUNC -> OFF
  //Abe added 12/5/12 END
}

void goToCenter()
{
  
    //Abe added 12/5/12 START
  TACTL = TASSEL_2 + MC_1;       //SET PIN TO TIMER FUNC -> ON
  //Abe added 12/5/12 END
  //CCR0 = 19276-1; // PWM Period        //compare 0
  CCR1 = 1400; // CCR1 PWM duty cycle //compare 1
  CurDirection = 1;
    //Abe added 12/5/12 START                             
  for(i=0; i< 32000; i++);                //Delay

  TACTL = TASSEL_2 + MC_0; //SET PIN TO TIMER FUNC -> OFF
  //Abe added 12/5/12 END
}

void pan()
{
  //will need to change the values - perhaps set a delay and slowly increment or decrement upon reaching leftmost/rightmost positions
 // CCR0 = 19276-1; // PWM Period        //compare 0
 // CCR1 = 1900; // CCR1 PWM duty cycle //compare 1
}


#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
  while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
  {
      if(UCA0RXBUF == 'S')
      {
         pan();
      }
      else{}
      if(UCA0RXBUF == 'M')
      {
        goToLeft();
      }
      else{}
      if(UCA0RXBUF == 'N')
      {
        goToCenter();
      }
      else{}
      if(UCA0RXBUF == 'O')
      {
        goToRight();
      }
      else{}
      if(UCA0RXBUF == 'L')
      {
        if(CurDirection == 0)
        {
          //Do Nothing
        }
        else if(CurDirection == 1)
        {
          goToLeft();
        }
        else
        {
          goToCenter();
        }
      }
      if(UCA0RXBUF == 'R')
      {
        if(CurDirection == 0)
        {
          goToCenter();
        }
        else if(CurDirection == 1)
        {
          goToRight();
        }
        else
        {
          //Do Nothing
        }
      }
  }
  /*  if(UCA0RXBUF == 'z')
    UCA0TXBUF = 'Q';
  else if(UCA0RXBUF == 'g')
    P1OUT ^= 0x40;
  else
    UCA0TXBUF = 'E';
  */
}


/*
// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
  P1OUT ^= 0x01;                            // P1.0 = toggle
  UCA0TXBUF = 'g';
  P1IFG &= ~0x08;                           // P1.4 IFG cleared
}
*/