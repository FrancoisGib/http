FROM ubuntu:latest AS build

WORKDIR /app

RUN apt-get update && apt-get install -y \
   build-essential \
   libssl-dev \
   && rm -rf /var/lib/apt/lists/*

COPY include ./include
COPY src ./src
COPY makefile ./
RUN make http-static

FROM alpine

WORKDIR /app

COPY --from=build /app/http ./
COPY examples ./examples
COPY cert ./cert

EXPOSE 8080
LABEL AUTHOR="François Gibier"

CMD ["./http", "8080"]
