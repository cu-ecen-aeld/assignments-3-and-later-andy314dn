# !/bin/bash
make clean && make
printf "\nStarting aesdsocket in daemon mode...\n\n"
./aesdsocket -d
