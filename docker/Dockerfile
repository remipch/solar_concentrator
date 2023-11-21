FROM ubuntu:22.04

# Prevent following package to ask questions during installation
ARG DEBIAN_FRONTEND=noninteractive

# Install esp-idf dependencies
RUN apt-get update && apt-get install -y git
RUN apt-get update && apt-get install -y wget
RUN apt-get update && apt-get install -y flex
RUN apt-get update && apt-get install -y bison
RUN apt-get update && apt-get install -y gperf
RUN apt-get update && apt-get install -y python3
RUN apt-get update && apt-get install -y python3-pip
RUN apt-get update && apt-get install -y python3-venv
RUN apt-get update && apt-get install -y cmake
RUN apt-get update && apt-get install -y ninja-build
RUN apt-get update && apt-get install -y ccache
RUN apt-get update && apt-get install -y libffi-dev
RUN apt-get update && apt-get install -y libssl-dev
RUN apt-get update && apt-get install -y dfu-util
RUN apt-get update && apt-get install -y libusb-1.0-0

# Clone esp-idf
RUN mkdir "/esp"
WORKDIR "/esp"
RUN git clone --recursive --branch release/v5.0  https://github.com/espressif/esp-idf.git

# Install esp-idf
WORKDIR "/esp/esp-idf"
ENV IDF_TOOLS_PATH=/esp/tools
# ENV IDF_TOOLS_PATH=/esp/tools
RUN ./install.sh esp32

## Initialize idf framework for each bash session
## so we can enter in an existing container with command : sudo docker exec -i -t solar_concentrator-dev-1 /bin/bash
## and use idf stuff directly (which is the main goal of this docker image)
RUN echo ". /esp/esp-idf/export.sh" >> ~/.bashrc

# Install useful tools
RUN apt-get update && apt-get install -y nano                 # minimal text editor
RUN apt-get update && apt-get install -y cu                   # serial communication
RUN apt-get update && apt-get install -y iputils-ping         # ping
RUN apt-get update && apt-get install -y arduino              # for motors_controller arduino script
RUN apt-get update && apt-get install -y libjpeg-dev          # to build quirc in container (not necessary for ESP32)
RUN apt-get update && apt-get install -y libpng-dev           # to build quirc in container (not necessary for ESP32)
RUN apt-get update && apt-get install -y gdb
RUN apt-get update && apt-get install -y clangd
RUN apt-get update && apt-get install -y clang

# Install useful python tools
RUN apt-get update && apt-get install -y python3-tk # python tk (matplotlib backend)
RUN apt-get update && apt-get install -y python3-pip          # pip (used just below)
RUN pip install --upgrade pip
RUN pip install opencv-python
RUN pip install matplotlib
RUN pip install getch # for manual python script using keyboard

# Add kubuntu-ppa/backports to get not-so-old versions of kde applications
RUN apt-get update && apt-get install -y software-properties-common
RUN add-apt-repository -y ppa:kubuntu-ppa/backports

# Install useful kde applications from kubuntu backports
# RUN apt-get update && apt-get install -y libpulse-dev           # required by last version of kate
RUN apt-get update && apt-get install -y kate
# RUN apt-get update && apt-get install -y kdevelop
RUN apt-get update && apt-get install -y breeze-icon-theme    # used by kde apps but not managed as an apt dependency
RUN apt-get update && apt-get install -y konsole              # used by kdevelop integrated konsole tool
# RUN apt-get update && apt-get install -y kdevelop-python      # python plugin

# Install useful coding tools usable from kate
RUN apt-get update && apt-get install -y jq                   # json processor
RUN apt-get update && apt-get install -y clang-format
RUN pip install cmake-format

# avahi needs dbus daemon, both service are correctly listed by 'ls /etc/rc*' command
# but they don't start correctly at startup
# FORCE start for each session (ugly hack.. I don't know how to solve it cleanly) :
# But it blocks dbus in the host and prevent 'systemctl suspend' simple action to work
# RUN echo "service dbus start" >> ~/.bashrc
# RUN echo "service avahi-daemon start" >> ~/.bashrc

# /!\ WARNING /!\
# Error if we use opencv-python and PyQt5 in the same python script :
# qt.qpa.plugin: Could not load the Qt platform plugin "xcb" in "/opt/esp/python_env/idf4.4_py3.8_env/lib/python3.8/site-packages/cv2/qt/plugins" even though it was found.
# I didn't manage to solve the problem
# For now, Qt5 is not required for this project
# RUN apt-get update && pip install PyQt5     # ATTENTION : license GPL (contaminante)

# force read/write permissions to individual configs (so it can be read/write from any user in host)
# RUN mkdir /root/.config
# RUN echo "touch /root/.config/kdeveloprc" >> ~/.bashrc
# RUN echo "chmod 777 /root/.config/kdeveloprc" >> ~/.bashrc

# Install clang tools specific to esp32
# TODO move with esp idf commands (but it will rebuild everything after)
RUN /esp/esp-idf/tools/idf_tools.py install xtensa-clang

# Add usefull aliases
RUN echo "alias h='cat /solar_concentrator/docker/docker_help.txt && alias'" >> ~/.bashrc
RUN echo "alias cdtra='cd /solar_concentrator/software/remote_tracking'" >> ~/.bashrc
RUN echo "alias cdsup='cd /solar_concentrator/software/supervisor_controller/build_esp32/'" >> ~/.bashrc
RUN echo "alias cdmot='cd /solar_concentrator/software/motors_controller/'" >> ~/.bashrc

# Finally print help
RUN echo "h" >> ~/.bashrc
