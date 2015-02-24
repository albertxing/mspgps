#include "msp430.h"

#define	LED1	BIT0
#define	LED2	BIT6

#define	BUTTON	BIT3
#define	TRIGGER	BIT4
#define	ECHO	BIT5
#define	ECHOS	BIT7

#define	TXD		BIT2
#define	RXD		BIT1

unsigned int mult;
unsigned int start_time;
unsigned int end_time;

void setup ();
void transmit (char*);
