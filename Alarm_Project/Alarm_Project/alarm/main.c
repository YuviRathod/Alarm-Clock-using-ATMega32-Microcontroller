/* Digital alarm clock using atmega32*/

#include <avr/io.h>
#define F_CPU 11059200
#include <util/delay.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>

#define enable 5
#define registerselection 6

void send_a_command(unsigned char command);
void send_a_character(unsigned char character);
void send_a_string(char *string_of_characters);

ISR(TIMER1_COMPA_vect);

static volatile int SEC = 0;
static volatile int MIN = 0;
static volatile int HOU = 0;

int main(void)
{
    // Set up ports
    DDRA = 0b11000000; // PA6 and PA7 as output, others as input
    DDRB = 0xFF;       // PORTB as output for LCD
    DDRD = 0xFF;       // PORTD as output for LCD control

    // Blink the LED to test if the LED hardware is working
  

    // Set up Timer1
    TCCR1B |= (1 << CS12) | (1 << CS10) | (1 << WGM12); // Prescaler 1024, CTC mode
    OCR1A = 10800;                                      // Compare value for 1 second
    sei();                                              // Enable global interrupts
    TIMSK |= (1 << OCIE1A);                             // Enable Timer1 compare interrupt

    char SHOWSEC[2];
    char SHOWMIN[2];
    char SHOWHOU[2];

    int ALSEC = 0;
    int ALMIN = 0;
    int ALHOU = 0;
    char SHOWALSEC[2];
    char SHOWALMIN[2];
    char SHOWALHOU[2];

    // Initialize LCD
    send_a_command(0x01); // Clear Screen
    _delay_ms(50);
    send_a_command(0x38); // 8-bit mode
    _delay_ms(50);
    send_a_command(0b00001100); // Display ON, Cursor OFF
    _delay_ms(50);

    while (1)
    {
        // Display current time
        itoa(HOU / 10, SHOWHOU, 10);
        send_a_string(SHOWHOU);
        itoa(HOU % 10, SHOWHOU, 10);
        send_a_string(SHOWHOU);
        send_a_string(":");
        send_a_command(0x80 + 3);

        itoa(MIN / 10, SHOWMIN, 10);
        send_a_string(SHOWMIN);
        itoa(MIN % 10, SHOWMIN, 10);
        send_a_string(SHOWMIN);
        send_a_command(0x80 + 5);
        send_a_string(":");
        send_a_command(0x80 + 6);

        itoa(SEC / 10, SHOWSEC, 10);
        send_a_string(SHOWSEC);
        itoa(SEC % 10, SHOWSEC, 10);
        send_a_string(SHOWSEC);

        // Check alarm status
        if (bit_is_set(PINA, 5))
        {
            send_a_string(" ALM:ON ");
            if ((ALHOU == HOU) & (ALMIN == MIN) & (ALSEC == SEC))
            {
                PORTA |= (1 << PA7); // Turn on LED
            }
        }
        if (bit_is_clear(PINA, 5))
        {
            send_a_string(" ALM:OFF");
            PORTA &= ~(1 << PA7); // Turn off LED
        }
        send_a_command(0x80 + 0x40 + 0);

        // Display alarm time
        send_a_string("ALARM:");
        send_a_command(0x80 + 0x40 + 7);

        itoa(ALHOU / 10, SHOWALHOU, 10);
        send_a_string(SHOWALHOU);
        itoa(ALHOU % 10, SHOWALHOU, 10);
        send_a_string(SHOWALHOU);
        send_a_command(0x80 + 0x40 + 9);
        send_a_string(":");
        send_a_command(0x80 + 0x40 + 10);

        itoa(ALMIN / 10, SHOWALMIN, 10);
        send_a_string(SHOWALMIN);
        itoa(ALMIN % 10, SHOWALMIN, 10);
        send_a_string(SHOWALMIN);
        send_a_command(0x80 + 0x40 + 12);
        send_a_string(":");
        send_a_command(0x80 + 0x40 + 13);

        itoa(ALSEC / 10, SHOWALSEC, 10);
        send_a_string(SHOWALSEC);
        itoa(ALSEC % 10, SHOWALSEC, 10);
        send_a_string(SHOWALSEC);
        send_a_command(0x80 + 0);

        // Time setting mode
        if (bit_is_set(PINA, 4))
        {
            if (bit_is_clear(PINA, 0))
            {
                if (MIN < 60)
                {
                    MIN++;
                    _delay_ms(220);
                }
                if (MIN == 60)
                {
                    if (HOU < 24)
                    {
                        HOU++;
                    }
                    MIN = 0;
                    _delay_ms(220);
                }
            }
            if (bit_is_clear(PINA, 1))
            {
                if (MIN > 0)
                {
                    MIN--;
                    _delay_ms(220);
                }
            }
            if (bit_is_clear(PINA, 2))
            {
                if (HOU < 24)
                {
                    HOU++;
                }
                _delay_ms(220);
                if (HOU == 24)
                {
                    HOU = 0;
                }
            }
            if (bit_is_clear(PINA, 3))
            {
                if (HOU > 0)
                {
                    HOU--;
                    _delay_ms(220);
                }
            }
        }

        // Alarm setting mode
        if (bit_is_clear(PINA, 4))
        {
            if (bit_is_clear(PINA, 0))
            {
                if (ALMIN < 60)
                {
                    ALMIN++;
                    _delay_ms(220);
                }
                if (ALMIN == 60)
                {
                    if (ALHOU < 24)
                    {
                        ALHOU++;
                    }
                    ALMIN = 0;
                    _delay_ms(220);
                }
            }
            if (bit_is_clear(PINA, 1))
            {
                if (ALMIN > 0)
                {
                    ALMIN--;
                    _delay_ms(220);
                }
            }
            if (bit_is_clear(PINA, 2))
            {
                if (ALHOU < 24)
                {
                    ALHOU++;
                }
                _delay_ms(220);
                if (ALHOU == 24)
                {
                    ALHOU = 0;
                }
            }
            if (bit_is_clear(PINA, 3))
            {
                if (ALHOU > 0)
                {
                    ALHOU--;
                    _delay_ms(220);
                }
            }
        }
    }
}

ISR(TIMER1_COMPA_vect)
{
    if (SEC < 60)
    {
        SEC++;
    }
    if (SEC == 60)
    {
        if (MIN < 60)
        {
            MIN++;
        }
        SEC = 0;
    }
    if (MIN == 60)
    {
        if (HOU < 24)
        {
            HOU++;
        }
        MIN = 0;
    }
    if (HOU == 24)
    {
        HOU = 0;
    }
}

void send_a_command(unsigned char command)
{
    PORTB = command;
    PORTD &= ~(1 << registerselection);
    PORTD |= 1 << enable;
    _delay_ms(3);
    PORTD &= ~(1 << enable);
    PORTB = 0xFF;
}

void send_a_character(unsigned char character)
{
    PORTB = character;
    PORTD |= 1 << registerselection;
    PORTD |= 1 << enable;
    _delay_ms(3);
    PORTD &= ~(1 << enable);
    PORTB = 0xFF;
}

void send_a_string(char *string_of_characters)
{
    while (*string_of_characters > 0)
    {
        send_a_character(*string_of_characters++);
    }
}
