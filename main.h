#include "msp430.h"

#define	LED1	BIT0

#define	TRIGGER	BIT0
#define	ECHO1	BIT1
#define	ECHO2	BIT2

#define	TXD		BIT2
#define	RXD		BIT1

unsigned int mult;
unsigned int lock;
int end1;
int end2;

void setup ();
void transmit (char*);
