FROM ubuntu:22.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake ninja-build git libsqlite3-dev ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY . /app
WORKDIR /app/QuantumFractalSystem

RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --parallel

# Run tests during build to validate
RUN cd build && ctest --output-on-failure

CMD ["./build/QuantumFractalSystem"]
