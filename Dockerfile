FROM ubuntu:latest as build

SHELL ["/bin/bash", "-c"]

# TODO: merge into single RUN
RUN apt-get update
RUN apt-get -y install build-essential
RUN apt-get -y install clang
RUN apt-get -y install cmake
RUN apt-get -y install gdb
RUN apt-get -y install libgtest-dev
RUN apt-get -y install pkg-config
RUN apt-get -y install libgmock-dev

COPY . /src
WORKDIR /src

RUN mkdir build \
    && cd build \
    && cmake .. \
    && make


FROM ubuntu:latest as runtime

RUN mkdir /usr/local/postgresql-proxy
COPY --from=build /src/build/postgresql-proxy /usr/local/postgresql-proxy/postgresql-proxy

WORKDIR /usr/local/postgresql-proxy

ENTRYPOINT [ "./postgresql-proxy" ]
