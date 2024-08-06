# motors controller

'motors_controller' manages the following features in a separate arduino Pro Mini board :
- analog input to measure motors current
- digital output to command motor drivers

Because it's not possible to manage everything simultaneously on ESP32-CAM board :
- wifi + analog input
- serial + digital pins (6 pins required to command motors)

The main supervisor_controller board (ESP32-CAM) communicates with this motors_controller
via I2C with a simple specific protocol consisting of sending a simple string with text commands.

## Commands syntax

Several commands can be sent together : `<command1>;<command2>;<command3>`

They will be buffered on arduino side and executed sequentially after the 'run' command is received.

All commands are in the same form : `<command_id>:<arg1>[,<arg2[,<arg3>]]`

* Stop and clear command : `c`

* Print state immediately : `s`

* Print all buffered commands immediately : `p`

* Print measure buffer immediately `m:<samples_count>`

* Set measure sampling : `ms:<period_ms>,<samples_count>`

* Set output level : `l:<output_level>` with :
    - `output_level` (between 0 and 255): value written to PWM_PIN to allow output voltage customization.

* Set motors output : `o:<motor_pins>,<max_time_ms>,<threshold>` with :
    - `motors_pin`: the integer value of all output pins
    - `max_time_ms`: max time before stopping motors
    - `threshold`: measured current threshold (0 to 1023) where motor must be stopped
