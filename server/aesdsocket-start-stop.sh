#!/bin/sh

case "$1" in
	start)
		echo "starting simple server"
		start-stop-daemon -S -n aesdsocket-start-stop -a /usr/bin/aesdsocket
		;;
	stop)
		echo "stopping simple server"
		start-stop-daemon -K -n aesdsocket-start-stop
		;;
	*)
		echo "Usage: $0 {start | stop}"
	exit 1
esac

exit 0