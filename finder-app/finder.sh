#!/bin/sh

success() {
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
# echo "Directory to search: $filesdir"
# echo "Search string: $searchstr"

# count the number of files in the directory and sub-directories
file_count=$(find "$filesdir" -type f | wc -l)

# count the number of lines that contain the search string in these files
match_count=$(grep -r "$searchstr" "$filesdir" 2>/dev/null | wc -l)

# print the result
echo "The number of files are $file_count and the number of matching lines are $match_count"

success
