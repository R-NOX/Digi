#!/bin/sh
### BEGIN INIT INFO
# Provides:          data-handler
# Required-Start:
# Required-Stop:
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Description:       Enable service provided by daemon.
### END INIT INFO

name="data-handler"
cmd="/usr/bin/$name"

get_pid() {
  ps | grep -v grep | grep $cmd | awk '{print $1;}'
}

is_running() {
  ps | grep -v grep | grep $cmd > /dev/null 2>&1
}

start() {
  if is_running; then
    echo "Already started"
  else
    echo "Starting $name"
    $cmd > /dev/null 2>&1
    if ! is_running; then
      echo "Unable to start, see log"
      exit 1
    fi
  fi
}

stop() {
  if is_running; then
      echo -n "Stopping $name.."
      kill -15 `get_pid`
      for i in 1 2 3 4 5 6 7 8 9 10
      # for i in `seq 10`
      do
          if ! is_running; then
              break
          fi

          echo -n "."
          sleep 1
      done
      echo

      if is_running; then
          echo "Not stopped; may still be shutting down or shutdown may have failed"
          exit 1
      else
          echo "Stopped"
      fi
  else
      echo "Not running"
  fi
}

case "$1" in
  start)
    start
  ;;
  stop)
    stop
  ;;
  restart)
    stop
    start
  ;;
  status)
    if is_running; then
        echo "Running"
    else
        echo "Stopped"
        exit 1
    fi
  ;;
  *)
    echo "Usage: $0 {start|stop|restart|status}"
esac
