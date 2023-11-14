/*
 * Created: 03.09.2021 13:00:08
 * Author : Alexander Schroeder
 * eTrabi
 */ 

#define F_CPU					9600000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>

//#define INVERT_CUR
//define INVERT_ACC

#define SOFT_FUSE				1000

#define TIMER_ON				((1 << COM0A1) | (1 << WGM01) | (1 << WGM00))
#define TIMER_INIT				((1 << WGM01) | (1 << WGM00))
#define OCR						OCR0A

#define I_max					16   // 4mV/A -> 500A max
#define Acc_min					150
#define Acc_max					920
#define Acc_range				(Acc_max - Acc_min)

volatile uint16_t Acc = 0;
volatile int16_t I = 0;

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
#ifdef INVERT_CUR
		I = 1023 - ADC;
#else
		I = ADC;
#endif

#ifdef SOFT_FUSE
		if (I >= SOFT_FUSE)
		{
			TCCR0A = TIMER_INIT;
			OCR = 0;
			cli();
			while(1);
		}
#endif
	}
}

int main(void)
{
	DDRB = 0b1;
	PORTB = 0;
	OCR = 0;
	TCCR0A = TIMER_INIT;
	TCCR0B = (1 << CS01);
	ACSR = 0b10000000;
	ADMUX = 2;
	ADCSRA = 0b11111110;
	ADCSRB = 0;
	
	sei();
	
	_delay_ms(500);
	uint16_t I_offset_filt = I << 6;
	for (uint16_t i = 0; i < 500; i++)
	{
		I_offset_filt -= I_offset_filt >> 6;
		I_offset_filt += I;
		_delay_ms(1);
	}
	I_offset = I_offset_filt >> 6;

// 	TCCR0A = TIMER_ON;
// 	OCR = I_offset >> 2;

    while (1)
    {
		if (TIFR0 & (1 << TOV0))
		{
			TIFR0 |= 1 << TOV0;
			
			if (Acc <= Acc_min)
			{
				TCCR0A = TIMER_INIT;
				OCR = 0;
			}
			else
			{
				TCCR0A = TIMER_ON;
				
				int32_t I_target_c;
				if (Acc >= Acc_max)
					I_target_c = I_max + I_offset;
				else
					I_target_c = (((uint32_t) Acc) - Acc_min)*I_max/Acc_range + ((uint32_t) I_offset);
					
				if ((I > I_target_c) && (OCR > 0))
					OCR--;
				else if ((I < I_target_c) && (OCR < 255))
					OCR++;
			}
		}
    }
}