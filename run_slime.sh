#!/bin/bash

time mpirun -quiet --mca btl_tcp_if_include eth0 --allow-run-as-root -np 4 --hostfile $1 --use-hwthread-cpus slime $2
