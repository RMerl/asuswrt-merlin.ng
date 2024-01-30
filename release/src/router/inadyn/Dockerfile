FROM alpine:latest
WORKDIR /root
COPY . ./

RUN apk --update --no-cache add --virtual .build-dependencies \
  autoconf \
  automake \
  confuse-dev \
  gcc \
  gnutls-dev \
  libc-dev \
  libtool \
  make

RUN ./autogen.sh
RUN ./configure --prefix=/usr --sysconfdir=/etc --localstatedir=/var
RUN make install

FROM alpine:latest
ARG revision=$revision
LABEL org.opencontainers.image.title="In-a-Dyn"
LABEL org.opencontainers.image.description="A dynamic DNS (DDNS) client with multiple SSL/TLS library support"
LABEL org.opencontainers.image.url="https://github.com/troglobit/inadyn"
LABEL org.opencontainers.image.licenses="GPL-2.0"

RUN apk --update --no-cache add \
  ca-certificates \
  confuse \
  gnutls

COPY --from=0 /usr/sbin/inadyn /usr/sbin/inadyn
COPY --from=0 /usr/share/doc/inadyn /usr/share/doc/inadyn
ENTRYPOINT ["/usr/sbin/inadyn", "--foreground"]
