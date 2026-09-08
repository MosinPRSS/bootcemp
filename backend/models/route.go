package models

import (
	"time"

	"github.com/lib/pq"
	"gorm.io/gorm"
)

type Route struct {
	*gorm.Model

	StartTime  time.Time
	EndTime    time.Time
	DaysOfWeek pq.Int32Array `gorm:"type:smallint[]"`
}
