mkdir build
cd build
cmake -G "Visual Studio 15 Win64" .. 
cmake --build . --target Valkun --config Debug
cd ..
