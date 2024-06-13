all: compile run

compile:
	g++ -o .\build\Raytracing.exe .\src\main.cpp -O3 -std=c++17

compile_debug:
	g++ -o .\build\Raytracing.exe .\src\main.cpp -g -std=c++17

run:
	.\build\Raytracing.exe > .\build\image.ppm