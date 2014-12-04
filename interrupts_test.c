/* interrupts_test.c
 * Created by Skyler Williams 12/02/2014
 * Test file for getting interrupts to work correctly at 44.1kHz
 *
 */


#include <stdio.h>
#include <P32xxxx.h>


int second_counter = 0;
unsigned char elapsed_seconds = 0;


// Initial register configuration
void initConfigs() {
    //  Assumes peripheral clock at 20MHz
    
    //  Use Timer1 for note duration
    //  T1CON
    //  bit 15: ON=0: don't enable timer
    //  bit 14: FRZ=0: keep running in exception mode
    //  bit 13: SIDL = 0: keep running in idle mode
    //  bit 12: TWDIS=1: ignore writes until current write completes
    //  bit 11: TWIP=0: don't care in synchronous mode
    //  bit 10-8: unused
    //  bit 7:  TGATE=0: disable gated accumulation
    //  bit 6:  unused
    //  bit 5-4: TCKPS=00: 1:1 prescaler
    //  bit 3:  unused
    //  bit 2:  don't care in internal clock mode
    //  bit 1:  TCS=0: use internal peripheral clock
    //  bit 0:  unused
    T1CON = 0b00010000000000;
    TMR1 = 0;

    // Initialize PORTD (LEDs) for testing output
    TRISD = 0;
    PORTD = 0;

    INTCON = 0x00000001; // Set single vec mode, rising edge on ext int 0
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1; // Enable Timer1 Interrupt

    PR1 = 454; // Set for 44.1kHz from a 20MHz clock

    T1CONbits.ON = 1; // Start the Timer
}


// Single Vector Interrupt Handler
void __ISR_SINGLE() SingleVecHandler(void) {
    if IFS0bits.T1IF {
        TMR1 = 0; // Reset Timer1
        ISF0bits.T1IF = 0; // Reset Timer1 interrupt
        ++second_counter;
        if second_counter == 44100 {
            ++elapsed_seconds;
            PORTD = elapsed_seconds;
        }
    } 
}


int main () {
    initConfigs();
    // Loop infinitely to see if interrupts increment global variables
    // and if variables are displayed on the LEDs
    while (1) {} 
    return 0;
}
