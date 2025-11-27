#!/bin/bash

id=$(ps -ef | grep '[w]aybar -c' | grep power | awk '{print $2}')
kill -SIGUSR1 $id
