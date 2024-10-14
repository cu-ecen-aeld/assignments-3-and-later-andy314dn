#!/bin/bash

success() {
    echo "Successfully executed!"
    exit 0
}

failure() {
    exit 1
}

# check if two arguments are provided
if [ "$#" -ne 2 ]; then
    echo "Error: Need two arguments"
    failure
fi

# assign names to each argument
filesdir="$1"
searchstr="$2"

# check if filesdir is a valid directory
if [ ! -d "$filesdir" ]; then
    echo "Error: '$filesdir' is not a valid directory"
    failure
fi

# display variable values
echo "Directory to search: $filesdir"
echo "Search string: $searchstr"

success
