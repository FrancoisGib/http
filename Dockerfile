FROM scratch

COPY a.out /

CMD ["./a.out", "8080"]