#!/bin/bash
echo "Compiling Dual binaries..."
g++ -shared -fPIC -o build/libDual.so src/Dual.cpp -I$JAVA_HOME/include -I$JAVA_HOME/include/linux -I/usr/local/lib/essentia -I/usr/local/include/include-java-lib -I/usr/local/lib/eigen-3.4.0 -lessentia -lfftw3 -lyaml -lavcodec -lavformat -lavutil -lswresample -ltag \
    && echo "Compiled successfully!" \
    && sudo cp build/** /usr/local/lib/ \
    && echo "Binaries placed on java env"
