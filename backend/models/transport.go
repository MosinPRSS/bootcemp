package models

import "gorm.io/gorm"

type Transport struct {
	*gorm.Model
	GovernmentNumber string
}
