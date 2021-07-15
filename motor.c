#include <msp430g2253.h>
#include <stdio.h>


    unsigned char Rx_Data = 0;                  // Byte received via UART

/**
 * main.c
 */
void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;               // stop watchdog timer
    DCOCTL = 0;                             // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ;                  // Set DCO
    DCOCTL = CALDCO_1MHZ;

    /*** Set-up GPIO ***/
    P1SEL = BIT1 + BIT2;                    // P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2;                   // P1.1 = RXD, P1.2=TXD

    /**Port Configuration for the motors**/

    //check for an change in transmitted data
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;

     //initial 'stop'
     //Right Motor Control pins
     P2DIR |=  BIT0 + BIT1 + BIT2;          // P2.0,P2.1,P2.2 all output to the drive motor A
     P2OUT &= ~BIT0 + BIT1 + BIT2;          // Clear P2.0,P2.1,P2.2 2.0 ENA 2.1 A1 2.2 A2

     P2SEL  &= ~BIT6;
     P2SEL2 &= ~BIT6;

     //Left Motor Control pins
     P2DIR |=  BIT3 + BIT4 + BIT6;          // P2.3,P2.4,P2.5    all output to the driver motor B
     P2OUT &=  ~BIT3 + BIT4 + BIT6;          // Clear P2.3,P2.4,P2.5  2.5 ENB 2.4 B1 2.3 B2

     /*** Set-up USCI A ***/
     UCA0CTL1 |= UCSSEL_2;                   // SMCLK
     UCA0BR0 = 104;                          // 1MHz 9600
     UCA0BR1 = 0;                            // 1MHz 9600
     UCA0MCTL = UCBRS0;                      // Modulation UCBRSx = 1
     UCA0CTL1 &= ~UCSWRST;                   // Initialize USCI state machine
     IE2 |= UCA0RXIE;                        // Enable USCI_A0 RX interrupt
     __bis_SR_register(LPM0_bits + GIE);     // Enter LPM0, interrupts enabled


//     // Timer 0 Configurtion
//        TACTL |= MC_0;                   // Stop Timer0_A3
//        TACTL  = TASSEL_2 + TAIE;        // TAR Clock = 1MHz,1MHz/8 = 125KHz,Enable Timer Interrupt
//        CCTL1  = CCIE;                   // Enable interrupt for CCR1
//        CCR1   = 6000;                // Load value
//        TACTL |= MC_2;                   // Start Timer0 in Continous Mode

     while(1){
         P1OUT &= ~BIT0;
         switch(Rx_Data)
         {

         case 0x48: //Stop H
             P2OUT &= ~BIT0 + ~BIT6;
             break;

         case 0x46: //Forward F
             //right motor
             P2OUT |=  BIT1;
             P2OUT &= ~BIT2;
             P2OUT |=  BIT0;

             //left motor
             P2OUT |=  BIT3;
             P2OUT &= ~BIT4;
             P2OUT |=  BIT6;
             break;

         case 0x42: //backward B

             //right motor
             P2OUT &= ~BIT1;
             P2OUT |= BIT2;
             P2OUT |=  BIT0;

             //left motor
             P2OUT &= ~BIT3;
             P2OUT |= BIT4;
             P2OUT |= BIT6;
             break;


         case 0x4C: //Right R
             //stop right motor
             P2OUT &= ~BIT1 + ~BIT0 + ~BIT2;
             //left motor forward
             P2OUT |=  BIT3;
             P2OUT &= ~BIT4;
             P2OUT |=  BIT6;
             break;


         case 0x52: //Left L
             //stop left motor
             P2OUT &= ~BIT4 + ~BIT3 + ~BIT6;

             //right motor forward
             P2OUT |=  BIT1;
             P2OUT &= ~BIT2;
             P2OUT |=  BIT0;
             break;

//         case 0x44: //Speed one D
//             TACTL |= MC_0;                   // Stop Timer0_A3
//                    TACTL  = TASSEL_2 + TAIE;        // TAR Clock = 1MHz,1MHz/8 = 125KHz,Enable Timer Interrupt
//                    CCTL1  = CCIE;                   // Enable interrupt for CCR1
//                    CCR1   = 6000;                // Load value
//                    TACTL |= MC_2;
//                      break;
//
//         case 0x4D: //speed two M
//             TACTL |= MC_0;                   // Stop Timer0_A3
//                    TACTL  = TASSEL_2 + TAIE;        // TAR Clock = 1MHz,1MHz/8 = 125KHz,Enable Timer Interrupt
//                    CCTL1  = CCIE;                   // Enable interrupt for CCR1
//                    CCR1   = 12000;                // Load value
//                    TACTL |= MC_2;
//                      break;
//
//         case 0x51: //speed 3 Q
//             TACTL |= MC_0;                   // Stop Timer0_A3
//                    TACTL  = TASSEL_2 + TAIE;        // TAR Clock = 1MHz,1MHz/8 = 125KHz,Enable Timer Interrupt
//                    CCTL1  = CCIE;                   // Enable interrupt for CCR1
//                    CCR1   = 37000;                // Load value
//                    TACTL |= MC_2;
//                      break;

         default:
             P2OUT = BIT0 + BIT6;
             P2OUT |= ~BIT1 + BIT2;
             P2OUT |= BIT3 + ~BIT4;
             break;

         }
     }
}


//  USCI A interrupt handler
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0RX_VECTOR))) USCI0RX_ISR (void)
#else
#error Compiler not supported!
#endif
{
    Rx_Data = UCA0RXBUF;                    // Assign received byte to Rx_Data
    P1OUT = BIT0;
    __bic_SR_register_on_exit(LPM0_bits);   // Wake-up CPU
}

//#pragma vector = TIMER0_A1_VECTOR
//__interrupt void TimerA(void)
//{
//   switch(TAIV)
//   {
//       case 0x002: // CCR1 interrupt
//                   P2OUT |=  BIT0;    // P2.0 = 1 ,1&2_EN = 1,Right Motor is started
//                   P2OUT |=  BIT6;    // P2.6 = 1 ,3&4_EN = 1,Left  Motor is started
//                   break;
//
//       case 0x004:  //CCR2 not used
//                    break;
//
//       case 0x00A: //TAR overflow interrupt
//                    P2OUT &= ~BIT0;     // P2.0 = 0 ,1&2_EN = 0,Right Motor is stoped
//                    P2OUT &= ~BIT6;     // P2.6 = 0 ,3&4_EN = 0,Left  Motor is stoped
//                    break;
//    }
// }
