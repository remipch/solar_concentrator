FROM espressif/idf:release-v4.4
ARG DEBIAN_FRONTEND=noninteractive # to prevent installer from asking questions
RUN apt-get update && apt-get install -y minicom
RUN apt-get update && apt-get install -y kate
RUN apt-get update && apt-get install -y kdevelop
RUN apt-get update && apt-get install -y breeze-icon-theme  # used by kdevelop but not managed as an apt dependency
RUN apt-get update && apt-get install -y konsole
RUN apt-get update && apt-get install -y mc       # console file explorer
RUN apt-get update && apt-get install -y iputils-ping

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

