cd SLIME
rm -fr build
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
cd ..
chmod 755 bin/slime
chmod 755 mpi-run.sh