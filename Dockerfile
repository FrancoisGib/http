FROM scratch

WORKDIR /app

COPY http ./
COPY examples ./examples

COPY cert ./cert

CMD ["./http", "8080"]