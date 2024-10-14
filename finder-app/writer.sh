#!/bin/bash

# handle success exit
success() {
    exit 0
}

# handle failure exit
failure() {
    exit 1
}

# check if two arguments are provided
if [ "$#" -ne 2 ]; then
    echo "Need two arguments: writefile writestr"
    failure
fi

# assign arguments to variables
writefile="$1"
writestr="$2"

# create the directory path if it does not exist
dirpath=$(dirname "$writefile")
if ! mkdir -p "$dirpath"; then
    echo "Error: Could not create directory path '$dirpath'."
    failure
fi

# write the content to the file, overwriting if it already exist
if ! echo "$writestr" > "$writefile"; then
    echo "Error: Could not write to file '$writefile'."
    failure
fi

success
