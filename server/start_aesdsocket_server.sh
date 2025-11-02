# !/bin/bash

YELLOW='\033[1;33m'
NC='\033[0m' # No Color

printf "Killing any existing ${YELLOW}aesdsocket${NC} instances...\n"
ps aux | grep "./aesdsocket -d" | grep -v grep | awk '{print $2}' | xargs kill
make clean && make
printf "\n${YELLOW}Starting aesdsocket in daemon mode...${NC}\n\n"
./aesdsocket -d
bash ../assignment-autotest/test/assignment8/sockettest.sh
