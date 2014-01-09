/*Here is the command, which will have manual button inputs of
 *L, R, X, A (left, right, stop panning, and autopan).
 *If we receive an m, n, or an o, we give M, N, and O output, respectively.  For s (sound received), we simply
 *return an A to autopan.
 *To Do: Read in inputs to UCA0RXBUF, output with UCA0TXBUF, verify, 
 *add a simple ISR that will change the appropriate integer to 1 so we know it was pressed.
 */

//#include "io430.h"
//#include "io430g2553.h"
#include "msp430g2553.h"
   
// TIMER UART SEL Definitions//
#define UART_TXD   0x20                     // TXD on P1.5 (Timer1_A.OUT0) Computer RX (MSP Tx) J3.1 (see laundpad datasheet (BRXD))
#define UART_RXD   0x40                     // RXD on P1.6 (Timer1_A.CCI1A) Computer TX (MSP Rx)

#define ON    0x01
#define OFF   0x00

//--------------------------------------------------------------------------//
//---START----TIMER UART CODE PULLED FROM testTimerUART.C written by Abe----//
//--------------------------------------------------------------------------//
// Conditions for 9600 Baud SW UART, SMCLK = 1MHz
#define UART_TBIT_DIV_2     (16000000 / (9600 * 2))/4 // HAD TO CHANGE TO 16mhz, from 1mhz
#define UART_TBIT           (16000000 / 9600)/4       // ALSO HAD TO DIVIDE BY 4 for
                                                      // DIFF CLOCK CONTROL REGISTER DEFINED
                                                      // IN DAVES UART.
                                                      // -I.DARAB
// Global variables used for full-duplex UART communication
unsigned int txData;                        // UART internal variable for TX
unsigned char rxBuffer;                     // Received UART character
// Function prototypes
void TimerA_UART_init(void);
void TimerA_UART_tx(unsigned char byte);
void TimerA_UART_print(char *string);
//-------------------------------------------------------------------------//
//---END------TIMER UART CODE PULLED FROM testTimerUART.C written by Abe---//
//-------------------------------------------------------------------------//

unsigned int i = 0;
int L_PRESSED = 0;
int R_PRESSED = 0;
int A_PRESSED = 0;
int X_PRESSED = 0;
int alarm = 0;
int auto_mode = ON;

void send_M();  //turn to leftmost position (m1)
void send_N(); //turn to center position (m2)
void send_O(); //turn to rightmost (m3)
void send_A(); //autopan
void send_X(); //stop autopan
void send_L(); //step camera to left
void send_R(); //step camera to right

int main( void )
{   
   WDTCTL = WDTPW + WDTHOLD;  //Pet the dog
   
   // configure the CPU clock (MCLK) 
   // to run from DCO @ 16MHz and SMCLK = DCO / 4
   BCSCTL1 = CALBC1_16MHZ; // Set DCO
   DCOCTL = CALDCO_16MHZ;
   BCSCTL2= DIVS_2 + DIVM_0; // divider=4 for SMCLK and 1 for MCLK
   
   P1OUT |= 0x9F;
    //sets timer uarts to 0, 1's for everything else

   // set the pin mode 3 for pins 1 & 2 of port 1 (Uart mode)
   P1SEL |= BIT1 + BIT2; // low bit = 1 for pin 1 and 2
   P1SEL2 |= BIT1 + BIT2; // high bit = 1 for pin 1 and 2
   
  //-----------------------------------------------//
  //------START--------------TIMER UART CODE ------//
  //-----------------------------------------------// 
   //set P1SEL for Timer UART
   P1SEL |= UART_TXD + UART_RXD; // TXD on P1.5 (Timer1_A.OUT0)
                                // RXD on P1.6 (Timer1_A.CCI1A)
   // Old test code
      TimerA_UART_init();                     // Start Timer_A UART
  //-----------------------------------------------//
  //------END--------------TIMER UART CODE ------//
  //-----------------------------------------------// 
     
   
   UCA0CTL1 = UCSWRST;      //RESET UART
  
   // configure the UART
   UCA0CTL0 = 0; //UART mode, No parity, LSB first, 8 data, 1 stop
   UCA0CTL1 |= UCSSEL_2; //use SCLK
   UCA0BR0 = 0x1A; //lower byte of UCBR0. 26dec  
                   //(4MHz / 9600 baud)  see table 15-5
   UCA0BR1 = 0x0; //upper byte of UCBR0.set to 0
   UCA0MCTL = UCBRF_1 + UCBRS_0 + UCOS16; //sets UCBRFx to 1,
                                   // UCBRSx to 0 , UCOS16=1
   UCA0CTL1 &= ~UCSWRST; // **Initialize USCI **
   UC0IE |= UCA0RXIE; // Enable USCI_A1 RX interrupt
   IE2 |= UCA0RXIE;

   
   P1DIR |= 0x45; // P1.0 -1- OUTPUT  LED_ALARM
                  // P1.1 -0- INPUT   UART_(MSP430)Rx
                  // P1.2 -1- OUTPUT  UART_(MSP430)Tx
                  // P1.3 -0- INPUT   BUTTON_LEFT_MOVE
                  // P1.4 -0- INPUT   BUTTON_RIGHT_MOVE
                  // P1.5 -0- INPUT   Timer UART_(MSP430)Tx
                  // P1.6 -1- OUTPUT  Timer UART_(MSP430)Rx
                  // P1.7 -0- INPUT   BUTTON_MODE(AUTO/MANUAL)
   
   P1REN |= 0x98;  // ALL BUTTONS PULLED HIGH P1.3/4/7
                  
   P1IE  |= 0x98;  // P1.3/4/7 interrupt enabled
   //P1IES |= 0x98;  // P1.3/4/7 Hi/lo edge
   P1IFG &= ~0x98; // P1.3/4/7 IFG cleared
      

   asm("EINT");

   //UART print test
   TimerA_UART_print("G2xx2 TimerA UART\r\n");
   TimerA_UART_print("READY.\r\n"); 
   
   while (1)
   {
     
     //Timer uart code
        // Set LED RED on if rcvd char = 'a', else turn it off.
 //--//       if (rxBuffer == 'a') P1OUT |= 0x01; else P1OUT &= ~0x01;    // P1.0
        // Echo received character
//--//        TimerA_UART_tx(rxBuffer);
      //End Timer uart code
     
     if(auto_mode == OFF)
     {
       if(L_PRESSED == 1)
       {
         send_L();
         L_PRESSED = 0;
         alarm = OFF;
       }
       if(R_PRESSED == 1)
       {
         send_R();
         R_PRESSED = 0;
         alarm = OFF;
       }
     }
     if(A_PRESSED == 1)
     {
       send_A();
       A_PRESSED = 0;
       alarm = OFF;
     }
     if(X_PRESSED == 1)
     {
       send_X();
       X_PRESSED = 0;
       alarm = OFF;
     }
     
     if(alarm == ON)
       P1OUT |= 0x01;
     else
       P1OUT &= ~0x01;
/********************************************/
    

     
      //DELAYS
      for(i=0; i< 65530; i++);               
      for(i=0; i< 65530; i++);               
      for(i=0; i< 65530; i++);               
      for(i=0; i< 65530; i++);               
   }
   
}

void send_M(void)
{
  UCA0TXBUF = 'M';
}
void send_N(void)
{
  UCA0TXBUF = 'N';
}
void send_O(void)
{
  UCA0TXBUF = 'O';
}
void send_X(void)
{
  UCA0TXBUF = 'X';
}
void send_L(void)
{
  UCA0TXBUF = 'L';
}
void send_R(void)
{
  UCA0TXBUF = 'R';
}
void send_A(void)
{
  UCA0TXBUF = 'A';
}
#pragma vector = PORT1_VECTOR
__interrupt void Button_Press_ISR(void)
{
    //Bit test, to determine which button was pressed
    if(((~P1IN) & BIT7) == BIT7 && auto_mode == OFF)  
    {
      L_PRESSED = 1;
    }
    if(((~P1IN) & BIT4) == BIT4 && auto_mode == OFF)
    {
      R_PRESSED = 1;
    }
    if(((~P1IN) & BIT3) == BIT3)
    {
        if(auto_mode == OFF)
        {
          A_PRESSED = 1;
          auto_mode = ON;
          P1OUT |= BIT6;
        }
        else
        {
          X_PRESSED = 1;
          auto_mode = OFF;
          P1OUT &= ~BIT6;
        }
    }
    
    alarm = OFF;
    
  //  P1OUT ^= 0x01; // P1.0 = toggle
    
    P1IFG &= ~BIT3; // P1.3/4/7 IFG cleared
    P1IFG &= ~BIT4; // P1.3/4/7 IFG cleared
    P1IFG &= ~BIT7; // P1.3/4/7 IFG cleared
  
}


#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
  while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
/*  if(UCA0RXBUF == 'z')
    UCA0TXBUF = 'Q';
  else if(UCA0RXBUF == 'g')
    P1OUT ^= 0x40;
//  else
  //  UCA0TXBUF = 'E';*/
  
       if(UCA0RXBUF == 's')  //autopan if a noise is detected
     {
       //rxBuffer = '!';
       send_A();
        
       alarm = ON;
     }
     else if(UCA0RXBUF == 'm') //if motion detected on m1, go to left
     {
       //rxBuffer = '!';
       if(auto_mode == ON)
       {
          send_M();
       }
     
       alarm = ON;
     }
     else if(UCA0RXBUF == 'n') //if motion is detected on m2, go to center
     {
       //rxBuffer = '!';
       if(auto_mode == ON)
       {
          send_N();
       }
       alarm = ON;
     }
     else if(UCA0RXBUF == 'o') //if motion is detected on m3, go to right
     {
       //rxBuffer = '!';
       if(auto_mode == ON)
       {
          send_O();
       }
       alarm = ON;
     }

}

//-----------------------------------------------//
//------START--------------TIMER UART CODE ------//
//-----------------------------------------------// 

//------------------------------------------------------------------------------
// Function configures Timer_A for full-duplex UART operation
//------------------------------------------------------------------------------
void TimerA_UART_init(void)
{
    TACCTL0 = OUT;                          // Set TXD Idle as Mark = '1'
    TACCTL1 = SCS + CM1 + CAP + CCIE;       // Sync, Neg Edge, Capture, Int
    TACTL = TASSEL_2 + MC_2;                // SMCLK, start in continuous mode
}
//------------------------------------------------------------------------------
// Outputs one byte using the Timer_A UART
//------------------------------------------------------------------------------
void TimerA_UART_tx(unsigned char byte)
{
    while (TACCTL0 & CCIE);                 // Ensure last char got TX'd
    TACCR0 = TAR;                           // Current state of TA counter
    TACCR0 += UART_TBIT;                    // One bit time till first bit
    TACCTL0 = OUTMOD0 + CCIE;               // Set TXD on EQU0, Int
    txData = byte;                          // Load global variable
    txData |= 0x100;                        // Add mark stop bit to TXData
    txData <<= 1;                           // Add space start bit
}

//------------------------------------------------------------------------------
// Prints a string over using the Timer_A UART
//------------------------------------------------------------------------------
void TimerA_UART_print(char *string)
{
    while (*string) {
        TimerA_UART_tx(*string++);
    }
}
//------------------------------------------------------------------------------
// Timer_A UART - Transmit Interrupt Handler
//------------------------------------------------------------------------------
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void)
{
    static unsigned char txBitCnt = 10;

    TACCR0 += UART_TBIT;                    // Add Offset to CCRx
    if (txBitCnt == 0) {                    // All bits TXed?
        TACCTL0 &= ~CCIE;                   // All bits TXed, disable interrupt
        txBitCnt = 10;                      // Re-load bit counter
    }
    else {
        if (txData & 0x01) {
          TACCTL0 &= ~OUTMOD2;              // TX Mark '1'
        }
        else {
          TACCTL0 |= OUTMOD2;               // TX Space '0'
        }
        txData >>= 1;
        txBitCnt--;
    }
}      
//------------------------------------------------------------------------------
// Timer_A UART - Receive Interrupt Handler
//------------------------------------------------------------------------------
#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer_A1_ISR(void)
{
    static unsigned char rxBitCnt = 8;
    static unsigned char rxData = 0;

    switch (__even_in_range(TA0IV, TA0IV_TAIFG)) { // Use calculated branching
        case TA0IV_TACCR1:                        // TACCR1 CCIFG - UART RX
            TACCR1 += UART_TBIT;                 // Add Offset to CCRx
            if (TACCTL1 & CAP) {                 // Capture mode = start bit edge
                TACCTL1 &= ~CAP;                 // Switch capture to compare mode
                TACCR1 += UART_TBIT_DIV_2;       // Point CCRx to middle of D0
            }
            else {
                rxData >>= 1;
                if (TACCTL1 & SCCI) {            // Get bit waiting in receive latch
                    rxData |= 0x80;
                }
                rxBitCnt--;
                if (rxBitCnt == 0) {             // All bits RXed?
                    rxBuffer = rxData;           // Store in global variable
                    rxBitCnt = 8;                // Re-load bit counter
                    TACCTL1 |= CAP;              // Switch compare to capture mode
                   // __bic_SR_register_on_exit(LPM0_bits);  // Clear LPM0 bits from 0(SR)
                }
            }
            break;
    }
}  

  //-----------------------------------------------//
  //------END--------------TIMER UART CODE ------//
  //-----------------------------------------------// 