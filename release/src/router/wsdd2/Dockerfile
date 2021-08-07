FROM alpine:latest AS builder
RUN apk add --update --no-cache \
  make \
  gcc \
  musl-dev \
  linux-headers
COPY * /root/
RUN cd /root/ && make && find /root 

FROM alpine:latest  
WORKDIR /root/
COPY --from=0 /root/wsdd2 /usr/sbin/
ENTRYPOINT /usr/sbin/wsdd2 -N $HOSTNAME -G ${WORKGROUP:-WORKGROUP}
