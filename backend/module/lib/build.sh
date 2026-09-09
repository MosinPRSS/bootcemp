clear

printf "******************************************\n"
printf "stas (copyright) 2026\n"
printf "make sure that you installed gcc libs\n"
printf "because i am lazy to make checks in script\n"
printf "******************************************\n"

printf "creating object file for library...\n"
g++ -c -fPIC BootCamp.cpp -o BootCampObj.o
printf "done...\n"

printf "creating shared library...\n"
g++ -shared BootCampObj.o -o MathLib.so
printf "done! exiting\n"