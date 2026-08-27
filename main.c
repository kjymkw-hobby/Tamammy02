/*
 * PIC16F1828 - Rocket Exhaust LED Flicker
 *
 * XC8 / MPLAB X
 *
 * 15 LEDs are used:
 *   RA2, RA4, RA5
 *   RB4, RB5, RB6, RB7
 *   RC0, RC1, RC2, RC3, RC4, RC5, RC6, RC7
 *
 * RA0/RA1 are left unused for ICSP (PICkit programming).
 * RA3/MCLR is left unused.
 *
 * LEDs are assumed ACTIVE-HIGH:
 * PIC pin -> resistor -> LED -> GND
 *
 * Use a resistor for every LED. For a first test, 1k ohm is a
 * conservative choice at 5 V.
 */

#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 32000000UL

// CONFIG1
#pragma config FOSC = INTOSC
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config MCLRE = ON
#pragma config CP = OFF
#pragma config CPD = OFF
#pragma config BOREN = ON
#pragma config CLKOUTEN = OFF
#pragma config IESO = OFF
#pragma config FCMEN = OFF

// CONFIG2
#pragma config WRT = OFF
#pragma config PLLEN = OFF
#pragma config STVREN = ON
#pragma config BORV = LO
#pragma config LVP = OFF

#define LED_COUNT 15
#define PWM_LEVELS 16

static volatile uint8_t brightness[LED_COUNT];
static uint8_t target[LED_COUNT];

/*
 * LED order:
 *  0 RA2   1 RA4   2 RA5
 *  3 RB4   4 RB5   5 RB6   6 RB7
 *  7 RC0   8 RC1   9 RC2  10 RC3
 * 11 RC4  12 RC5  13 RC6  14 RC7
 *
 * Higher base values make the exhaust brighter overall.
 * You can rearrange these values to match the physical LED layout.
 */
static const uint8_t base_brightness[LED_COUNT] = {
    15, 15, 14,
    13, 14, 15, 13,
    12, 13, 15, 13,
    12, 13, 12, 11
};

static uint16_t rng_state = 0xACE1u;

/* Small 16-bit pseudo-random generator. */
static uint8_t random4(void)
{
    uint16_t lsb = rng_state & 1u;
    rng_state >>= 1;
    if (lsb) {
        rng_state ^= 0xB400u;
    }
    return (uint8_t)(rng_state & 0x0Fu);
}

/* Clamp a signed value to the 0..15 PWM range. */
static uint8_t clamp_pwm(int16_t value)
{
    if (value < 0)  return 0;
    if (value > 15) return 15;
    return (uint8_t)value;
}

/*
 * Build the three output bytes from the current PWM counter.
 * This gives each LED 16 brightness levels.
 */
static void update_led_outputs(void)
{
    uint8_t a = 0;
    uint8_t b = 0;
    uint8_t c = 0;
    uint8_t p = 0;

    if (brightness[0] > p)  a |= (1u << 2); // RA2
    if (brightness[1] > p)  a |= (1u << 4); // RA4
    if (brightness[2] > p)  a |= (1u << 5); // RA5

    if (brightness[3] > p)  b |= (1u << 4); // RB4
    if (brightness[4] > p)  b |= (1u << 5); // RB5
    if (brightness[5] > p)  b |= (1u << 6); // RB6
    if (brightness[6] > p)  b |= (1u << 7); // RB7

    if (brightness[7]  > p) c |= (1u << 0); // RC0
    if (brightness[8]  > p) c |= (1u << 1); // RC1
    if (brightness[9]  > p) c |= (1u << 2); // RC2
    if (brightness[10] > p) c |= (1u << 3); // RC3
    if (brightness[11] > p) c |= (1u << 4); // RC4
    if (brightness[12] > p) c |= (1u << 5); // RC5
    if (brightness[13] > p) c |= (1u << 6); // RC6
    if (brightness[14] > p) c |= (1u << 7); // RC7

    LATA = a;
    LATB = b;
    LATC = c;
}

static volatile uint8_t pwm_counter = 0;

void __interrupt() isr(void)
{
    if (INTCONbits.TMR0IF) {
        INTCONbits.TMR0IF = 0;

        /*
         * 32 MHz / 4 = 8 MHz instruction clock.
         * TMR0 prescaler = 1:8.
         * Overflow period = 256 us.
         * 16-level software PWM = about 244 Hz.
         */
        pwm_counter++;
        if (pwm_counter >= PWM_LEVELS) {
            pwm_counter = 0;
        }

        /*
         * The comparison uses a local copy of the counter so the
         * outputs are changed as one group.
         */
        {
            uint8_t p = pwm_counter;
            uint8_t a = 0;
            uint8_t b = 0;
            uint8_t c = 0;

            if (brightness[0] > p)  a |= (1u << 2);
            if (brightness[1] > p)  a |= (1u << 4);
            if (brightness[2] > p)  a |= (1u << 5);

            if (brightness[3] > p)  b |= (1u << 4);
            if (brightness[4] > p)  b |= (1u << 5);
            if (brightness[5] > p)  b |= (1u << 6);
            if (brightness[6] > p)  b |= (1u << 7);

            if (brightness[7]  > p) c |= (1u << 0);
            if (brightness[8]  > p) c |= (1u << 1);
            if (brightness[9]  > p) c |= (1u << 2);
            if (brightness[10] > p) c |= (1u << 3);
            if (brightness[11] > p) c |= (1u << 4);
            if (brightness[12] > p) c |= (1u << 5);
            if (brightness[13] > p) c |= (1u << 6);
            if (brightness[14] > p) c |= (1u << 7);

            LATA = a;
            LATB = b;
            LATC = c;
        }
    }
}

static void init_pic(void)
{
    /*
     * 8 MHz HFINTOSC + 4x PLL = 32 MHz.
     * FOSC is INTOSC, so SCS=00 uses the configuration-selected clock.
     */
    OSCCON = 0b11110000;

    /* All analog-capable pins used here are forced to digital I/O. */
    ANSELA = 0x00;
    ANSELB = 0x00;
    ANSELC = 0x00;

    /*
     * RA2, RA4, RA5 = outputs.
     * RA0, RA1 = left free for ICSP.
     * RA3 = MCLR.
     */
    TRISA = 0b00001011;

    /* RB4-RB7 = outputs. */
    TRISB = 0x00;

    /* RC0-RC7 = outputs. */
    TRISC = 0x00;

    LATA = 0x00;
    LATB = 0x00;
    LATC = 0x00;

    /*
     * Timer0:
     * internal instruction clock (Fosc/4)
     * prescaler 1:8
     */
    OPTION_REGbits.T0CS = 0;
    OPTION_REGbits.T0SE = 0;
    OPTION_REGbits.PSA  = 0;
    OPTION_REGbits.PS   = 0b010;

    TMR0 = 0;
    INTCONbits.TMR0IF = 0;
    INTCONbits.TMR0IE = 1;
    INTCONbits.PEIE = 0;
    INTCONbits.GIE = 1;
}

static void init_effect(void)
{
    uint8_t i;

    for (i = 0; i < LED_COUNT; i++) {
        brightness[i] = base_brightness[i];
        target[i] = base_brightness[i];
    }
}

/*
 * Update the "flame" every 20 ms.
 *
 * Each LED gets a slightly different random target.
 * Occasionally a stronger pulse is generated, giving a more
 * convincing rocket-exhaust / engine-flame effect.
 */
static void update_effect(void)
{
    uint8_t i;

    for (i = 0; i < LED_COUNT; i++) {
        int16_t variation = (int16_t)(random4() & 0x07u) - 3;
        int16_t next = (int16_t)base_brightness[i] + variation;

        /*
         * About once every 16 updates, make one LED flare strongly.
         * This creates irregular "licks" in the exhaust.
         */
        if ((random4() & 0x0Fu) == 0u) {
            next += 3;
        }

        target[i] = clamp_pwm(next);

        /* Smoothly move the actual brightness toward the target. */
        if (brightness[i] < target[i]) {
            brightness[i]++;
        } else if (brightness[i] > target[i]) {
            brightness[i]--;
        }
    }
}

void main(void)
{
    init_pic();
    init_effect();

    while (1) {
        update_effect();
        __delay_ms(20);
    }
}
