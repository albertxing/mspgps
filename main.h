#include "msp430.h"

#define	LED1	BIT0

#define TRIG1 BIT0
#define TRIG2 BIT1
#define TRIG3 BIT2

#define ECHO BIT3

#define	TXD		BIT2
#define	RXD		BIT1

unsigned int n;
unsigned int* dest;
unsigned int mult;
unsigned int start[3];
unsigned int end[3];

float da;
float db;
float dc;

void setup ();
void transmit (char*);
