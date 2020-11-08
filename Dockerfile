FROM debian:buster-slim
COPY examples /opt/nawasrc/examples
COPY include /opt/nawasrc/include
COPY libs /opt/nawasrc/libs
COPY src /opt/nawasrc/src
COPY systemd /opt/nawasrc/systemd
COPY CMakeLists.txt /opt/nawasrc/
COPY docker/nawa/config.ini /opt/nawasrc/
RUN set -eux; \
	apt-get update; \
	apt-get install -y --no-install-recommends \
		ca-certificates git libssl1.1 libcurl4 libargon2-1 libboost-system1.67.0 libboost-thread1.67.0 \
		libssl-dev libboost-dev libboost-system1.67-dev libboost-thread1.67-dev libcurl4-openssl-dev libargon2-0-dev \
		unzip g++ libc6-dev make cmake; \
	mkdir /opt/nawasrc/build; \
	cd /opt/nawasrc/build; \
	cmake -DCMAKE_BUILD_TYPE=RELEASE ..; \
	make nawarun; \
	make helloworld; \
	make install; \
	mkdir /nawa; \
	cp libhelloworld.so /nawa/; \
	cp ../config.ini /nawa/; \
	useradd -M nawa; \
	chown -R nawa:nawa /nawa; \
	cd /; \
	rm -R /opt/nawasrc; \
	apt-get -y remove \
		git libssl-dev libboost-dev libboost-system1.67-dev libboost-thread1.67-dev libcurl4-openssl-dev libargon2-0-dev \
		unzip g++ libc6-dev make cmake; \
	apt-get -y autoremove; \
	apt-get clean;
EXPOSE 8000
CMD ["nawarun", "/nawa/config.ini"]
