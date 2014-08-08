#!/bin/sh
# This script is to announce a message on a server automatically
# and repeatedly.  It is intended for server admin use.
# Here is an example:
#
# announce.sh localhost account password 30 "Attention: Here is an
#   announcement\nAnd here is another announcement"
#
# The bnchat program can be obtained from the bnetd package.

BNCHAT=bnchat
PIPE="/tmp/pipe-bnannounce-$$"


cleanup () {
        kill -9 "${pid}" 2> /dev/null
        rm -f "${PIPE}" 2> /dev/null
        exit 0
}


if [ -z "$4" ]; then
        echo -e "Usage: $0 server account password interval [msgs] ..."
        echo -e "   server    server ip or hostname"
        echo -e "   account   your server account"
        echo -e "   password  password for your account"
        echo -e "   interval  time intervals between announce in seconds"
        echo -e "   [msgs]    messages you want to announce"
        echo
        echo -e "Notes: Your account should have announce or admin permissions"
        echo -e "       If interval is zero then bnannounce will only print"
        echo -e "       one copy of the announcement."
        echo
        exit
fi

rm -f "${PIPE}"
mknod "${PIPE}" p > /dev/null
if [ $? -ne 0 ] ; then
        echo "$0: failed to make pipe file ${PIPE}, check your permissions." >&2
        exit 1
fi

server="$1"
user="$2"
pass="$3"
interval="$4"
shift 4
msg="`echo -e "$*" | sed -e 's/^/\/announce /g'`"

"${BNCHAT}" < "${PIPE}" > /dev/null 2>&1 &
pid="$!"
trap "eval cleanup" SIGINT SIGQUIT SIGTERM EXIT

echo -e "${user}" > "${PIPE}"
echo -e "${pass}" > "${PIPE}"
echo "/join Support" > "${PIPE}"

while kill -0 "${pid}" 2> /dev/null; do
        echo "/announce ${msg}" > "${PIPE}"
        if [ "${interval}" -lt "1" ]; then
            exit
        fi
        sleep "${interval}"
done

exit
