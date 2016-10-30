/*
 * IR PULSE COUNTER ATTINY85.c
 *
 * Created: 7/14/2016 12:08:50 PM
 * Author : Davit B.
 */ 

#define F_CPU 8000000UL
#include <avr/io.h>   
#include <util/delay.h>
#include <avr/interrupt.h>

#define OUTPUT_DURATION 1000
#define BIT_MAX_INDEX 4
#define PWMPIN PB1
#define DELAY 2

/* Infrared signal ISR variables */
int inputStateA=0b000000000;
int timingA_in_prog=0;
int pulseA_start=0;
int pulseA_length=0;
volatile int detected_A=0;

/* Counter variables */
volatile int IRcount=0; //Variable x for number counted
volatile int pwmvalue=0;
volatile int countCopy=0;


int powers[] = {8,4,2,1};
int foo(int *a){
	int res=0,i;
	for(i=3;i>-1;i--){
		res = res + (a[i]*powers[i]);
	}
	return res;
}

void pwm_setup(void){
	// PB0/OCOA (pin5) and PB1/OCOB (pin6), have PWM ability
	// Both running from Timer 0.
	TCCR0B |= (1<<CS01) | (1<<CS00); // Pre-scale factor = 64

	// Set to 'Fast PWM' mode
	TCCR0A |= (1<<WGM01) | (1<<WGM00);

	// Clear OC0B output on compare match, upwards counting
	TCCR0A |= (1<<COM0A1) | (1<<COM0B1);
}
void pwm_write(uint8_t value){
	OCR0B = value;
}

int main(void){
	DDRB = 0b000010; // PB1 is the output PWM pin
					 // PB0 is the input read pin from the transmitter
	
	//Pin Change Interrupt Config
	SREG  =  SREG | 0b10000000;	
	MCUCR = MCUCR | 0b00000000;  
	GIMSK = 0b00100000; // Set PCIE to enable pin change interrupts
	PCMSK = 0b00000001; // Enables pin change interrupts on pcint0
	TCCR1 |= (1<<CS13) | (1<<CS11) | (1<<CS10); // Sets timer 1  to run with 1024 scaled
								  // clock-cycles prescaler

	//Enable interrupts
	sei();
	
	//Enable pwm
	pwm_setup();
//	pwm_write(200);
	
	while(1){
		/* Check the # of pulses */
		countCopy = IRcount;
//		if(countCopy > 50)
			pwmvalue = countCopy; // 50 is a testing value
		/* Set pwm output */
		pwm_write(pwmvalue);
		//reset pulses
		IRcount = 0;
		countCopy = 0;
		//delay 1000ms
		_delay_ms(1000);
		
	}
	return 1;
}

ISR (PCINT0_vect){  
	inputStateA = PINB & 0b00000001; 
//	pwm_write(50);
	// << PB0 is the one we use for IRfly_R
	
	/* LOW */	
	if (inputStateA == 0b00000000){
		//check if we are already timing a pulse
		if ( (timingA_in_prog == 0) )
			TCNT1=0; //not timing anything, reset counter 
		if ( (timingA_in_prog == 0) ){ //not already timing a pulse on A,this is a new falling edge. 
			//set current tcnt0 as pulseA_start
			pulseA_start=TCNT1;
			pulseA_length=0;
			timingA_in_prog = 1; //set flag for A
		}
	}
	
	/* HIGH */	
	if (inputStateA == 0b00000001){
		if((timingA_in_prog == 1)){ 
			pulseA_length= TCNT1 - pulseA_start;
			timingA_in_prog = 0; //clear flag for A
			
			/* 2.4ms pulse, 1024 prescalar */
			if((pulseA_length <= 40) && (pulseA_length >= 20)){
				IRcount++;
//				pwm_write(50);
			}
		}
	else 
		detected_A=0;
		
	}	
}

