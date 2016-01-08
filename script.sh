#!/bin/bash

echo "################## Cleaning & Building";
ino clean
ino build

read -p "################## Upload? [y/n]?" CONT
if [ "$CONT" == "y" ]; then
  echo "################## Uploading";
  ino upload -p /dev/tty.SLAB_USBtoUART;
else
  echo "################## skipping upload";
  exit 0;
fi

read -p "################## Open serial terminal? [y/n]?" CONT
if [ "$CONT" == "y" ]; then
  echo "################## Opening picocom";
  echo "################## You can exit with CTRL+A followed by CTRL+Q";
  ino serial -p /dev/tty.SLAB_USBtoUART;
else
  echo "################## skipping serial terminal";
fi


exit 0
