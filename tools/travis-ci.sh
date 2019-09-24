#!/bin/bash -e
# Install some dependencies manually
DEPS_DIR="${HOME}/deps"
mkdir -p ${DEPS_DIR}
cd ${DEPS_DIR}

# Recent CMake:
CMAKE_URL="https://cmake.org/files/v3.11/cmake-3.11.4-Linux-x86_64.tar.gz"
mkdir cmake && wget --no-check-certificate --quiet -O - ${CMAKE_URL} | tar --strip-components=1 -xz -C cmake
export PATH=${DEPS_DIR}/cmake/bin:${PATH}
cmake --version

# Function for creating a new 'build' directory
function renew_build() {
  echo "Renew build directory..."
  cd $TRAVIS_BUILD_DIR

  # Remove any existing build directory
  [ -e build ] && rm -r -f build
  mkdir build
  cd build

  # Configure the project and build it
  cmake -GNinja -DCMAKE_CXX_FLAGS="$CMAKE_CXX_FLAGS -Werror" \
        -DFU2_WITH_NO_EXCEPTIONS=$NO_EXCEPTIONS -DFU2_WITH_NO_DEATH_TESTS=ON \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=$1 ..
}

if [[ $CXX == *"clang"* ]]; then
  # Abort when the sanitizers detect an error
  LSAN_OPTIONS=verbosity=1:log_threads=1:abort_on_error=1
  ASAN_OPTIONS=verbosity=1:log_threads=1:abort_on_error=1
  UBSAN_OPTIONS=print_stacktrace=1:symbolize=1:halt_on_error=1:print_summary=1

  # - ASan (LSan):
  echo "Building with address sanitizer..."
  CMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer"
  renew_build Debug

  ninja -j2
  ctest --verbose

  # - UBSan:
  echo "Building with undefined behaviour sanitizer..."
  CMAKE_CXX_FLAGS="-fsanitize=undefined -fno-omit-frame-pointer"
  renew_build Debug

  ninja -j2
  ctest --verbose
else
  # Build an run the tests suite with valgrind
  echo "Building for valgrind..."
  renew_build Debug

  ninja -j2
  valgrind --error-exitcode=1 --leak-check=full --show-reachable=yes ctest --verbose
fi

# Also check the release builds of all toolchains
echo "Building in Release mode..."
renew_build Release
ninja -j2
ctest --verbose
