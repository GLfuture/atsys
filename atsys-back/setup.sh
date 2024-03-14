#!/bin/bash

if command -v docker &> /dev/null; then
    echo "docker has installed"
else 
    echo "running 'sudo apt install -y docker.io'"
    sudo apt install -y docker.io
fi

sudo gpasswd -a $USER docker
sudo newgrp docker

echo "start download image......"
docker pull gongzhijie/face-service:v3
echo "completely download"
if [ -d "know" ]; then
    echo "directory 'know' exists in current directory"
else
    echo "create drectory 'know' in this directory"
fi

docker run -itd --name face --network host --restart always -v ${PWD}/know:/app/know gongzhijie/face-service:v3
echo "start download image......"
docker pull gongzhijie/attendance:latest
echo "completely download"
docker run -itd --name atsys --network host --restart always gongzhijie/attendance:latest
