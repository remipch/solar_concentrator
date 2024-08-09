# Solar Concentrator Supervisor Electronics

This folder contains the custom board designed for the supervisor.

It has been designed with [LibrePCB](https://librepcb.org) and produced by [AISLER](https://aisler.net).

It doesn't do anything complicated, just glue things together :
* Allow to plug the ESP32-CAM board
* Allow to plug the Arduino Pro Mini board
* Include connectors for 2 panel boards
* Include a voltage regulator to convert the input 12V battery to 5V
* Include some switchs to power ON/OFF the child boards
* Include a FTDI socket to upload the ESP32-CAM firmware inplace (Arduino has its own FTDI socket)
* Include a minimal voltage converter for the child boards to communicate together

## Schematics

![Supervisor schematics](supervisor_schematics.png)

## Board

### Components

![Supervisor board](supervisor_board_components.png)

### Top layer

![Supervisor board](supervisor_board_top_layer.png)

### Bottom layer

![Supervisor board](supervisor_board_bottom_layer.png)

### 3D model

![Supervisor board](supervisor_board_3d_model.png)

### Photo

![Supervisor board](supervisor_board_photo.jpg)

## License

TODO
