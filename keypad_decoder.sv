/* Name & Email: Skyler Williams (szwilliams@hmc.edu)
 * Date of Creation: 09/24/2014
 *
 *  Modules for E155 Final Project Keypad Decoding
 */



/* Name & Email: Skyler Williams (szwilliams@hmc.edu)
 * Date of Creation: 09/25/2014, modified 11/21/2014
 * 
 * Module Summary: This is the structural module for the entire lab. 
 */
module keypad_decoder(input logic clk,
                input logic [3:0]col,
                output logic [3:0]row,
                output logic [1:0]waveform_select,
                output logic [1:0]filter_select,
                output logic [2:0]filter_params,
                output logic [7:0]amp_envelope);
    
    // Initialize internal keypad value
    logic [5:0]keypad = 6'd0;

    // Scans the keypad for presses, sending them in a predefined format 
    // to keypad
    keypadScanner scanKeypad(clk, col, row, keypad);

    // Initialize internal variables for passing keypress information
    logic keyPressed = 1'b0;

    // Combinatorial logic determine if a key is being pressed
    keypressCheck isKeypressed(keypad, keyPressed);

    // Initialize the keypad-type pressed memories
    // Think these should be 3-bits since we only care about up to decimal 4 for each
    logic [2:0]mode_key = 3'd0;
    logic [2:0]ctrl_key = 3'd0;
    logic [2:0]row_key = 3'd0;

    // Debounce switch signal and deal with storing keypresses in memory
    debouncer debounceKeypad(clk, keyPressed, keypad, mode_key, ctrl_key, row_key);

    // Initialize parameters controlled by the keypad
    //waveform_select = 2'd0;
    //filter_select = 2'd0;
    //filter_params = 4'd0;
    //amp_envelope = 8'd0;

    // Act on the keyPressed according to the stored mode and control keys
    // This will have more outputs when we decide how exactly to output 
    // filter frequency and ASDR envelope
    keypadAction processEvent(keyPressed, mode_key, ctrl_key, row_key, waveform_select, filter_select, filter_params, amp_envelope);

endmodule



/* Name & Email: Skyler Williams (szwilliams@hmc.edu)
 * Date of Creation: 09/25/2014
 * 
 * Module Summary: Scanning circuit to extract keypresses from the 16-key
 *                  keypad. Using counters, the rows and columns are looped 
 *                  through, powering each row using combinatorial logic 
 *                  and checking if each column is subsequently powered to 
 *                  determine keypresses. First key pressed is key read. 
 *                  Keypad is represented as 6-bits, with the first 3 bits 
 *                  representing the column chosen, and the second 3 bits 
 *                  representing the row chosen. Keypad 000_000 was chosen 
 *                  to represent no current keypresses, which made handling
 *                  consecutive keypresses not too difficult.
 */
module keypadScanner(input logic clk, 
                      input logic [3:0]col,
                      output logic [3:0]row,
                      output logic [5:0]keypad);
   logic [4:0]scanCount = 5'd1;
   logic keypadTriggered = 1'd0;
   logic [2:0]currentRow = 2'd0;
   logic [2:0]currentCol = 2'd0;
    
    // State logic for powering the switch rows
    always_comb
       case(currentRow)
           3'b000 : row = 4'b1110;
           3'b001 : row = 4'b1101;
           3'b010 : row = 4'b1011;
           3'b011 : row = 4'b0111;
           default : row = 4'b1111;
       endcase

   always_ff@(posedge clk)
   begin
        scanCount <= scanCount + 1'b1;

        if (currentCol == 3'b011)
        begin
            currentRow <= currentRow + 1'b1;
            currentCol <= 3'd0;
        end
        else
            currentCol <= currentCol + 1'b1;
         
        if (col[currentCol] == '0)
        begin
            keypadTriggered <= 1'd1;
            keypad <= {currentRow + 1'b1, currentCol + 1'b1};
        end

        if (scanCount[4])
        begin
            if (!keypadTriggered)
                keypad <= 6'd0;

            // Reset row and column counters, scanCount, and keypadTriggered to get ready for
            // another scan pass
            scanCount <= 5'd0;
            keypadTriggered <= 1'd0;
            currentRow <= 3'd0;
            currentCol <= 3'd0;
        end
    end
endmodule



/* Name & Email: Skyler Williams (szwilliams@hmc.edu)
 * Date of Creation: 11/21/2014
 * 
 * Module Summary: Combinatorial logic to get the correct 4-bit number output 
 *                  for the 6-bit representation of the keypad.
 */
module keypressCheck(input logic [5:0]keypad,
                    output logic keyPressed);
always_comb
    if (keypad == 6'b000_000)
        keyPressed = 1'b0;
    else 
        keyPressed = 1'b1;

endmodule



/* Name & Email: Skyler Williams (szwilliams@hmc.edu)
 * Date of Creation: 09/25/2014, modified 11/21/2014
 * 
 * Module Summary: Debounces incoming signal representing a keypress on 16-key
 *                  keypad, and stores the last mode and control keys that have 
 *                  been fully read/debounced. The mode key is the most significant
 *                  3-bits of keypadInput, and the control key is the least 
 *                  significant 3-bits of keypadInput.
 */
module debouncer(input logic clk,
                  input logic keyPressed,
                  input logic [5:0]keypad,
                  output logic [2:0]mode_key,
                  output logic [2:0]ctrl_key,
                  output logic [2:0]row_key);
// Testing counter at 60Hz value, seems to work well
logic [17:0]count = 18'd0;
logic memoryChange = 1'd1;
logic is_mode = 1'd0;

// If the bottom row is powered, then it's a mode
always_comb
    if (keypad[5])
        is_mode = 1;
    else
        is_mode = 0;

always_ff@(posedge clk)
begin    
    if (keyPressed)
    begin
        if (count[16] & count[17])
        begin
            if (memoryChange) // Deals with not re-reading held key
            begin
                if (is_mode)
                begin
                    mode_key <= keypad[2:0]; // Still first three bits
                end
                else
                    ctrl_key <= keypad[2:0];
                    row_key <= keypad[5:3];
                memoryChange <= 1'b0;
            end
        end
        else
            count <= count + 1'b1;
    end
    else
    begin
        count <= '0;
        memoryChange = 1'b1;
    end
end

endmodule



/* Name & Email: Skyler Williams (szwilliams@hmc.edu)
 * Date of Creation: 11/21/2014
 * 
 * Module Summary: Takes action based on the mode_key selected and the
 *                  ctrl_key selected, when keyPressed.
 *                  
 */
module keypadAction(input logic keyPressed,
                  input logic [2:0]mode_key,
                  input logic [2:0]ctrl_key,
                  input logic [2:0]row_key,
                  output logic [1:0]waveform_select,
                  output logic [1:0]filter_select,
                  output logic [2:0]filter_params,
                  output logic [7:0]amp_envelope);
                        
logic [1:0]ctrlMinusOne;
assign ctrlMinusOne = ctrl_key - 1;

always_comb
    if (keyPressed) // If keyPressed, then act on it according to selected mode
    begin
        case(mode_key)
            3'b001 : begin // Waveform Select Mode
                case(ctrl_key)
                    3'b001 : begin // Sine wave
                        waveform_select = 2'b00;
                        // Power correct LEDs
                        end
                    3'b010 : begin // Sawtooth wave
                        waveform_select = 2'b01;
                        // Power correct LEDs
                        end
                    3'b011 : begin // Triangle wave
                        waveform_select = 2'b10;
                        // Power correct LEDs
                        end
                    3'b100 : begin // Square wave
                        waveform_select = 2'b11;
                        // Power correct LEDs
                        end
                    default : waveform_select = 2'b00; // Sine wave by default
                endcase
                end
            3'b010 : begin // Filter Freqency Select Mode
                if (row_key[0] & row_key[1]) // If 3rd row, select filter type
                    filter_select = ctrl_key;
                else // Otherwise, select filter parameter
                begin
                    // Sends filter values 1-8, bottom left->top right, to filter
                    filter_params = {ctrlMinusOne, ~row_key[0]}; 
                end
                end
            3'b011 : begin // Amp ASDR Envelope Select Mode
                case(ctrl_key)
                    3'b001 : begin // Attack, first 2 bits of amp. env.
                        // Subtraction give higher values for physically higher keys
                        amp_envelope[1:0] = 3'b100 - row_key; 
                        end
                    3'b010 : begin // Decay, second 2 bits of amp. env.
                        amp_envelope[3:2] = 3'b100 - row_key;
                        end
                    3'b011 : begin // Sustain, third 2 bits of amp. env.
                        amp_envelope[5:4] = 3'b100 - row_key;
                        end
                    3'b100 : begin // Release, fourth 2 bits of amp. env.
                        amp_envelope[7:6] = 3'b100 - row_key;
                        end
                endcase
                end
            3'b100 : begin // Stored Presets Select Mode
                case(ctrl_key)
                    default : waveform_select = 2'b0; // Cases for stored presets, these will modify everything from previous cases
                endcase
                end
            default : waveform_select = 2'b00; // Should never happen
        endcase
    end

endmodule



