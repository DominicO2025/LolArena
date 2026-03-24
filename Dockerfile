FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git

WORKDIR /app

COPY . .

RUN mkdir build && cd build && cmake .. && make

EXPOSE 18080

CMD ["./build/arena_tracker"]