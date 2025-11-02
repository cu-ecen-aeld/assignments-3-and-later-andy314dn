#!/bin/sh

BLUE='\033[1;34m'
NC='\033[0m' # No Color

make clean && make
sudo ./aesdchar_unload
printf "\n${BLUE}Re-loading the driver now...${NC}\n\n"
sudo ./aesdchar_load
printf "\n${BLUE}Running drivertest.sh to verify functionality after reload...${NC}\n\n"
sudo ../assignment-autotest/test/assignment8/drivertest.sh
printf "\n${BLUE}Test complete.${NC}\n"
