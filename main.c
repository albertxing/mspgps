#include "main.h"

int main (void) {
	setup();

	// Main application loop
	while (1) {
		TACTL = TACLR | TASSEL_2 | MC_2;
		TACCTL1 = 0;

		for (n = 0; n < 3; n++) {
			// Send trigger pulse
			P2OUT &= ~TRIG1;
			P2IFG = 0;
			P2IE |= ECHO;
			start[n] = TAR;

			// Wait for echo
			__bis_SR_register(LPM0_bits + GIE);

			while (TAR % 2000 > 20);
		}

		TACTL = 0;

		da = (float)(end[0] - start[0]) / 30;
		db = (float)(end[1] - start[1]) / 30;
		dc = (float)(end[2] - start[2]) / 30;

//		float x = (da * da - db * db + 225) / 30.0f - 7.5f;
//		float y = da * da - (x + 7.5f) * (x + 7.5f);
//
//		transmit((char*)&x);
//		transmit((char*)&y);

		// Toggle LED
		P1OUT ^= LED1;

		// Delay between measurements
		TACCR0 = 2000;						// Period
		TACCR1 = 2000;						// Capture/compare at end
		TACTL = TACLR | TASSEL_1 | MC_1;	// TACLK = ACLK, Up mode.
		TACCTL1 = CCIE;						// TACCTL0
		__bis_SR_register(LPM3_bits + GIE);
	}
}

void setup () {
	WDTCTL = WDTPW + WDTHOLD;	// Stop WDT

	// Calibrate clocks
	BCSCTL1 = CALBC1_1MHZ;		// use 8MHz clock
	DCOCTL = CALDCO_1MHZ;
	BCSCTL2 &= ~(DIVS_3);		// no divider for smclk
	BCSCTL2 &= ~(DIVM_3);		// no divider for mclk

	// Set up LEDs
	P1DIR |= LED1;
	P1OUT &= ~LED1;

	P2DIR |= TRIG1 + TRIG2 + TRIG3;
	P2DIR &= ~ECHO;
	P2REN |= ECHO;
	P2OUT |= TRIG1 + TRIG2 + TRIG3;
	P2OUT &= ~ECHO;
	P2IES = 0;

	P1DIR |= TXD;
	P1OUT |= TXD;

	__enable_interrupt();		// Enable interrupts.

	// Configure UART
	P1SEL = BIT1 + BIT2;		// P1.1 = RXD, P1.2=TXD
	P1SEL2 = BIT1 + BIT2;		// P1.1 = RXD, P1.2=TXD
	UCA0CTL1 |= UCSSEL_2;		// Use SMCLK
	UCA0BR0 = 98;				// Set baud rate to 9600 with 1MHz clock (Data Sheet 15.3.13)
	UCA0BR1 = 3;				// Set baud rate to 9600 with 1MHz clock
	UCA0MCTL = UCBRS0;			// Modulation UCBRSx = 1
	UCA0CTL1 &= ~UCSWRST;		// Initialize USCI state machine
}

void transmit (char* result) {
	unsigned int TXByte;
	int i;
	for (i = 0; i < 4; i++) {
		TXByte = *result++;
		// Wait for TX buffer
		while (!(IFG2 & UCA0TXIFG));
		UCA0TXBUF = TXByte;
	}
}

// used as loop delay
#pragma vector=TIMER0_A1_VECTOR
__interrupt void TA1_ISR(void) {
	switch (TAIV) {
	case 2:
		TACCTL1 &= ~CCIFG;
		// No more interrupts for now
		TACCTL1 = 0;
		__bic_SR_register_on_exit(LPM3_bits);
		break;
	case 10:
		// Timer overflow, increment mult
		/*
		if (mult++ > 20) {
			end[n] = 0;
			P2IE = 0;
			P2OUT |= TRIG1 + TRIG2 + TRIG3;
			__bic_SR_register_on_exit(LPM0_bits);
		}
		TACTL &= ~TAIFG;
		*/
		break;
	}
}

#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void) {
	if (P2IFG & ECHO) {
		end[n] = TAR;
		P2IFG &= ~ECHO;
		P2IE = 0;
		P2OUT |= TRIG1 + TRIG2 + TRIG3;
		__bic_SR_register_on_exit(LPM0_bits);
	}
}
