#!/bin/sh
# Tester script for assignment 1 and assignment 2
# Author: Siddhant Jajoo

set -e
set -u

printf "Current script location: \e[1;33m`realpath "$0"`\e[0m\n"
SCRIPTLOC=$(dirname $0)
printf "The place where this script is located: \e[1;33m${SCRIPTLOC}\e[0m\n"

NUMFILES=10
WRITESTR=AELD_IS_FUN
WRITEDIR=/tmp/aeld-data
if [ "$SCRIPTLOC" != "/usr/bin" ]; then
	username=$(cat conf/username.txt)
	assignment=`cat ../conf/assignment.txt`
else
	username=$(cat /etc/finder-app/conf/username.txt)
	assignment=`cat /etc/finder-app/conf/assignment.txt`
fi
printf "User name: \e[1;33m$username\e[0m\n"

if [ $# -lt 3 ]
then
	echo "Using default value ${WRITESTR} for string to write"
	if [ $# -lt 1 ]
	then
		echo "Using default value ${NUMFILES} for number of files to write"
	else
		NUMFILES=$1
	fi	
else
	NUMFILES=$1
	WRITESTR=$2
	WRITEDIR=/tmp/aeld-data/$3
fi

MATCHSTR="The number of files are ${NUMFILES} and the number of matching lines are ${NUMFILES}"

echo "Writing ${NUMFILES} files containing string ${WRITESTR} to ${WRITEDIR}"

rm -rf "${WRITEDIR}"

# create $WRITEDIR if not assignment1
# assignment=`cat ../conf/assignment.txt`
# assignment=`cat /etc/finder-app/conf/assignment.txt`

if [ $assignment != 'assignment1' ]
then
	mkdir -p "$WRITEDIR"

	#The WRITEDIR is in quotes because if the directory path consists of spaces, then variable substitution will consider it as multiple argument.
	#The quotes signify that the entire string in WRITEDIR is a single string.
	#This issue can also be resolved by using double square brackets i.e [[ ]] instead of using quotes.
	if [ -d "$WRITEDIR" ]
	then
		echo "$WRITEDIR created"
	else
		exit 1
	fi
fi
#echo "Removing the old writer utility and compiling as a native application"
if [ "$SCRIPTLOC" != "/usr/bin" ]; then
	make clean
	make
fi
# make clean
# make
# make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- all

for i in $( seq 1 $NUMFILES)
do
	# for assignment 1
	#./writer.sh "$WRITEDIR/${username}$i.txt" "$WRITESTR"
	if [ "$SCRIPTLOC" != "/usr/bin" ]; then
		# for assignment 2
		./writer "$WRITEDIR/${username}$i.txt" "$WRITESTR"
	else
		# for assignment 4
		writer "$WRITEDIR/${username}$i.txt" "$WRITESTR"
	fi
done

if [ "$SCRIPTLOC" != "/usr/bin" ]; then
	OUTPUTSTRING=$(./finder.sh "$WRITEDIR" "$WRITESTR")
else
	OUTPUTSTRING=$(finder.sh "$WRITEDIR" "$WRITESTR" | tee /tmp/assignment4-result.txt)
fi

# remove temporary directories
rm -rf /tmp/aeld-data

## add output of file utility after building "writer" with CROSS_COMPILE
#make clean
#make CROSS_COMPILE=aarch64-none-linux-gnu-
#file writer > ../assignments/assignment2/fileresult.txt
#make clean

set +e
echo ${OUTPUTSTRING} | grep "${MATCHSTR}"
if [ $? -eq 0 ]; then
	echo "success"
	exit 0
else
	echo "failed: expected  ${MATCHSTR} in ${OUTPUTSTRING} but instead found"
	exit 1
fi
