/* midi_reader.c
 * Created by Skyler Williams 11/21/2014
 * Reading MIDI input via UART and properly handling 8-note polyphony based 
 * on MIDI input. 
 *
 */

#define READ_STATUS 0
#define READ_DATA_1 1
#define READ_DATA_2 2
#define MIDI_OFF 128
#define MIDI_ON 144

unsigned char midi_status_byte = READ_STATUS;
unsigned char midi_read_status = 0;
unsigned char midi_input = 0;
unsigned char newest_note = 0;
unsigned char newest_velocity = 0;

// Loop through current_notes, if not zero, add to running sum and increment
// number of contributors to sum, in the end divide by the number of contributors.
// Have next_note_index increment by looking for next 0 value
// If incremented 8 times, then stop incrementing and choose that value (the last
// value entered, so the behavior should be more than 8 notes removes last note played)
// Remove notes by looping through and setting to 0
unsigned char current_notes[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // MIDI notes aka 1...127
unsigned char current_phase_steps[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned char current_note_values[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // Fill in the interrupt code
unsigned char next_note_index = 0;

char read_midi_byte();
void increment_note_index();
void process_new_note();


void midi_reader() {

    while (1) {
        midi_input = read_midi_byte(); // Will block until new byte received

        // MIDI Running Status: If you don't get a new status byte after reading 
        // a full two data bytes, infer the previous status type
        switch(midi_read_status) {
            case READ_STATUS : // Read Status Byte
                if (midi_input > 127) {
                    // If the status byte is note off/on
                    if (midi_input == 128 || midi_input == 144) {
                        midi_status_byte = midi_input; 
                        midi_read_status = READ_DATA_1;
                    }
                    break;
                } // If not a status byte, then go into reading 1st data byte
            case READ_DATA_1 : // Read first data byte, note byte
                if (midi_input < 127) {
                    newest_note = midi_input;
                    midi_read_status = READ_DATA_2;
                }
                break;
            case READ_DATA_2 : // Read second data byte, velocity byte
                if (midi_input < 127) {
                    newest_velocity = midi_input;
                    midi_read_status = READ_STATUS;
                    process_new_note();
                }
                break;
        } // end switch(midi_read_status)
    } // end while(1)

} // end midi_reader()


// read_midi_byte
//
char read_midi_byte() {
    return getcharserial();
}


// increment_note_index
void increment_note_index() {
    int i = 1;
    for (; i < 9; ++i) } // For each 8 potential note entries
        if (current_notes[(next_note_index + i) & 7] == 0) {
            next_note_index = next_note_index + i;
        }
    }
    // If none of the notes are 0, do not increment
}


// process_new_note
// Code to process a MIDI note, based on the velocity and status bytes
void process_new_note() {
    // Add new note to the notes array if velocity is non-zero and status is MIDI_ON, 
    // remove otherwise
    if ((midi_status_byte == MIDI_ON) && (newest_velocity != 0)) {
        unsigned char newest_index = next_note_index & 7;
        current_notes[newest_index] = newest_note; // 3 LSBs gives index
        current_phase_steps[newest_index] = phase_steps[newest_note]; // NOTE: this is assuming
        // that the index of the phase steps is the same as the note number, will have to do 
        // some appropriate math once we decide how many notes we are supporting to find the 
        // propper index into phase_steps
        increment_note_index();
    } else { // Check if newest_note is in current_notes and remove if so
        int i = 0;
        for (; i < 8; ++i) {
            if (current_notes[i] == newest_note) {
                current_notes[i] = 0;
                current_phase_steps[i] = 0;
                break;
            }
        }
    }
}


// TODO: Make this a proper initialization for MIDI communication
// initUART
// Code to initialize UART
// This code was taken from the materials given out in E155 by the 
// professors in the file "uart.c"
void initUART(void) 
{
    // Configure UART
    // Using UART3 since nothing else uses PORTF

    TRISFbits.TRISF5 = 0; // RF5 is UART3 TX (output) 
    TRISFbits.TRISF4 = 1; // RF4 is UART3 RX (input)

    // Want rate of 31250 baud
    // #####################################################
    // Assuming PIC peripheral clock Fpb = Fosc / 2 = 20 MHz 
    // based on default instructions in lab 1.
    // #####################################################
    // U3BRG = (Fpb / 16*baud rate) - 1
    // -> U3BRG = 39 (decimal)
    // Actual baud rate 31250
    // Calculation found in GPIO lecture p.25
    U3ABRG = 39;

    // UART3 Mode Register
    // bit 31-16: unused
    // bit 15:  ON = 1: enable UART
    // bit 14:  FRZ = 0: don't care when CPU in normal state
    // bit 13:  SIDL = 0: don't care when CPU in normal state
    // bit 12:  IREN = 0: disable IrDA
    // bit 11:  RTSMD = 0: don't care if not using flow control 
    // bit 10:  unused
    // bit 9-8: UEN = 00: enable U1TX and U1RX, disable U1CTSb and U1RTSb
    // bit 7:   WAKE = 0: do not wake on UART if in sleep mode 
    // bit 6:   LPBACK = 0: disable loopback mode
    // bit 5:   ABAUD = 0: don't auto detect baud rate
    // bit 4:   RXINV = 0: U1RX idle state is high
    // bit 3:   BRGH = 0: standard speed mode
    // bit 2-1: PDSEL = 00: 8-bit data, no parity 
    // bit 0:   STSEL = 0: 1 stop bit 
    U3AMODE = 0x8000;

    // UART3 Status and control register 
    // bit 31-25: unused
    // bit 13: UTXINV = 0: U1TX idle state is high 
    // bit 12: URXEN = 1: enable receiver
    // bit 11: UTXBRK = 0: disable break transmission 
    // bit 10: UTXEN = 1: enable transmitter
    // bit 9: UTXBF: don't care (read-only)
    // bit 8: TRMT: don't care (read-only)
    // bit 7-6: URXISEL = 00: interrupt when receive buffer not empty 
    // bit 5: ADDEN = 0: disable address detect
    // bit 4: RIDLE: don't care (read-only)
    // bit 3: PERR: don't care (read-only)
    // bit 2: FERR: don't care (read-only)
    // bit 1: OERR = 0: reset receive buffer overflow flag
    // bit 0: URXDA: don't care (read-only)
    U3ASTA = 0x1400;
}


// Code to read char from UART
// This code was taken from the materials given out in E155 by the 
// professors in the file "uart.c"
char getcharserial(void) 
{ 
    while (!U3ASTAbits.URXDA);  // wait until data available
    return U3ARXREG;            // return character received from serial port
}

int main() {
    return 0;
}
