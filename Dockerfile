FROM scratch

WORKDIR /app

COPY http ./
COPY src ./src

COPY cert ./cert

CMD ["./http", "8080"]