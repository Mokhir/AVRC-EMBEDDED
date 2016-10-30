/*
 * IR PULSE SYNC RECEIVER ATTINY13A.c
 *
 * Created: 7/17/2016 5:34:44 PM
 * Author : Davit Balagyozyan
 */ 


#define F_CPU 9600000UL
#include <avr/io.h>   
#include <util/delay.h>
#include <avr/interrupt.h>  

/* ISR routine variables for IR Pulse timing */
int inputStateA=0b000000000;
int previousStateA=0b000000000;
int timingA_in_prog=0;
int pulseA_start=0;
int pulseA_length=0;
volatile int detected_A=0;
int outputA_in_prog=0;

/* Integrate and Fire variables*/
volatile int charge = 0;
int jumpStep = 100;
int maxCharge = 1000;



int main(void){
	DDRB = 0b011100; //set pins 2, 3 , 4 as outputs

	/* Flash status LED to indicate processor is working 
	PORTB = PORTB | 0b000100; _delay_ms(3); PORTB = PORTB & ~0b000100; _delay_ms(50);
	PORTB = PORTB | 0b000100; _delay_ms(3); PORTB = PORTB & ~0b000100;
	*/
	
	//Pin Change Interrupt Config
	SREG  =  SREG | 0b10000000;	//sreg bit 7 enables global interrupts
	MCUCR = MCUCR | 0b00000000;  //enable logical change trigger MCU control reg 
	//Dec2015 - datasheet indicates MCUCR only affects sleep-mode and power reg stuff.
	GIMSK = 0b00100000;  // Set PCIE to enable pin change interrupts
	PCMSK=0b00000011;  //enable pin change interrupts on pcint0 and pcint1
	TCCR0A = TCCR0A; // no change to default settings
	TCCR0B = TCCR0B | 0b00000101; // CS02 = 1 AND CS00 = 1 for 1024 prescalar
	//2.4ms sony header pulse gives 22.5 counts at 9.6mhz/1024 
	sei();
	
	/* SYNC RECEIVER */
	while(1){
		/* Option A */
		/*
		charge++; // [increment of 1ms]
		_delay_us(1000);
		PORTB = PORTB & ~0b011000;
		/
		*	ISR RUNNING CHECKING FOR SYNC PULSES
		*
		if(detected_A == 1){
			charge = charge + jumpStep; // jumpStep = 100; [100ms]
			detected_A = 0;
		}
		if(charge >= maxCharge){ // maxCharge = 550; [550ms]
			PORTB = PORTB | 0b011000; // PIN THAT TRIGGERS TRANSMITTER TO FIRE
			charge = 0;
		}
		*/
		
		/* Option B */
		charge++; // [increment of 0.5ms]
		_delay_us(500);
		/*
		*	ISR RUNNING CHECKING FOR SYNC PULSES
		*/
		if(detected_A == 1){
			//charge = charge + jumpStep; // jumpStep = 100; [100ms]
			
			PORTB = PORTB | 0b011000; // PIN THAT TRIGGERS TRANSMITTER TO FIRE
			_delay_us(100); // 0.1ms delay for Transmitter to check input register and fire
			PORTB = PORTB & ~0b011000;
			charge=0;
			detected_A = 0;
		}
		else if(charge == maxCharge){ // maxCharge = 1000; [500ms]
			PORTB = PORTB | 0b011000; // PIN THAT TRIGGERS TRANSMITTER TO FIRE
			_delay_us(100); // 0.1ms delay for Transmitter to check input register and fire
			PORTB = PORTB & ~0b011000;
			charge = 0;
		}
		
	}
		
return 1;
}

ISR (PCINT0_vect){  //Pin change interrupt vector		
	inputStateA = PINB & 0b00000001; // << PB0 is the one we use for IRfly_R	
	//PORTB ^= 0b00000100;		
    if(inputStateA == 0b00000000){ // FALLING EDGE
		if((timingA_in_prog == 0)){
			TCNT0=0;
			pulseA_start=TCNT0;
			pulseA_length=0;
			timingA_in_prog = 1; //set flag for A
		}
	}
	if(inputStateA == 0b00000001){ // RISING EDGE
		if((timingA_in_prog == 1)){ 
			pulseA_length = TCNT0 - pulseA_start;
			timingA_in_prog = 0; //clear flag for A
					
			if((pulseA_length <= 26) && (pulseA_length >= 20)){
				detected_A=1;
			}
			else{
//				detected_A=0;
			}
		}
	}
	//Flip status pin again so it doesn't get too bright or stay on
	//PORTB ^= 0b00000100;		
}

