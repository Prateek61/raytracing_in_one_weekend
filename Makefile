all: compile run

compile:
	g++ -o .\build\Raytracing.exe .\src\main.cpp -O3 -std=c++11

compile_debug:
	g++ -o .\build\Raytracing.exe .\src\main.cpp -g

run:
	.\build\Raytracing.exe > .\build\image.ppm