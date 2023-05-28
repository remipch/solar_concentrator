FROM espressif/idf:release-v4.4
ARG DEBIAN_FRONTEND=noninteractive # to prevent installer from asking questions
RUN apt-get update && apt-get install -y kdevelop
RUN apt-get update && apt-get install -y breeze-icon-theme  # used by kdevelop but not managed as an apt dependency
RUN apt-get update && apt-get install -y konsole            # used by kdevelop integrated konsole tool
RUN apt-get update && apt-get install -y cu                 # serial communication
RUN apt-get update && apt-get install -y iputils-ping       # ping
RUN apt-get update && apt-get install -y kdevelop-python    # python plugin
RUN apt-get update && apt-get install -y python3-tk         # python tk (matplotlib backend)

RUN apt-get update && apt-get install -y arduino

# Note : full pip path must be sepcified here otherwise pip package seems to be installed for anotehr python version
RUN /opt/esp/python_env/idf4.4_py3.8_env/bin/pip install --upgrade pip
RUN /opt/esp/python_env/idf4.4_py3.8_env/bin/pip install opencv-python
RUN /opt/esp/python_env/idf4.4_py3.8_env/bin/pip install matplotlib
RUN /opt/esp/python_env/idf4.4_py3.8_env/bin/pip install getch # for manual python script using keyboard

RUN apt-get update && apt-get install -y mc                 # console file explorer
RUN apt-get update && apt-get install -y nano               # minimal text editor

# Not necessary (avahi-tools is installed by default) :
# RUN apt-get update && apt-get install -y avahi-discover

# avahi needs dbus daemon, both service are correctly listed by 'ls /etc/rc*' command
# but they don't start correctly at startup
# FORCE start for each session (ugly hack.. I don't know how to solve it cleanly) :
RUN echo "service dbus start" >> ~/.bashrc
RUN echo "service avahi-daemon start" >> ~/.bashrc

# /!\ WARNING /!\
# Error if we use opencv-python and PyQt5 in the same python script :
# qt.qpa.plugin: Could not load the Qt platform plugin "xcb" in "/opt/esp/python_env/idf4.4_py3.8_env/lib/python3.8/site-packages/cv2/qt/plugins" even though it was found.
# I didn't manage to solve the problem
# For now, Qt5 is not required for this project
# RUN apt-get update && pip install PyQt5     # ATTENTION : license GPL (contaminante)

# systematically initialize idf framework for each bash session
# so we can enter in an existing container with command : sudo docker exec -i -t solar_concentrator-dev-1 /bin/bash
# and use idf stuff directly (which is the main goal of this docker image)
RUN echo ". $IDF_PATH/export.sh" >> ~/.bashrc

# force read/write permissions to individual configs (so it can be read/write from any user in host)
# RUN mkdir /root/.config
# RUN echo "touch /root/.config/kdeveloprc" >> ~/.bashrc
# RUN echo "chmod 777 /root/.config/kdeveloprc" >> ~/.bashrc

# Print useful project help from container
RUN echo "cat /solar_concentrator/docker/docker_help.txt" >> ~/.bashrc

