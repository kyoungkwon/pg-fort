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
RUN apt -y install libpg-query-dev
RUN apt -y install pkg-config


FROM build-deps as build

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
