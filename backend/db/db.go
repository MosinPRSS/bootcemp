package db

import (
	"bootcemp/server/models"
	"fmt"
	"os"

	"bootcemp/server/config"

	"gorm.io/driver/postgres"
	"gorm.io/gorm"
)

func buildDSN() string {
	user := os.Getenv("POSTGRES_USER")
	pass := os.Getenv("POSTGRES_PASSWORD")
	dbname := os.Getenv("POSTGRES_DB")
	host := os.Getenv("POSTGRES_HOST")
	port := os.Getenv("POSTGRES_PORT")
	tz := "Europe/Moscow"

	return fmt.Sprintf("host=%s user=%s password=%s dbname=%s port=%s sslmode=%s TimeZone=%s",
		host, user, pass, dbname, port, "disable", tz)
}

func connect() (*gorm.DB, error) {
	dsn := buildDSN()

	db, err := gorm.Open(postgres.Open(dsn), &gorm.Config{})
	if err != nil {
		return nil, fmt.Errorf("postgres connection failed (host=%s db=%s): %w",
			os.Getenv("POSTGRES_HOST"),
			os.Getenv("POSTGRES_DB"),
			err)
	}
	return db, nil
}

func autoMigrate(db *gorm.DB) error {
	var Models = []interface{}{
		&models.Driver{},
		&models.DriverRoute{},
		&models.Route{},
		&models.Stop{},
		&models.Ticket{},
		&models.Transport{},
		&models.TransportModel{},
		&models.TransportType{},
	}
	return db.AutoMigrate(Models...)
}

func Init(logger *config.Logger) (*gorm.DB, error) {
	db, err := connect()
	if err != nil {
		return nil, err
	}

	logger.InfLogger.Printf("Performing schema migration")
	if err := autoMigrate(db); err != nil {
		return nil, fmt.Errorf("Database migration failed", err)
	}

	return db, nil
}
