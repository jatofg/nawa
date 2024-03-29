# For CI and testing purposes only
FROM debian:buster-slim
COPY examples /opt/nawasrc/examples
COPY include /opt/nawasrc/include
COPY internal /opt/nawasrc/internal
COPY libs /opt/nawasrc/libs
COPY src /opt/nawasrc/src
COPY systemd /opt/nawasrc/systemd
COPY tests /opt/nawasrc/tests
COPY docs /opt/nawasrc/docs
COPY CMakeLists.txt /opt/nawasrc/
COPY version.cmake /opt/nawasrc/
COPY configure.cmake /opt/nawasrc/
COPY docker/nawa/config.ini /opt/nawasrc/
RUN set -eux; \
	apt-get update; \
	apt-get install -y --no-install-recommends \
		ca-certificates git libssl1.1 libcurl4 libargon2-1 libboost-system1.67.0 libboost-thread1.67.0 libboost-chrono1.67.0 libboost-date-time1.67.0 \
		libssl-dev libboost-dev libboost-system1.67-dev libboost-thread1.67-dev libcurl4-openssl-dev libargon2-0-dev \
		unzip g++ libc6-dev make cmake; \
	mkdir /opt/nawabuild; \
	cd /opt/nawabuild; \
	cmake -DCMAKE_BUILD_TYPE=RELEASE -DBuildTests=ON ../nawasrc; \
	make; \
	make install; \
	mkdir /nawa; \
	cp libhelloworld.so /nawa/; \
	cp ../nawasrc/config.ini /nawa/; \
	useradd -M nawa; \
	chown -R nawa:nawa /nawa; \
	cd /; \
	rm -R /opt/nawasrc; \
	apt-get -y remove \
		git libssl-dev libboost-dev libboost-system1.67-dev libboost-thread1.67-dev libcurl4-openssl-dev libargon2-0-dev \
		unzip g++ libc6-dev make cmake; \
	apt-get -y autoremove; \
	apt-get clean;
