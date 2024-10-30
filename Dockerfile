FROM scratch

WORKDIR /app

COPY http ./
COPY src ./src

ENTRYPOINT ["./http", "8080"]