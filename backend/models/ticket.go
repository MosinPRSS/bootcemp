package models

import "gorm.io/gorm"

type Ticket struct {
	gorm.Model

	// Route foreign key
	RouteID uint
	Route   Route

	// somehow will be calculated maybe?
	TotalCost uint32

	// Stop foreign key
	EndStopID uint
	EndStop   Stop
}
