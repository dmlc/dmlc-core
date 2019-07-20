FROM multiarch/debian-debootstrap:s390x-jessie

ENV DEBIAN_FRONTEND noninteractive
ENV GOSU_VERSION 1.10

RUN uname -a
# Add repositories for Debian Jessie
RUN printf 'deb http://deb.debian.org/debian/ oldstable main contrib non-free\ndeb-src http://deb.debian.org/debian/ oldstable main contrib non-free\ndeb http://deb.debian.org/debian/ oldstable-updates main contrib non-free\ndeb-src http://deb.debian.org/debian/ oldstable-updates main contrib non-free\ndeb http://deb.debian.org/debian-security oldstable/updates main' > /etc/apt/sources.list
RUN apt-get update && \
  apt-get install -y --no-install-suggests --no-install-recommends \
  build-essential \
  gcc \
  make \
  git \
  cmake \
  ca-certificates && \
  update-ca-certificates --fresh

# Install lightweight sudo (not bound to TTY)
RUN set -ex; \
    wget --no-check-certificate -O /usr/local/bin/gosu "https://github.com/tianon/gosu/releases/download/$GOSU_VERSION/gosu-amd64" && \
    chmod +x /usr/local/bin/gosu && \
    gosu nobody true

COPY entrypoint.sh /scripts/

WORKDIR /workspace
ENTRYPOINT ["/scripts/entrypoint.sh"]
