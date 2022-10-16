#!/usr/bin/env bash
mpirun -q -np 4 -hostfile hostfile --allow-run-as-root ./slime "${1}"
pkill slime
