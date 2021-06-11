#FROM nvidia/opengl:1.2-glvnd-devel-ubuntu18.04
#FROM nvidia/cuda:11.0-devel-ubuntu18.04
FROM nvidia/cudagl:10.0-runtime-ubuntu18.04
ENV DEBIAN_FRONTEND="noninteractive"
RUN  apt-get update -y  && apt-get -y install wget && apt-get install python3 -y && apt-get install python3-venv -y && apt-get install python3-pip -y && apt-get install python3-opencv -y && apt-get install p7zip-full -y && apt-get install pigz && apt-get install cmake build-essential git libegl1-mesa-dev libglew-dev pkg-config libwayland-dev libxkbcommon-dev wayland-protocols libeigen3-dev libjpeg-dev libpng-dev -y
ENV NVIDIA_DRIVER_CAPABILITIES=all
