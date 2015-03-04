#include "msp430.h"

#define	LED1	BIT0

#define	TRIGGER	BIT0
#define	ECHO1	BIT1
#define	ECHO2	BIT2

#define	TXD		BIT2
#define	RXD		BIT1

unsigned int mult;
unsigned int lock;
unsigned int end1;
unsigned int end2;

void setup ();
void transmit (char*);
