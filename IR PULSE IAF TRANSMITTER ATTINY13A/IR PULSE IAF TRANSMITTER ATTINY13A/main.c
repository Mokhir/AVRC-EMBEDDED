/*
* CUSTOM TRANSMITTER BECAUSE I ACCIDENTLY OVERWROTE DERRICKS TRANSMITTER CODE
* Davit Balagyozyan
*/


#include <avr/io.h>
#define F_CPU 9600000UL
#include <util/delay.h>
//set fuses - no clkdiv , 9.6mhz clock speed
#include <avr/interrupt.h>

//#define PB4_IR 		0b00010100 //try out status light
#define PB4_IR 		0b00010000 // IR LED
#define PB3_visible 0b00001000 // YELLOW LED
#define PB2_status 	0b00000100
#define PB1_input	0b00000010

/* Array index for bits from PulseSequence */
#define FIRSTBIT 5
#define SECONDBIT 6

/* BEACON ID'S		FIRSTBIT=		SECONDBIT=
* A : YELLOW 00		1				2
* B : GREEN 01		3				4
* C : RED 10		5				6
* D : BLUE 11		7				8
*/

/* Pulse Sequence is the time for transmission ON */
int PulseSequence[] = {24, //Sync pulse (transmitters have to sync)
	06, 06, // ID A: 00
	06, 12, // ID B: 01
	12, 06, // ID C: 10
	12, 12, // ID D: 11
	};
/* Wait Sequence is the time for transmission OFF */
int WaitSequence[] = {156,
	12, 156, 
	12, 156, 
	12, 156, 
	12, 156,
	};
/* DelayMultiplier is the number to multiply sequence numbers to get
	the real-time delay */
int DelayMultipler = 50;

volatile int triggerSignalReceived = 0;
volatile int IRstate=1;
volatile int IRtransmit=0;
volatile int ind=0;
volatile int delay = 600; //Default value, should be changed by fire1pulse()

void delay_us(uint16_t count) {
	/* IMPORTANT */
	/* IN "count" USE ONLY MULTIPLES OF 3 WITH THE delay_ms() FUNCTION OR ELSE WILL FAIL */
	while(count -=3) {
		_delay_us(2);
	}
}

void delay_usNORMAL(uint16_t count) {
	/* IMPORTANT */
	/* IN "count" USE ONLY MULTIPLES OF 3 WITH THE delay_ms() FUNCTION OR ELSE WILL FAIL */
	while(count--) {
		_delay_us(1);
	}
}

void fireSequence(){
	// IRtransmit is a flag to trigger the ISR
	// With new timer, 660 gives a 1.2ms pulse
	/* IMPORTANT */
	/* USE ONLY MULTIPLES OF 3 WITH THE delay_ms() FUNCTION OR ELSE FAILURE */
		
	
	ind=0;
	while(ind<=8){
		/* Calculate and transmit PULSE */
		
		delay = PulseSequence[ind] * DelayMultipler;
		if(ind == FIRSTBIT || ind == SECONDBIT || ind == 0)
			IRtransmit=1;
		delay_us(delay);
	
		/* Calculate and transmit WAIT period */
		delay = WaitSequence[ind] * DelayMultipler;
		IRtransmit=0;delay_us(delay);
		ind++;
	}	
	/* Transmission is about 22.5ms total for the sequence before restart
	* So there is 44Hz of sequence transmitted. Meaning that the resolution has to detect
	* error from 44 instances of ID Signal per second.
	*/
}

int main(void){
	DDRB = 0b00011000; //pb4 and pb3  set as outputs, keep PB2 an input so TransLED doesn't light up too much ?

	//Timer Count/Compare Interrupt Settings
	SREG = 0b10000000;	//sreg bit 7 enables global interrupts
	TCCR0B=0b00000001;  //initialize counter , no clock prescalar
	TIMSK0=0b00000100;  //enable ocie0A
	TCNT0 = 0; //Initialize Timer
	OCR0A=77; //8microsecs
	//OCR0A=154; //16microsecs
	//OCR0A can be changed anywhere dynamically in the code
	sei();
	
	/* TRANSMITTER */
	while(1){	
	/* Option C for IAF */
		triggerSignalReceived = PINB & PB1_input;
		if(triggerSignalReceived == PB1_input){
			PORTB |= PB3_visible;
			fireSequence();
			PORTB &= ~PB3_visible;
			triggerSignalReceived = 0;
		}
	}
	return 1;
}

ISR (TIM0_COMPA_vect)  // Timer interrupt for generating a 38kHz Pulse. Continuously running, 
					   // only output to pin when IRtransmit==1
{
	// ISR is constantly running in the background but IRTransmit controls if it does anything
	cli();
	//sony sirc 38khz carrier wave  pulse high for 8us
	if (IRstate==1){
		if(IRtransmit==1){
			PORTB = PORTB | PB4_IR; //-turn on pb4 - IR LED and pb2 - Transm LED
		}
		else{
			//	PORTB = PORTB & ~0b00010000; //-make sure pb4 is off
		}
		OCR0A=48; //8usec on pulse //Target, output compare register 0 A
		// OCR0A IS THE POWER RATIO. 48 <-> 96 IS JUST A POWER RATIO OF THE 38KHZ SIGNAL
		//Whenever TCNT0 = 48, wakes up, then wait
		IRstate=2;
	}

	//sony sirc 38khz carrier wave pulse low for 16us
	else if (IRstate==2)
	{
		PORTB = PORTB & ~PB4_IR; //-make sure pb4 and pb2 are off
		OCR0A=96; //16usec on pulse
		IRstate=1;
	}

	TCNT0=0;
	sei();
}
