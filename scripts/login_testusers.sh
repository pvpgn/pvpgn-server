#!/bin/sh

# tries to login with a massive number of test accounts

# See make_testusers.sh for a way to create these accounts.


# number of accounts to connect with
numaccts=400

# "prefix" of account names
name="bob"

# account password
pass="bob"

# delay between printing messages
delay=30

# where to connect
server=localhost

# number of zero-padded columns in suffix
padding=6

# how many seconds until logout
total=600


num=0
while [ "${num}" -lt "${numaccts}" ]; do
    num="`expr \"${num}\" '+' '1'`"
    form="`printf \"%0${padding}d\" \"${num}\"`"
    (
        echo ''
        echo "${name}${form}"
        echo "${pass}"
        total=0;
        while [ "${total}" -lt "${maxtime}" ]; do
            sleep "${delay}"
            echo "My name is ${name}${form}"
            total="`expr \"${total}\" '+' \"${delay}\"`"
        done
        echo "/quit"
    ) | telnet "${server}" 6112 &
done

exit 0
