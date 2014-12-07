# generateWaveforms.py
# Created by Skyler Williams 11/16/2014
# Waveform generating modules for sawtooth wave, square wave, and phase 
# steps.
# 


# Can get LUTs for sine and triangle from the interwebs, so gotten from there! See sine_lut.txt and triangle_lut.txt
# Assuming our DAC is 12-bits (ouput will be unsigned) and our phase is stored as 16-bits (unsigned)


def generatePhaseSteps(num_period_bits, sample_rate, start_note, stop_note):
    newline_counter = 0
    output_table = ""
    # For the midi notes we desire
    for note in range(start_note, stop_note):
        # Note frequency formula found here: http://newt.phys.unsw.edu.au/jw/notes.html
        note_frequency = 440 * pow(2, (note - 69)/12.0)
        # print note_frequency
        # Ratio of note_freq to sample_freq, multiply by total number of steps in a period
        phase_step = int(round((note_frequency / sample_rate) * (pow(2, num_period_bits))))
        newline_counter += 1
        if newline_counter == 8:
            output_table += str(phase_step) + ",\n"
            newline_counter = 0
        else:
            output_table += str(phase_step) + ","

    f = open('phase_steps.txt', 'w')
    f.write(output_table)
    f.close()
    #print output_table


def generateSawtoothWave(num_period_bits, num_output_bits):
    newline_counter = 0
    output_table = ""

    max_period = pow(2, num_period_bits)
    max_output = pow(2, num_output_bits)

    sawtooth_output = -1 # start at -1 since will increment on period_step == 0
    # Compute the step for the sawtooth, powers of 2 makes this nice
    sawtooth_step = max_period/max_output

    output_table += "// Lookup Table for Sawtooth Waveform\n"
    output_table += "// Generated using python file generateWaveforms.py\n"
    output_table += "// Parameters used for function generateSawtoothWave: num_period_bits = " + str(num_period_bits) + ". num_output_bits = " + str(num_output_bits)

    for period_step in range(max_period):
        if period_step % sawtooth_step == 0:
            sawtooth_output += 1

        newline_counter += 1
        if newline_counter == 16:
            output_table += "0x" + format(sawtooth_output, 'x') + ",\n"
            newline_counter = 0
        else:
            output_table += "0x" + format(sawtooth_output, 'x') + ","

    f = open('sawtooth_lut.txt', 'w')
    f.write(output_table)
    f.close()
    #print output_table


def generateSquareWave(num_period_bits, num_output_bits):
    newline_counter = 0
    output_table = ""

    max_period = pow(2, num_period_bits)
    max_output = pow(2, num_output_bits)
    half_max_period = max_period / 2

    square_output = max_output - 1 # Start at 0, so subtract 1 from max

    output_table += "// Lookup Table for Square Waveform\n"
    output_table += "// Generated using python file generateWaveforms.py\n"
    output_table += "// Parameters used for function generateSquareWave: num_period_bits = " + str(num_period_bits) + ". num_output_bits = " + str(num_output_bits)

    for period_step in range(max_period):
        if period_step == half_max_period:
            square_output = 0

        newline_counter += 1
        if newline_counter == 16:
            output_table += "0x" + format(square_output, 'x') + ",\n"
            newline_counter = 0
        else:
            output_table += "0x" + format(square_output, 'x') + ","

    f = open('square_lut.txt', 'w')
    f.write(output_table)
    f.close()
    #print output_table
    

def main():
    generatePhaseSteps(16, 44054, 21, 109)
    # generateSquareWave(16, 12)
    generateSawtoothWave(16, 16)

if __name__ == "__main__":
    main()