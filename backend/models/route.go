package models

import (
	"time"

	"github.com/lib/pq"
	"gorm.io/gorm"
)

type Route struct {
	gorm.Model

	// StartStop foreign key
	StartStopID uint
	StartStop   Stop

	// EndStop foreign key
	EndStopID uint
	EndStop   Stop

	// additional data
	StartTime time.Time
	EndTime   time.Time

	// when our transport works
	// (i hope it will work...)
	DaysOfWeek pq.Int32Array `gorm:"type:smallint[]"`
}

// for readability
type Geostring string

type Stop struct {
	gorm.Model

	Name string

	// Will be available as _string_
	// When selecting a data from DB you must use smth like
	// db.Select("..., ST_AsText(coordinates) as coordinates")...
	//
	// NOTE: can be used ONLY IN postgresql
	//
	// For adding coordinates in variable use this form:
	// newPlace := Stop{
	// 		...
	//		Coordinates: "POINT(37.6173 55.7558)"
	//      ...
	// }
	// _Coordinates_ uses WGS 84 standard
	Coordinates Geostring `gorm:"type:geography(Point, 4326)"`
}
