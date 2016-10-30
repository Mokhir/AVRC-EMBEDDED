/*
 * IR PULSE COUNTER ATMEGA328.c
 *	FOR ATMEGA328P
 * Created: 7/18/2016 12:54:04 AM
 * Author : Mok
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>

/* USART Serial Variable */
#define BaudRate 9600 // Arduino IDE monitor reads at 9600baud
#define MYUBRR (F_CPU / 16 / BaudRate) - 1
char String[] = "\n Error: ";
char newline = '\n';
char dash = '-';
char buffer[5] = {'5','_','5','\n','\0'};
char error[8];


/* Infrared signal ISR variables */
int inputStateA=0b000000000;
int timingInProg=0;
int pulseStart=0;
int pulseLength=0;
int waitStart=0;
int waitLength=0;

/* Counter variables */
int FirstBit = 0;
int SecondBit = 0;


/*	PARTS FOR SIMPLER IR PULSE COUNTER IN ATTINY85 */
/*
//#define PWMPIN PB1
volatile int pwmvalue=0;
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
*/
void USART_init(unsigned int ubrr){
	/* Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	/* Enabled receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame formateL 8data, 2stop bit */
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}
void USART_Transmit(unsigned char data){
	/* Wait for empty transmit buffer */
	while( !(UCSR0A & (1<<UDRE0)) );
	/* Put the data into buffer, sends the data */
	UDR0 = data;
}
unsigned char USART_Receive(void){
	/* Wait for data to be received */
	while( !(UCSR0A & (1<<RXC0)) );
	/* Get and return received data from buffer */
	return UDR0;
}
void USART_Flush(void){
	unsigned char dummy;
	while( UCSR0A & (1<<RXC0))
	dummy = UDR0;
}
void USART_putstring(char* p){
	while(*p != 0x00){
		USART_Transmit(*p);
		p++;
	}
}

void intForSerial(uint16_t n){
	itoa(n,buffer,10);
	USART_putstring(buffer);
	USART_Transmit(newline);
}

void printPair(){
	if(FirstBit == 2){
		buffer[0] = '1';
	}
	else{
		buffer[0] = '0';
	}
	if(SecondBit == 2){
		buffer[2] = '1';
	}
	else{
		buffer[2] = '0';
	}
	USART_putstring(buffer);
}

void printError(uint16_t n){
	USART_putstring(String);
	itoa(n,error,10);
	USART_putstring(error);
	USART_Transmit(newline);
}


int main(void){
	DDRB = 0b00001000; // PB3 is the output status LED port
	USART_init(MYUBRR);
	//Pin Change Interrupt Config
	SREG  = SREG | 0b10000000;
	PCMSK0 = 0b00000001; // Enables pin change interrupts on pcint0
	PCICR = 0b00000001; // Any logical change on pcint0 triggers interrupt
	PCIFR = 0b00000001;
	TCCR0A = TCCR0A;
//	TCCR0B |= (1<<CS02) | (1<<CS00); // Sets timer 1  to run with 1024 scaled
	TCCR0B |= (1<<CS02); // Sets timer 1  to run with 1024 scaled

	sei();
	
	while(1){
//		USART_putstring(String);
//		_delay_ms(5000);
//		USART_Flush();
	}
	return 1;
}

ISR (PCINT0_vect){
	inputStateA = PINB & 0b00000001;
	//	pwm_write(50);
	// << PB0 is the one we use for IRfly_R

	/* LOW */
	if(inputStateA == 0b00000000){
		if((timingInProg == 0)){
			waitLength = TCNT0 - waitStart;
			TCNT0=0;
			pulseStart=TCNT0;
			pulseLength=0;
			timingInProg = 1;
			
			/* STATE 2 */
			if(FirstBit > 0){ // FIRSTBIT = 1 OR 2
				if(waitLength >= 50 && waitLength <= 80){
					// ADVANCE TO STATE 3
				}
				else{
					FirstBit = 0; // RESET TO STATE 1
				}
			}
		}
	}
	
	/* HIGH */
	if (inputStateA == 0b00000001){
		if((timingInProg == 1)){
			pulseLength= TCNT0 - pulseStart;
			timingInProg = 0; //clear flag for A
			waitStart = TCNT0;
			if(pulseLength >120)
			USART_Transmit(newline);

			/* STATE 1 */
			if(FirstBit < 1){ // FIRSTBIT = 0
				if(pulseLength >= 24 && pulseLength <= 40){
					FirstBit = 1; // ADVANCE TO STATE 2
				}
				else if(pulseLength >= 50 && pulseLength <= 80){
					FirstBit = 2; // ADVANCE TO STATE 2
				}
			}
			
			/* STATE 3 */
			else if(FirstBit > 0 && SecondBit < 1){ // FIRSTBIT = 1 OR 2 and SECONDBIT = 0
				if(pulseLength >= 24 && pulseLength <= 40){
					SecondBit = 1; 
					printPair();
				}
				else if(pulseLength >= 50 && pulseLength <= 80){
					SecondBit = 2;
					printPair();
				}
				else{
					printError(pulseLength);
				}
				FirstBit = 0;
				SecondBit = 0;
			}
		}
	}
}

