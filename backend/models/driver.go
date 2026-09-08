package models

import (
	"gorm.io/gorm"
)

type Driver struct {
	*gorm.Model
	// i dont see any point for separating names
	// like 1st name, surname and etc.
	Name string
}
