cmake -S . -B build-debug -D ENABLE_TEST=ON && cmake --build build-debug --target tests && ./build-debug/tests
