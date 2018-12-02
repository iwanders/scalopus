

Retry the cmakelists:
```bash
build/scalopus_transport$ rm -rf * ; CXX=/usr/bin/clang++-7 cmake ../../repo/scalopus_transport/
build/scalopus_lttng$ rm -rf * ; CXX=/usr/bin/clang++-7 cmake ../../repo/scalopus_lttng/ -DScalopusTransport_DIR=$(pwd)/../scalopus_transport/
build/scalopus_examples$ rm -rf * ; CXX=/usr/bin/clang++-7 cmake ../../repo/scalopus_examples/ -DScalopusTransport_DIR=$(pwd)/../scalopus_transport/ -DScalopusLttng_DIR=$(pwd)/../scalopus_lttng/ && make
```
