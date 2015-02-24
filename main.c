#include "main.h"

int main (void) {
	setup();

	// Main application loop
	while (1) {
		// Send trigger pulse
		P1OUT |= TRIGGER;
		__delay_cycles(10);
		P1OUT &= ~TRIGGER;

		// Wait for echo
		__bis_SR_register(LPM0_bits + GIE);

		float result = (end_time - start_time + 0xffffp0 * (mult - 1)) / 470;
		transmit((char*)&result);

		// Toggle LED
		P1OUT ^= LED1;

		// Delay between measurements
		TACCR0 = 1000;						// Period
		TACCR1 = 1000;						// Capture/compare at end
		TACTL = TACLR | TASSEL_1 | MC_1;	// TACLK = ACLK, Up mode.
		TACCTL1 = CCIE;						// TACCTL0
		__bis_SR_register(LPM3_bits + GIE);
	}
}

void setup () {
	WDTCTL = WDTPW + WDTHOLD;	// Stop WDT

	// Calibrate clocks
	BCSCTL1 = CALBC1_8MHZ;		// use 8MHz clock
	DCOCTL = CALDCO_8MHZ;
	BCSCTL2 &= ~(DIVS_3);		// no divider for smclk
	BCSCTL2 |= DIVM_3;			// divide by 8 for mclk, operate at 1MHz

	// Set up LEDs
	P1DIR |= LED1 + LED2 + TRIGGER;
	P1OUT &= ~(LED1 + LED2 + TRIGGER + ECHOS);
	P1OUT |= ECHO;

	P1DIR &= ~(ECHO + ECHOS);
	P1REN = ECHO + ECHOS;
	P1IES = ECHO;
	P1IE = ECHO + ECHOS;
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
__interrupt void ta1_isr(void) {
	switch (TAIV) {
	case 2:
		TACCTL1 &= ~CCIFG;
		// No more interrupts for now
		TACCTL1 = 0;
		__bic_SR_register_on_exit(LPM3_bits);
		break;
	case 10:
		// Timer overflow, increment mult
		mult++;
		TACTL &= ~TAIFG;
		break;
	}
}

#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR(void) {
	if (P1IFG & ECHO) {
		if (!P1IES) {
			// TACLK = SMCLK, Continuous mode, enable interrupts
			TACTL = TACLR | TASSEL_2 | MC_2 | TAIE;
			start_time = TAR;
			P1IES |= ECHO;
			P1IFG &= ~ECHO;
			TACCTL1 = 0;
			mult = 0;
		} else {
			end_time = TAR;
			TACTL = 0;
			P1IES &= ~ECHO;
			P1IFG &= ~ECHO;

			__bic_SR_register_on_exit(LPM0_bits);
		}
	}
}
