# daemon runs in the background
# run something like tail /var/log/DeroGoldd/current to see the status
# be sure to run with volumes, ie:
# docker run -v $(pwd)/DeroGoldd:/var/lib/DeroGoldd -v $(pwd)/wallet:/home/derogold --rm -ti derogold:0.6.0
ARG base_image_version=0.6.0
FROM phusion/baseimage:$base_image_version

ADD https://github.com/just-containers/s6-overlay/releases/download/v1.21.2.2/s6-overlay-amd64.tar.gz /tmp/
RUN tar xzf /tmp/s6-overlay-amd64.tar.gz -C /

ADD https://github.com/just-containers/socklog-overlay/releases/download/v2.1.0-0/socklog-overlay-amd64.tar.gz /tmp/
RUN tar xzf /tmp/socklog-overlay-amd64.tar.gz -C /

ARG DEROGOLD_BRANCH=master
ENV DEROGOLD_BRANCH=${TURTLECOIN_BRANCH}

# install build dependencies
# checkout the latest tag
# build and install
RUN apt-get update && \
    apt-get install -y \
      build-essential \
      python-dev \
      gcc-9.3 \
      g++-9.3 \
      git cmake \
      libboost1.69-all-dev && \
    git clone https://github.com/derogold/derogold.git /src/derogold && \
    cd /src/derogold && \
    git checkout $DEROGOLD_BRANCH && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_CXX_FLAGS="-g0 -Os -fPIC -std=gnu++11" .. && \
    make -j$(nproc) && \
    mkdir -p /usr/local/bin && \
    cp src/DeroGoldd /usr/local/bin/DeroGoldd && \
    cp src/zedwallet-beta /usr/local/bin/zedwallet-beta && \
    cp src/miner /usr/local/bin/miner && \
    strip /usr/local/bin/DeroGoldd && \
    strip /usr/local/bin/zedwallet-beta && \
    strip /usr/local/bin/miner && \
    cd / && \
    rm -rf /src/derogold && \
    apt-get remove -y build-essential python-dev gcc-4.9 g++-4.9 git cmake libboost1.69-all-dev && \
    apt-get autoremove -y && \
    apt-get install -y  \
      libboost-system1.69.0 \
      libboost-filesystem1.69.0 \
      libboost-thread1.69.0 \
      libboost-date-time1.69.0 \
      libboost-chrono1.69.0 \
      libboost-regex1.69.0 \
      libboost-serialization1.69.0 \
      libboost-program-options1.69.0 \
      libicu66

# setup the derogoldd service
RUN useradd -r -s /usr/sbin/nologin -m -d /var/lib/DeroGoldd DeroGoldd && \
    useradd -s /bin/bash -m -d /home/derogold DeroGoldd && \
    mkdir -p /etc/services.d/DeroGoldd/log && \
    mkdir -p /var/log/DeroGoldd && \
    echo "#!/usr/bin/execlineb" > /etc/services.d/DeroGoldd/run && \
    echo "fdmove -c 2 1" >> /etc/services.d/DeroGoldd/run && \
    echo "cd /var/lib/DeroGoldd" >> /etc/services.d/DeroGoldd/run && \
    echo "export HOME /var/lib/DeroGoldd" >> /etc/services.d/DeroGoldd/run && \
    echo "s6-setuidgid DeroGoldd /usr/local/bin/DeroGoldd" >> /etc/services.d/DeroGoldd/run && \
    chmod +x /etc/services.d/DeroGoldd/run && \
    chown nobody:nogroup /var/log/DeroGoldd && \
    echo "#!/usr/bin/execlineb" > /etc/services.d/DeroGoldd/log/run && \
    echo "s6-setuidgid nobody" >> /etc/services.d/DeroGoldd/log/run && \
    echo "s6-log -bp -- n20 s1000000 /var/log/DeroGoldd" >> /etc/services.d/DeroGoldd/log/run && \
    chmod +x /etc/services.d/DeroGoldd/log/run && \
    echo "/var/lib/DeroGoldd true DeroGoldd 0644 0755" > /etc/fix-attrs.d/DeroGoldd-home && \
    echo "/home/derogold true turtlecoin 0644 0755" > /etc/fix-attrs.d/derogold-home && \
    echo "/var/log/DeroGoldd true nobody 0644 0755" > /etc/fix-attrs.d/DeroGoldd-logs

VOLUME ["/var/lib/DeroGoldd", "/home/derogold","/var/log/DeroGoldd"]

ENTRYPOINT ["/init"]
CMD ["/usr/bin/execlineb", "-P", "-c", "emptyenv cd /home/derogold export HOME /home/derogold s6-setuidgid derogold /bin/bash"]
