#!/usr/bin/env bash
mpirun -q -np 8 -hostfile hostfile --allow-run-as-root ./slime_cloud "${1}"
pkill slime_cloud
