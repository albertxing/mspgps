#include "main.h"

int main (void) {
	setup();

	// Main application loop
	while (1) {
		lock = 2;

		// Send trigger pulse
		P2OUT |= TRIGGER;
		__delay_cycles(10);
		P2OUT &= ~TRIGGER;

		TACTL = TACLR | TASSEL_2 | MC_2 | TAIE;
		TACCTL1 = 0;
		mult = 0;

		// Wait for echo
		__bis_SR_register(LPM0_bits + GIE);

		float result = (end2 - end1) / 235.1;
		transmit((char*)&result);

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
	BCSCTL1 = CALBC1_8MHZ;		// use 8MHz clock
	DCOCTL = CALDCO_8MHZ;
	BCSCTL2 &= ~(DIVS_3);		// no divider for smclk
	BCSCTL2 |= DIVM_3;			// divide by 8 for mclk, operate at 1MHz

	// Set up LEDs
	P1DIR |= LED1;
	P1OUT &= ~LED1;

	P2DIR |= TRIGGER;
	P2DIR &= ~(ECHO1 + ECHO2);
	P2REN = ECHO1 + ECHO2;
	P2IES = ECHO1 + ECHO2;
	P2IE = ECHO1 + ECHO2;

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
		if (mult++ > 22)
			__bic_SR_register_on_exit(LPM0_bits);
		TACTL &= ~TAIFG;
		break;
	}
}

#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void) {
	if (P2IFG & ECHO1) {
		end1 = TAR + (mult - 1) * 0xffffu;
		P2IFG &= ~ECHO1;
		lock--;
	}

	if (P2IFG & ECHO2) {
		end2 = TAR + (mult - 1) * 0xffffu;
		P2IFG &= ~ECHO2;
		lock--;
	}

	if (!lock) {
		TACTL = 0;
		__bic_SR_register_on_exit(LPM0_bits);
	}
}
