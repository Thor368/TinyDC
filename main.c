/*
 * bumpercar.c
 *
 * Created: 03.09.2021 13:00:08
 * Author : AUKTORAAlexanderSchr
 */ 

#define F_CPU					9600000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>

#define I_max					101   // 10A = 50mV/A 5Vref 10bit
#define Acc_min					204
#define Acc_max					820
#define Acc_range				(Acc_max - Acc_min)

volatile uint16_t Acc = 0;
volatile int16_t I = 0;
volatile bool loop = false;

uint16_t I_offset = 0;

ISR(ADC_vect)
{
	if (ADMUX == 2)
	{
		ADMUX = 3;
#ifdef INVERT_ACC
		Acc = 1023 - ADC;
#else
		Acc = ADC;
#endif
	}
	else
	{
		ADMUX = 2;
		I = ADC;

		loop = true;
	}
}

int main(void)
{
	DDRB = 0b10;
	PORTB = 0;
	OCR0B = 0;
	TCCR0A = 0b100001;
	TCCR0B = 0b1;
	ACSR = 0b10000000;
	ADMUX = 2;
	ADCSRA = 0b11111110;
	ADCSRB = 0;
	
	sei();
	
// 	_delay_ms(100);
// 	uint32_t I_offset_filt = 0;
// 	for (uint8_t i = 0; i < 255; i++)
// 	{
// 		I_offset_filt -= I_offset_filt >> 6;
// 		I_offset_filt += I;
// 		_delay_ms(1);
// 	}
// 	I_offset = I_offset_filt >> 6;
	I_offset = 512;
	
    while (1) 
    {
		if (loop)
		{
			loop = false;
			
				OCR0B = 127;
// 			if (Acc <= Acc_min)
// 			{
// 				TCCR0A = 0b1;
// 				OCR0B = 0;
// 			}
// 			else
// 			{
// 				TCCR0A = 0b100001;
// 				
// 				int16_t I_target_c;
// 				if (Acc >= Acc_max)
// 					I_target_c = I_max + I_offset;
// 				else
// 					I_target_c = (Acc - Acc_min)*I_max/Acc_range + I_offset;
// 					
// 				if ((I > I_target_c) && (OCR0B > 0))
// 					OCR0B--;
// 				else if ((I < I_target_c) && (OCR0B < 255))
// 					OCR0B++;
// 			}
		}
    }
}

