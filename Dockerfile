FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# ── System dependencies ───────────────────────────────────────────────────────
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libcurl4-openssl-dev \
    nlohmann-json3-dev \
    libssl-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# ── Download Crow (header-only) ───────────────────────────────────────────────
RUN git clone --depth=1 https://github.com/CrowCpp/Crow.git /crow

WORKDIR /app
COPY . .

# ── Build ─────────────────────────────────────────────────────────────────────
RUN cmake -S . -B build -DCROW_DIR=/crow/include && \
    cmake --build build --parallel

EXPOSE 18080

# RIOT_API_KEY must be passed at runtime:
#   docker run -e RIOT_API_KEY=RGAPI-xxx ...
CMD ["./build/arena_tracker"]