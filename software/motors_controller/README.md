# motors controller

'motors_controller' manage the following features in a separate arduino Pro Mini board :
- analog input to measure motors current
- digital output to command motor drivers

Because it's not possible to manage everything simultaneously on ESP32-CAM board :
- wifi + analog input
- serial + digital pins (6 pins required to command motors)

Main panel_controller board (ESP32-CAM) communicates with this motors_controller via I2C with a simple specific protocol consisting of sending a simple string with text commands :

## Commands syntax

Several commands can be sent together : `<command1>;<command2>;<command3>`

They will be buffered on arduino side and executed sequentially after the 'run' command is received.

All commands are in the same form : `<command_id>:<arg1>[,<arg2[,<arg3>]]`

## Measure sampling ('ms')
`ms:<period_ms>,<samples_count>`

## Print measure buffer ('m')
`m:<samples_count>`



