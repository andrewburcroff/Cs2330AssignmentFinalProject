#include <msp430.h>
#include <libemb/serial/serial.h>
#include <libemb/conio/conio.h>
#include "colors.h"

void checkLED(int pass);

int ones = 0x0;                            //Intailize ones in hex
int teens = 0x0;                           //Intailize teens in hex

/**********************Intailize Color for Print*********************/

unsigned int color = 1;                    //Intailize color to 1.
int RED = 0;                               //Intailize RED.
int GREEN = 0;                             //Intailize Green.
int BLUE = 0;                              //Intailize Blue.
int value =1;                              //Intailize value to 1.

/************************BIT Intailize********************************/

int A = BIT0;                             //P1.0
int B = BIT5;                             //P1.5
int c = BIT0;                             //P2.0
int D = BIT2;                             //P2.2
int E = BIT3;                             //P2.3
int F = BIT4;                             //P2.4
int G = BIT7;                             //P1.7
int XIN = BIT6;                           //P1.6
int XOUT = BIT7;                          //P1.7

/**************************Main Method******************************/

int main(void)
{
    WDTCTL  = WDTPW | WDTHOLD;              // wizard words
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL  = CALDCO_1MHZ;

/******************************P1 Materials*************************/

    P1DIR = A|B|BIT6|BIT7;
    P1REN |= BIT3;
    P1SEL = BIT6 |BIT4;
    P1OUT = BIT3;
    P1IE  |= BIT3;
    P1IES |= BIT3;

/****************************P2 Materials***************************/

    P2DIR = -1;
    P2OUT = BIT6 | BIT7;
    P2SEL = BIT5 | BIT1;

/***************************Toggle button**************************/

    TA0CTL   = TASSEL_2 | MC_1 | ID_3;      // use TA0.1 for PWM on P1.6
    TA0CCR0  = 0x3FF;                       // 10-bit maximum value
    TA0CCR1  = 0;                           // start off
    TA0CCTL1 = OUTMOD_7;                    // reset/set output mode

    TA1CTL   = TASSEL_2 | MC_1 | ID_3;      // use TA1.1 for PWM on P1.6
    TA1CCR0  = 0x3FF;                       // 10-bit maximum value
    TA1CCR1  = 0;                           // start off
    TA1CCTL1 = OUTMOD_7;                    // reset/set output mode

    TA1CCR2  = 0;                           // 10-bit maximum value
    TA1CCTL2 = OUTMOD_7;                    // reset/set output mode

    ADC10CTL1 = INCH_4 | ADC10DIV_3;        // ADC10 channel 4, clock divider 3
    ADC10CTL0 = SREF_0 | ADC10SHT_3 |       // VCC/VSS ref, 64 x ADC10CLKs
               ADC10ON | ADC10IE;           // ADC10 enable, ADC10 interrupt enable
    ADC10AE0  = BIT4;                       // analog enable channel 4

    serial_init(9600);
     __enable_interrupt();                  // interrupts enabled

    TA0CCTL0 = CCIE;
    while(1)
    {
        __delay_cycles(10000);                  //Wait for ADC ref to settle
        ADC10CTL0 |= ENC + ADC10SC;             //Sampling and conversion start
        if(color == 1){
            TA0CCR1 = ADC10MEM & 0x3F8;         //Assigns the value held in ADC10MEM to the TA1CCR1 register
            RED = (ADC10MEM & 0x3F8)/128;       //Getting the value of input of memory / 128.
        }else if(color == 2){
            TA1CCR1 = ADC10MEM & 0x3F8;         //Assigns the value held in ADC10MEM to the TA1CCR1 register
            GREEN = (ADC10MEM & 0x3F8)/128;     //Getting the value of input of memory / 128.
        }else if(color == 3){
            TA1CCR2 = ADC10MEM & 0x3F8;         //Assigns the value held in ADC10MEM to the TA1CCR2 register
            BLUE = (ADC10MEM & 0x3F8)/128;      //Getting the value of input of memory / 128.
        }
        int name = (RED + (GREEN * 8) + (BLUE * 64));
        cio_printf("\r RED %i, GREEN %i, BLUE %i, color: %s        ", RED, GREEN, BLUE, colors[name]);
    }
    return 0;
}

/*****************************change the color***********************/

#pragma vector=PORT1_VECTOR
__interrupt void portinterrupt (void){
  P1IFG &= ~BIT3;
    if(color == 1){
      color = 2;                            //Setting the color to equal 2.
    }else if(color == 2){
      color = 3;                            //Setting the color to equal 3.
    }else if(color == 3){
      color = 1;                            //Setting the color to equal 1.
    }
}

/***********************7 Segement Display**************************/

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0 (void){
  int number = ((ADC10MEM & 0x3F8)/8);      //Setting the number of input /8.
  ones = number % 16;                       //Setting the ones to number of 16 of hex.
  teens = ((number / 16)%16);               //Setting the remainder of the number.

  if (value == 1){
    P2OUT = XIN;                             //Turning on the XOUT of p2.
    P1OUT = BIT3;                            //Turning on the BIT3 of p1.
    checkLED(teens);                         //CheckLED of the teens.
    value =2;                                //Setting the value to 2.
  }
  else if(value ==2){
    P2OUT = XOUT;                            //Turning on XOUT of p2.
    P1OUT = BIT3;                            //Turning on BIT3 of P1.
    checkLED(ones);                          //CheckLED of the ones.
    value = 1;                               //Setting the value to 1.
  }
}
  void checkLED(int pass){
    if(pass == 0x0){
        P1OUT |= G;                         //Turn off the G part.
    }
    else if(pass == 0x1){
      P2OUT |= D|E|F;                       //Turn off the D, E and F part.
      P1OUT |= A|G;                         //Turn off the A and G part.

    }
    else if(pass == 0x2){
        P2OUT |= F|c;                       //Turn off the F and c part.
    }
    else if(pass == 0x3){
      P2OUT |= F|E;                         //Turn off the F and E part.
    }
    else if(pass == 0x4){
      P2OUT |= E|D;                         //Turn off the E and D part.
    }
    else if(pass == 0x5){
      P1OUT |= B;                           //Turn off the B part.
      P2OUT |= E;                           //Turn off the E part.
    }
    else if(pass == 0x6){
      P1OUT |= B;                           //Turn off the B part.
    }
    else if(pass == 0x7){
      P1OUT |=G;                            //Turn off the G part.
      P2OUT |= F|E|D;                       //Turn off the F, E, D part.
    }
    else if(pass == 0x8){
                                            //Nothing to do
    }
    else if(pass == 0x9){
      P2OUT |= E;                           //Turn off the E Part.
    }
    else if(pass == 0xA){
        P2OUT |=D;                          //Turn off the D part.
    }
    else if(pass == 0xB){
      P1OUT |= A|B;                         //Turn off the A and B part.
    }
    else if(pass == 0xC){
      P1OUT |= B|G;                         //Turn off the B and G part.
      P2OUT |= c;                           //Turn off the c part.
    }
    else if(pass == 0xD){
      P1OUT |= A;                           //Turn off the A part.
      P2OUT |= F;                           //Turn off the F part.
    }
    else if(pass == 0xE){
      P1OUT |= B;                           //Turn off the B part.
      P2OUT |= C;                           //Turn off the C part.
    }
    else if(pass == 0xF){
      P2OUT |= C|D;                         //Turn off the C and D part.
      P1OUT |= B;
    }
}
