#!/bin/bash -e

gcc -o logger -g logger.c test_logger.c network.c -lpthread


