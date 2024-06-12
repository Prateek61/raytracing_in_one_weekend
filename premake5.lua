workspace "Raytracing"
    configurations { "Debug", "Release" }
    platforms { "x64" }
    startproject "Raytracing"
    location "build"

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

project "Raytracing"
    kind "ConsoleApp"
    language "C++"
    targetdir "build"
    objdir "build/int"

    files { "src/**.h", "src/**.cpp" }