# Besm ALLAH

## Install dependencies
sudo apt update
sudo apt install -y git cmake build-essential libprotobuf-dev protobuf-compiler

### ONNX
### Clone ONNX Runtime
git clone --recursive https://github.com/microsoft/onnxruntime --depth 1 --branch v1.21.0 --recursive
cd onnxruntime

### Build (this will take a while)
./build.sh --config Release --build_shared_lib --parallel $(nproc)

### Install
sudo cmake --install build/Linux/Release --prefix /usr/local

### Update cmake if needed

cmake --version

sudo apt remove cmake

Visit https://cmake.org/download/ and download the latest bash script.

    In my case cmake-3.6.2-Linux-x86_64.sh is sufficient.

Copy the script to /opt/

chmod +x /opt/cmake-3.*your_version*.sh

cd /opt

sudo bash /opt/cmake-3.*your_version*.sh
Y/Y

sudo ln -s /opt/cmake-3.*your_version*/bin/* /usr/local/bin

cmake --version

## Install espeak
sudo apt install -y git make autoconf automake libtool pkg-config gcc g++ libpcaudio-dev
    
git clone git@github.com:espeak-ng/espeak-ng.git

cd espeak-ng
~~git checkout 1.52.0~~ Need to use latest code to have espeak_TextToPhonemesWithTerminator

./autogen.sh

./configure --prefix=/usr

make -j$(nproc)        # Compile with multiple cores

sudo make install      # Install system-wide

sudo ldconfig          # Update shared library cache

# Test
make && ./NormalizeCSV AR RAW ../data/dataset_AR-EN.txt
make && ./NormalizeCSV AR IPA ../data/dataset_AR-EN-concise.txt
make && ./NormalizeCSV FA RAW ../data/dataset_FA-EN-AR.txt
make && ./NormalizeCSV FA IPA ../data/dataset_FA-EN-AR-concise.txt
make && ./NormalizeCSV AR IPA ../data/test.txt