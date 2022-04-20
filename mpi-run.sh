
#!/bin/bash

mpirun --mca btl_tcp_if_include eth0 --allow-run-as-root -np 2 --hostfile $1 --use-hwthread-cpus --map-by node:PE=2 --bind-to none --report-bindings slime $2
