/* interrupts_test.c
 * Created by Skyler Williams 12/02/2014
 * Test file for getting interrupts to work correctly at 44.1kHz
 *
 */


void initTimer() {
    //  Assumes peripheral clock at 5MHz
    
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

    INTCON = 0x00000001; // Set single vec mode, rising edge on ext int 0
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1; // Enable Timer1 Interrupt

    PR1 = 113; // Set for 44.1kHz from a 5MHz clock

    T1CONbits.ON = 1; // Start the Timer
}



// Single Vector Example
void __ISR_SINGLE() SingleVecHandler(void) {
    while(INTSTAT) {
        if IFS0bits.T1IF {
            /* do a thing */
        } 
        else if  IFS0bits.T2IF {
            /* do a different thing */
        }
    }
}



int main () {
    return 0;
}