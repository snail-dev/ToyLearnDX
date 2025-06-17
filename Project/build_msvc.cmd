
cmake -S . -B build
cd build
del /S/Q *.cso
cmake --build . --config Debug -- -m
cd ..