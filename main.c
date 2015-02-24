#include  "msp430.h"

#define     LED1                  BIT0
#define     LED2                  BIT6

#define     BUTTON                BIT3
#define     TRIGGER               BIT4
#define     ECHO                  BIT5
#define     ECHOS                 BIT7

#define     TXD                   BIT2
#define     RXD                   BIT1

unsigned int TXByte;
int flag;

unsigned int mult;
unsigned int start_time;
unsigned int end_time;
float result;

void main(void) {
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

	/* next three lines to use internal calibrated 1MHz clock: */
	BCSCTL1 = CALBC1_8MHZ;		// use 8MHz clock
	DCOCTL = CALDCO_8MHZ;
	BCSCTL2 &= ~(DIVS_3);		// no divider for smclk
	BCSCTL2 |= DIVM_3;			// divide by 8 for mclk, operate at 1MHz

	// setup port for leds:
	P1DIR |= LED1 + LED2 + TRIGGER;
	P1OUT &= ~(LED1 + LED2 + TRIGGER + ECHOS);
	P1OUT |= ECHO;

	P1DIR &= ~(ECHO + ECHOS);
	P1REN = ECHO + ECHOS;
	P1IES = ECHO;
	P1IE = ECHO + ECHOS;
	P1DIR |= TXD;
	P1OUT |= TXD;

	__enable_interrupt();                     // Enable interrupts.

	/* Configure hardware UART */
	P1SEL = BIT1 + BIT2; // P1.1 = RXD, P1.2=TXD
	P1SEL2 = BIT1 + BIT2; // P1.1 = RXD, P1.2=TXD
	UCA0CTL1 |= UCSSEL_2; // Use SMCLK
	UCA0BR0 = 98; // Set baud rate to 9600 with 1MHz clock (Data Sheet 15.3.13)
	UCA0BR1 = 3; // Set baud rate to 9600 with 1MHz clock
	UCA0MCTL = UCBRS0; // Modulation UCBRSx = 1
	UCA0CTL1 &= ~UCSWRST; // Initialize USCI state machine

	/* Main Application Loop */
	while (1) {
		// Send trigger pulse
		P1OUT |= TRIGGER;
		__delay_cycles(10);
		P1OUT &= ~TRIGGER;

		__bis_SR_register(LPM0_bits + GIE);

		result = (float)((float)(end_time - start_time + 65535 * (mult - 1)) / 470);
		char* char_p = (char*)&result;
		int i;
		for (i = 0; i < 4; i++) {
			TXByte = *char_p++;
			while (!(IFG2 & UCA0TXIFG)); // wait for TX buffer to be ready for new data
			UCA0TXBUF = TXByte;
		}

		P1OUT ^= LED1;  // toggle the light every time we make a measurement.

		// set up timer to wake us in a while:
		TACCR0 = 1000;                             //  period
		TACTL = TACLR | TASSEL_1 | MC_1;                  // TACLK = ACLK, Up mode.
		TACCR1 = 1000; // interrupt at end
		TACCTL1 = CCIE;                // TACCTL0

		// go to sleep, wait till timer expires to do another measurement.
		__bis_SR_register(LPM3_bits + GIE); // LPM0 with interrupts enabled  turns cpu off.
	}
}

// used as loop delay
#pragma vector=TIMER0_A1_VECTOR
__interrupt void ta1_isr(void) {
	switch (TAIV) {
	case 2:
		TACCTL1 &= ~CCIFG;                         // reset the interrupt flag
		TACCTL1 = 0;                               // no more interrupts.
		__bic_SR_register_on_exit(LPM3_bits);      // Restart the cpu
		break;
	case 10:
		mult++;				// timer overflowed, increment mult
		TACTL &= ~TAIFG;
		break;
	}
}

#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR(void) {
	if (P1IFG & ECHO) {
		if (!P1IES) {
			TACTL = TACLR | TASSEL_2 | MC_2 | TAIE; // TACLK = SMCLK, Continuous mode, enable interrupts
			start_time = TAR;
			mult = 0;
			P1IES |= ECHO;
			P1IFG &= ~ECHO;
			TACCTL1 = 0; // disable timer capture/compare interrupts
		} else {
			end_time = TAR;
			TACTL = 0;
			P1IES &= ~ECHO;
			P1IFG &= ~ECHO;

			__bic_SR_register_on_exit(LPM0_bits);
		}
	}
}
