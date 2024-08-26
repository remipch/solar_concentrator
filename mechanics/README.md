# Solar Concentrator Mechanics :gear:

This folder contains the 3D models of the parts used in the solar panel.

- [Overview](#overview)
- [Solar panel](#solar-panel)
    - [Stand](#stand)
        - [Stand back board](#stand-back-board)
            - [Motor block](#motor-block)
        - [Stand front board](#stand-front-board)
    - [Panel frame](#panel-frame)
        - [Panel horizontal axis](#panel-horizontal-axis)
        - [Panel vertical axis](#panel-vertical-axis)
    - [Panel board](#panel-board)
- [License](#license)

## Overview

![Solar panel animation](images/solar_panel.gif)

The originality of the mechanical design is to use a simple [cablebot](https://en.wikipedia.org/wiki/Cable_robots) instead
of a traditional 2 axis tracker where each axis has a dedicated motor.
* Advantage: cheaper motors and cheaper mechanical structure can be used.
* Drawback: each axis cannot be controlled independently.

Another originality is the way to precisely set mirror orientation without
requiring high precision mechanics.

Mechanical parts have been modelled with [OpenSCAD](https://openscad.org/).

> [!TIP]
> You can click on the images below to view them in an online 3D viewer.

## Solar panel

> [!NOTE]
> The cable is not modelled because I didn't find a simple way to do it with OpenSCAD.
>
> Each cable is actually wound around the motor axis, then passes through the pulley
> and is tied to a corner of the rotating panel.

| Assembled | Exploded |
| --------- | -------- |
| [![Solar panel](images/solar_panel_assembled.png)](https://remipch.github.io/test_website/view_3d.html?model=temp) | [![Solar panel](images/solar_panel_exploded.png)](https://remipch.github.io/test_website/view_3d.html?model=temp)

### Stand

| Assembled | Exploded |
| --------- | -------- |
| [![Stand](images/stand_assembled.png)](https://remipch.github.io/test_website/view_3d.html?model=temp) | [![Stand](images/stand_exploded.png)](https://remipch.github.io/test_website/view_3d.html?model=temp)

#### Stand back board

| Assembled | Exploded |
| --------- | -------- |
| [![Stand back board](images/stand_back_board_assembled.png)](https://remipch.github.io/test_website/view_3d.html?model=temp) | [![Stand back board](images/stand_back_board_exploded.png)](https://remipch.github.io/test_website/view_3d.html?model=temp)

##### Motor block

| Assembled | Exploded |
| --------- | -------- |
| [![Motor block](images/motor_block_assembled.png)](https://remipch.github.io/test_website/view_3d.html?model=temp) | [![Motor block](images/motor_block_exploded.png)](https://remipch.github.io/test_website/view_3d.html?model=temp)

#### Stand front board

| Assembled | Exploded |
| --------- | -------- |
| [![Stand front board](images/stand_front_board_assembled.png)](https://remipch.github.io/test_website/view_3d.html?model=temp) | [![Stand front board](images/stand_front_board_exploded.png)](https://remipch.github.io/test_website/view_3d.html?model=temp)

### Panel frame

| Assembled | Exploded |
| --------- | -------- |
| [![Panel frame](images/panel_frame_assembled.png)](https://remipch.github.io/test_website/view_3d.html?model=temp) | [![Panel frame](images/panel_frame_exploded.png)](https://remipch.github.io/test_website/view_3d.html?model=temp)

#### Panel horizontal axis

| Assembled | Exploded |
| --------- | -------- |
| [![Panel horizontal axis](images/panel_horizontal_axis_assembled.png)](https://remipch.github.io/test_website/view_3d.html?model=temp) | [![Panel horizontal axis](images/panel_horizontal_axis_exploded.png)](https://remipch.github.io/test_website/view_3d.html?model=temp)

#### Panel vertical axis

| Assembled | Exploded |
| --------- | -------- |
| [![Panel vertical axis](images/panel_vertical_axis_assembled.png)](https://remipch.github.io/test_website/view_3d.html?model=temp) | [![Panel vertical axis](images/panel_vertical_axis_exploded.png)](https://remipch.github.io/test_website/view_3d.html?model=temp)

### Panel board

| Assembled | Exploded |
| --------- | -------- |
| [![Panel board](images/panel_board_assembled.png)](https://remipch.github.io/test_website/view_3d.html?model=temp) | [![Panel board](images/panel_board_exploded.png)](https://remipch.github.io/test_website/view_3d.html?model=temp)

## License

Copyright 2024 Rémi Peuchot

This folder is distributed under [Creative Commons Attribution-NonCommercial-ShareAlike 4.0](mechanics/LICENSE.md)
