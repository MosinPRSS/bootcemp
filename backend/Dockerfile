# stage 1

FROM golang:1.27.1-alpine AS builder

WORKDIR /app/server

# copies for deps
COPY go.mod go.sum ./
RUN go mod download

COPY . .

RUN CGO_ENABLED=0 GOOS=linux go build -ldflags="-w -s" -o main .

# stage 2
FROM golang:1.27.1-alpine AS runner

COPY --from=builder /app/server/main /main

EXPOSE 8080

ENTRYPOINT [ "/main" ]
