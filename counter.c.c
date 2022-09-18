/*
 * trial..c
 *
 *  Created on: Sep 18, 2022
 *      Author: Nada
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

unsigned char g_tick = 0, num1 = 0, num2 = 0, flag = 0;
void increment();

/*
 * each 1 second interrupt happens.
 * increment is called to display the correspond number.
 */

ISR (TIMER0_COMP_vect) {
	g_tick++;
	if (g_tick == 4) {
		increment();
		g_tick = 0;
	}
	if (flag == 1) {
		TCCR0 = 0;
		flag = 0;
	}
}

//reset
ISR (INT0_vect) {
	num1 = num2 = 0;
}

/*
 * disable timer clock to pause counting.
 */
ISR (INT1_vect) {
	if (!(PIND & (1 << PD3))) {
		flag = 1;
	}
}

//resume counting

ISR (INT2_vect) {
	TCCR0 = (1 << WGM01) | (1 << FOC0) | (1 << CS00) | (1 << CS02);
}

/*
 * interrupt 0 initialization
 * internal pull up activate
 * enable interrupt 0
 * with falling edge
 */

void int0_int() {
	DDRD &= ~(1 << PD2);		  //INPUT
	PORTD |= (1 << PD2);
	GICR |= (1 << INT0);
	MCUCR |= (1 << ISC01);
	MCUCR |= (1 << ISC00);
}

/*
 * interrupt 1 initialization
 * internal pull up activate
 * enable interrupt 1
 * with falling edge
 */

void int1_int() {
	DDRD &= ~(1 << PD3);		  //INPUT
	PORTD |= (1 << PD3);
	GICR |= (1 << INT1);
	MCUCR |= (1 << ISC11);
	MCUCR &= ~(1 << ISC10);
}

/*
 * interrupt 2 initialization
 * external pull up with PB2
 * enable interrupt 2
 * with falling edge
 */

void int2_int() {
	DDRB &= ~(1 << PB2);
	PORTB &= ~(1 << PB2);
	GICR |= (1 << INT2);
	MCUCSR &= ~(1 << ISC2);		//FALLING EDGE
}

/*
 * timer 0 initialization
 * TCNT0->initial value 0
 * OCR0->compare with 250,which means 0.25 sec.
 * pre-scaler 1024
 * TIMSK->enable module interrupt bit
 */

void timer0_init() {
	TCNT0 = 0;
	OCR0 = 250;
	TCCR0 = (1 << WGM01) | (1 << FOC0) | (1 << CS00) | (1 << CS02);
	TIMSK |= (1 << OCIE0);
}

/*
 * calculation of displayed number
 */

void increment() {
	if (num1 == 9) {
		num1 = 0;
		if (num2 == 9) {
			num1 = num2 = 0;
		} else {
			num2++;
		}
	} else {
		num1++;
	}
}

/*
 * counter always count from 0 to 99 and back, using round robin algorithm, enabling each pin of port A each 1 ms, 7 segment in Multiplexing mode.
 * it appears as always displaying.
 */

void display() {
	PORTA = 0x01;
	PORTC = (PORTC & 0x80) | (num1 & 0x0F);
	_delay_ms(1);
	PORTA = 0x02;
	PORTC = (PORTC & 0x80) | (num2 & 0x0F);
	_delay_ms(1);
}

int main() {
	/*
	 * configuring portA,C as output ports,displaying 0 at first.
	 */

	DDRA |= 0x03;
	DDRC |= 0x7F;

	PORTC = 0;
	PORTA = 0;
	DDRD |= (1 << PD0);
	PORTD &= ~(1 << PD0);

	timer0_init();
	int0_int();
	int1_int();
	int2_int();

	SREG |= (1 << 7);

	while (1) {
		display();
	}
	return 0;
}

