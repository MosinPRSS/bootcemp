package main

import (
	"bootcemp/server/config"
	"context"
	"log"
	"net/http"
	"os"
	"os/signal"
	"syscall"
	"time"

	"github.com/joho/godotenv"
	"golang.org/x/sync/errgroup"
)

func main() {
	// general context to shut the server gracefully
	generalCtx, stopServer := context.WithTimeout(context.Background(), 15*time.Second)
	defer stopServer()

	// an error group with its goroutines context and errors handling
	wg, _ := errgroup.WithContext(context.Background())

	mux := http.NewServeMux()
	mux.HandleFunc("/", calculateJSON)

	const addr = "127.0.0.1:8080"

	// custom logging
	logger := &config.Logger{
		ErrLogger: log.New(os.Stderr, "ERROR\t", log.Ldate|log.Ltime|log.Lshortfile),
		InfLogger: log.New(os.Stdout, "INFO\t", log.Ldate|log.Ltime),
	}

	srv := &http.Server{
		Addr:     addr,
		ErrorLog: logger.ErrLogger,
		Handler:  mux,
	}

	_ = godotenv.Load(".env")

	/*
		DB, err := db.Init(logger)
		if err != nil {
			logger.ErrLogger.Fatalf("Error initializing DB: %s", err)
		}
	*/
	wg.Go(func() error {
		logger.InfLogger.Printf("Attempting to bind on %s", addr)

		if err := srv.ListenAndServe(); err != nil && err != http.ErrServerClosed {
			return err
		}
		return nil
	})

	logger.InfLogger.Printf("Starting HTTP server %s", addr)
	if err := wg.Wait(); err != nil {
		logger.ErrLogger.Fatalf("Error starting HTTP server on %s: %s", addr, err)
	}

	logger.InfLogger.Printf("Successfully started the server on %s", srv.Addr)

	signalCtx, signalStop := signal.NotifyContext(context.Background(), os.Interrupt, syscall.SIGTERM)
	defer signalStop()

	<-signalCtx.Done()
	logger.InfLogger.Print("Captured shutdown signal")

	if err := srv.Shutdown(generalCtx); err != nil {
		logger.ErrLogger.Fatalf("Error gracufully shutting down an HTTP server: %s", err)
	}

	// to make us able to perform DB's graceful shut, we need to extract a raw sql.DB from gorm.DB wrapper
	/*
		if sqlDB, err := DB.DB(); err != nil {
			if err := sqlDB.Close(); err != nil {
				logger.ErrLogger.Fatalf("Error closing database: %s", err)
			}
		} else {
			logger.ErrLogger.Fatalf("Error getting sql DB handle: %s", err)
		}*/
	logger.InfLogger.Print("An HTTP server has been gracefully shutted down")
}
