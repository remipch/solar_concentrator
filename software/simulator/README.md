# Solar Concentrator Simulator

Before any hardware construction, a simulator was developed
using [Panda3D](https://www.panda3d.org/), a Python game engine.

This simulator can simulate between 1 and 10 panels of different sizes,
immersed in an environment including some background elements.

Its simple user interface allows to configure:
* the global position on the planet
* the date and time
* the size and position of the background elements
* the number, size and position of the panels in the grid

From such a given configuration, it simulates the reflection of solar rays
(taking into account the shadows) and estimates the corresponding power received on the target area.

From the provided docker container, the simulator can be started with the following command:

``` bash
cd /solar_concentrator/software/simulator

python3 solar_concentrator_simulator.py
```

A remote control allows to send configuration values and get estimated output values
from an external Python program. Any parameter that can be set manually can also
be set remotely with this `remote_control`.

This is used for two purposes :
* `end_to_end_test.py` starts the simulator, sets some configuration values and checks
if the estimated power value is as expected
* `time_walker_recorder.py` uses the current simulator configuration and estimates
the received solar power for a given time range
