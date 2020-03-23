cd SLIME/SLIME
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
cd ..
chmod 755 bin/slime
cd ..
chmod 755 mpi-run.sh