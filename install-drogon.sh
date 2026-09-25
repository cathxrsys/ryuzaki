mkdir temp
cd temp

git clone --recurse-submodules https://github.com/drogonframework/drogon.git
cd drogon

cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_EXAMPLES=OFF \
    -DBUILD_TESTING=OFF

cmake --build build -j$(nproc)
sudo cmake --install build

cd ..
rm -rf temp