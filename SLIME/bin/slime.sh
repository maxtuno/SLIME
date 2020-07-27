#!/usr/bin/env bash
mpirun -q -np 4 -hostfile hostfile ./slime "${1}"
