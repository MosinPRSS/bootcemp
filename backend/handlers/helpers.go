package handlers

import (
	"encoding/json"
	"net/http"
)

func writeJSONERresponse(w http.ResponseWriter, status int, payload any) error {
	w.Header().Set("Allow", http.MethodPost)
	w.Header().Set("Content-type", "application/json; charset=utf-8")
	w.WriteHeader(status)

	if err, ok := payload.(error); ok {
		payload = map[string]string{"error": err.Error()}
	}

	return json.NewEncoder(w).Encode(payload)
}
