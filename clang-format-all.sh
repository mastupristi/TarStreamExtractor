#!/bin/bash

find . -type d \( -path ./ThirdParties -o -path ./bsp/board -o -path ./bsp/device -o -path ./drivers/nxpDrivers \) -prune -false -o -regex '.*\.\(cpp\|hpp\|cu\|c\|h\)' -exec clang-format -style=file -i {} \;
