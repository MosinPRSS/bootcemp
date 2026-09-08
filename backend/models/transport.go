package models

import "gorm.io/gorm"

type Transport struct {
	gorm.Model
	GovernmentNumber string

	// foreign key
	TransportModelID uint
	TransportModel   TransportModel
}

type TransportModel struct {
	gorm.Model
	Name        string
	CostPerHour float64
	Capacity    uint32

	// foreign key
	TransportTypeID uint
	TransportType   TransportType
}

type TransportType struct {
	gorm.Model
	Name string
}
