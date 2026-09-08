package config

import "log"

type Logger struct {
	ErrLogger *log.Logger
	InfLogger *log.Logger
}
