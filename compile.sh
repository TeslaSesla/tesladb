#!/bin/bash

echo "---TeslaDB library---"
echo "Compile this library only if you want to get example how it work."
echo "If you want something else - check 'doc' directory"
echo ""

read -n 1 -p "Are you sure that you want to compile it? (y/[a]): " AMSURE

if [ $AMSURE == "y" ]
then
    echo ""
    echo "Compiling..."
    g++ main.cpp database.cpp database.h -std=c++17 -lstdc++fs -pthread -o tesladbExample
else
    echo ""
fi
    exit 0
