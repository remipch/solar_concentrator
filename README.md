# Solar Concentrator :sunny:

This project is a homemade automated solar concentrator :wrench: :sunny: :mag_right:

It consists of:
* An orientable mirror panel (currently 48 focusing mirrors, totalling 1m²)
* A fixed target (currently a simple concrete oven with a black metal plate and a tempered glass)
* The supervisor: an electronic control device that measures the target, computes and sends commands to the motors

On 26 june 2024, when adding some aluminium foils inside the oven to improve insulation,
the oven temperature has risen to __210°C after 30 minutes__.

The video below shows the concentrator in action on 8 July 2024:
* First half of the video is the assembly (speed x10)
* Second half is the automatic sun tracking for about 7 hours (speed x600)

https://github.com/user-attachments/assets/06e3a9e1-d853-4ddd-b5d8-55cf9d6ecc89

| :warning: This project may harm people or animals in the surroundings |
|---------------------------------------------------------------------- |
| It can __cause permanent blindness, burn skin or ignite objects__ |
| Even when the concentrator is not powered, the sun remains concentrated 48 times in a single small area |
| This point of focus is constantly __moving along a trajectory that is difficult to estimate__ |
| In its current state, it __must be continuously monitored__ and the mirrors must be covered quickly in the event of an anomaly |

## Key points

* Uses only standard, cheap and available components and materials :bricks: :nut_and_bolt:
* Does not require a high precision construction, everything can be made by hand with common tools :straight_ruler: :carpentry_saw:
* Easy to assemble and disassemble :screwdriver:
* Easy to understand and modify :mag_right:
* Open Source :unlock:

Limitations of this version:
* There is no safety layer implemented (see warning above :warning:)
* An initial manual step is required to orientate the panel before starting to track the sun automatically
* Only one panel can be controlled
* Does not work on cloudy days (it is the user's responsibility to remove the clouds from the sky :smile:)

To build this project, some products have been happily "hacked" out of their original use:
* The tempered glass used in the oven comes from an [Ikea shelf](https://www.ikea.com/fr/fr/p/komplement-tablette-en-verre-blanc-80257647/)
* The mirrors are [decorative bathroom mirrors](https://www.bricoman.fr/lot-6-miroirs-adhesif-carre-15x15-cm-1429043.html)
* The motors are [model engines](https://www.gotronic.fr/art-motoreducteur-mfa-950d8101ln-11376.htm)

## Motivations

This project has been an enjoying way to create, discover and learn some new concepts and tools:
* Mechanics:
    * Invent, validate and build the original mechanical design
    * Learn [OpenSCAD](https://openscad.org/), a 3D parametric design software
    * Use it to model the mechanical parts
* Electronics:
    * Learn [LibrePCB](https://librepcb.org/), an electronic design suite
    * Use it to design a specific electronic board
* Software:
    * Learn [Panda3D](https://www.panda3d.org/), a python game engine
    * Use it to develop the simulator, allowing to simulate light rays and estimate the sun power received by the target
    * Learn [ESP32 ecosystem and tools](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/index.html)
    * Use them to implement the whole control logic in a tiny ESP32-CAM board

## Next step

The next step is to control an arbitrary number of panels to obtain several killowatts of power.

This would open up the following possibilities:
* Rapidly boiling a large volume of liquid
* Desalinate or sterilise a large volume of water
* Cook or sterilize a large volume of food
* Melting plastic, metal or glass
* Increasing photovoltaic pannel output power with [concentrated photovoltaics](https://en.wikipedia.org/wiki/Concentrator_photovoltaics)

## Technical breakdown

### Mechanics

The originality of the mechanical design is to use a simple cablebot instead
of a traditional 2 axis tracker where each axis has a dedicated motor.
* Advantage: cheaper motors and cheaper mechanical structure can be used.
* Drawback: each axis cannot be controlled independently.

Another originality is the way to precisely set mirror orientation without
requiring high precision mechanics.

### Electronics

### Software

#### Simulator

A simulator has been developped before any other development to try to evaluate
the theoretical solar power received on the target.


