#!/bin/bash

if [ -f "nginx" ]; then
    rm -rf nginx
fi

if [ ! -d "debug" ]; then
  mkdir debug;
fi

if [ ! -d "lib" ]; then
  mkdir lib;
fi

cd debug;
rm -rf *;
cd ..

cd lib
rm -rf *;
cd ..

cd debug
echo "************first Compile***************\n"
cmake -DCMAKE_BUILD_TYPE=Debug ..;
make -j8;

rm -rf *
clear
echo "************second Compile***************\n"
cmake -DCMAKE_BUILD_TYPE=Debug ..;
make -j8;

cd ..
rm -rf debug

if [ ! -d "logs" ]; then
  mkdir logs;
fi

cd logs;
rm -rf *
touch error.log