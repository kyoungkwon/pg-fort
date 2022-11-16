FROM ubuntu:latest as build-deps

SHELL ["/bin/bash", "-c"]

# TODO: merge into single RUN
RUN apt update
RUN apt -y install build-essential
RUN apt -y install cmake
RUN apt -y install gdb
RUN apt -y install git
RUN apt -y install libgmock-dev
RUN apt -y install libgtest-dev
RUN apt -y install libpqxx-dev
RUN apt -y install nlohmann-json3-dev
RUN apt -y install pkg-config
RUN apt -y install valgrind


FROM build-deps as build

COPY . /src
WORKDIR /src

RUN mkdir build \
    && cd build \
    && cmake -DBUILD_SHARED_LIBS=OFF .. \
    && make


FROM ubuntu:latest as runtime

RUN mkdir /usr/local/fanout-service
COPY --from=build /src/build/fanout-service /usr/local/fanout-service/fanout-service

WORKDIR /usr/local/fanout-service

ENTRYPOINT [ "./fanout-service" ]
