

Retry the cmakelists:
```bash
# build seasocks:
rm -rf * && cmake ../../3rd_party/seasocks/ -DEFLATE_SUPPORT=off && make 
# build scalopus:
rm -rf * && nlohmann_json_DIR=$(pwd)/../json/ Seasocks_DIR=$(pwd)/../seasocks/ cmake ../../repo/  && make -j8
```
