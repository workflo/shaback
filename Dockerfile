FROM debian:9.8 AS build

RUN apt-get update
RUN apt-get install -y cmake g++ libssl-dev libz-dev liblua5.1-dev \
    libbz2-dev liblzma-dev git libgdbm-dev

COPY src /src/

WORKDIR /src
RUN rm -f /src/CMakeCache.txt
RUN cmake .
RUN make install


FROM debian:9.8

RUN apt-get update
RUN apt-get install -y openssl
RUN apt-get install -y cron
RUN apt-get install -y rsync
RUN apt-get install -y liblua5.1 openssl
RUN apt-get install -y awscli

RUN apt-get clean && rm -rf /var/cache/apt

COPY --from=build /usr/local/bin/shaback /usr/local/bin/shaback
COPY --from=build /usr/local/etc/shaback /usr/local/etc/shaback/
COPY docker-entrypoint.sh /

VOLUME ["/backup", "/src"]

ENTRYPOINT ["/docker-entrypoint.sh"]
#CMD ["--help"]
