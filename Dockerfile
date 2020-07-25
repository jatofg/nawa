FROM debian:buster-slim
COPY examples /opt/nawasrc/examples
COPY include /opt/nawasrc/include
COPY libs /opt/nawasrc/libs
COPY src /opt/nawasrc/src
COPY systemd /opt/nawasrc/systemd
COPY CMakeLists.txt /opt/nawasrc/
COPY docker/nawa/config.ini /opt/nawasrc/
ADD https://github.com/jatofg/fastcgipp/archive/lite_legacy.zip /opt/fastcgisrc/
RUN set -eux; \
	apt-get update; \
	apt-get install -y --no-install-recommends \
		libssl1.1 libcurl4 libargon2-1 \
		libssl-dev libboost-dev libcurl4-openssl-dev libargon2-0-dev \
		unzip g++ libc6-dev make cmake; \
	cd /opt/fastcgisrc/; \
	unzip -q lite_legacy.zip; \
	mkdir fastcgipp-lite/build; \
	cd fastcgipp-lite/build; \
	cmake -DCMAKE_BUILD_TYPE=RELEASE ..; \
	make; \
	make install; \
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
	echo "/usr/local/lib" > /etc/ld.so.conf.d/libc.conf; \
	ldconfig -v; \
	cd /; \
	rm -R /opt/nawasrc; \
	rm -R /opt/fastcgisrc; \
	apt-get -y remove \
		libssl-dev libboost-dev libcurl4-openssl-dev libargon2-0-dev \
		unzip g++ libc6-dev make cmake; \
	apt-get -y autoremove; \
	apt-get clean;
EXPOSE 8000
CMD ["nawarun", "/nawa/config.ini"]
