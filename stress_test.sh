#!/bin/bash

for i in {1..200}
do
  while true ;do
    echo "Spam message from client $i"
    sleep 1
  done | ./client &
done

wait
