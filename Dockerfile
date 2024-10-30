FROM scratch

COPY http /
COPY src ./

ENTRYPOINT ["./http", "8080"]