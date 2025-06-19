# -------------------- 1. Build stage ---------------------------------------
FROM ubuntu:22.04 AS builder
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake g++ git pkg-config libopencv-dev python3-opencv \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

# CMakeLists.txt must list BOTH src files: main.cpp  tracker.cpp
RUN rm -rf build && mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . -j$(nproc) && \
    cmake --install . --prefix /stage && \
    echo "== /stage tree ==" && ls -R /stage

# -------------------- 2. Runtime stage -------------------------------------
FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        libopencv-core4.5d libopencv-imgcodecs4.5d libopencv-imgproc4.5d \
        libopencv-videoio4.5d && \
    rm -rf /var/lib/apt/lists/*

# Copy *everything* that was installed
COPY --from=builder /stage /stage

# Pick the crop_tracking binary no matter where it is
RUN cp $(find /stage -type f -name 'crop_tracking*' | head -n1) \
        /usr/local/bin/crop_tracking && \
    chmod +x /usr/local/bin/crop_tracking

ENTRYPOINT ["crop_tracking"]
