FROM debian:9.3 AS build

RUN apt-get update
RUN apt-get install -y cmake g++ libssl-dev libz-dev liblua5.1-dev \
     libbz2-dev liblzma-dev git libgdbm-dev

COPY src /src/

WORKDIR /src
RUN rm -f /src/CMakeCache.txt
RUN cmake .
RUN make install


FROM debian:9.3
COPY --from=build /usr/local/bin/shaback /usr/local/bin/shaback
COPY --from=build /usr/local/etc/shaback /usr/local/etc/shaback/