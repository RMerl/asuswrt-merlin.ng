FROM alpine

RUN apk add --no-cache gcc libc-dev linux-headers make

COPY *.c *.h Makefile /source/

RUN cd /source && make

FROM alpine

COPY --from=0 /source/jitterentropy-rngd /usr/bin

CMD ["jitterentropy-rngd", "-v"]
