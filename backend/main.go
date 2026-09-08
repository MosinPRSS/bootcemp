package main

import (
	"context"
	"log"
	"net/http"
	"os"

	"golang.org/x/sync/errgroup"
)

func main() {
	wg, _ := errgroup.WithContext(context.Background())
	mux := http.NewServeMux()

	infLogger := log.New(os.Stdout, "INFO\t", log.Ldate|log.Ltime)
	errLogger := log.New(os.Stderr, "ERROR\t", log.Ldate|log.Ltime|log.Lshortfile)

	const addr = "127.0.0.1:4308"

	srv := &http.Server{
		Addr:     addr,
		ErrorLog: errLogger,
		Handler:  mux,
	}

	wg.Go(func() error {
		infLogger.Printf("Attempting to bind on %s", addr)

		if err := srv.ListenAndServe(); err != nil && err != http.ErrServerClosed {
			return err
		}
		return nil
	})

	infLogger.Printf("Starting HTTP server %s", addr)
	if err := wg.Wait(); err != nil {
		errLogger.Fatalf("Error starting HTTP server on %s: %s", addr, err)
	}
}
