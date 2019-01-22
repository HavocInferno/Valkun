mkdir buildRelease
cd buildRelease
cmake -G "Visual Studio 15 Win64" .. 
cmake --build . --target Valkun --config Release
cd ..


