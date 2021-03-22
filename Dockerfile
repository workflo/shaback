FROM debian:10.8 AS build

RUN apt-get update
RUN apt-get install -y cmake g++ libssl-dev libz-dev liblua5.1-dev \
    libbz2-dev liblzma-dev git libgdbm-dev
RUN apt-get install libcurl4-openssl-dev libssl-dev uuid-dev zlib1g-dev libpulse-dev

RUN mkdir -p /aws-sdk-cpp/build
RUN cd /aws-sdk-cpp && git clone -b version1.9 --recurse-submodules https://github.com/aws/aws-sdk-cpp
RUN cd build && cmake /aws-sdk-cpp/aws-sdk-cpp -D BUILD_ONLY="s3"
RUN make && make install

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
