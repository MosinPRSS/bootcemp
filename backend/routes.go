package main

import (
	"bootcemp/server/module"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
)

// A general handlers that takes JSON file as an input and returns another JSON as a result
func calculateJSON(w http.ResponseWriter, req *http.Request) {
	if req.URL.Path != "/" {
		http.NotFound(w, req)
		return
	}

	if req.Method != http.MethodPost {
		writeJSONERresponse(w, http.StatusMethodNotAllowed, fmt.Errorf("Only POST is Allowed for data uploading"))
		return
	}

	body, err := io.ReadAll(req.Body)
	if err != nil {
		writeJSONERresponse(w, http.StatusBadRequest, fmt.Errorf("Invalid requset: %w", err))
		return
	}
	defer req.Body.Close()

	inputRoot := &module.Root{}
	validatedInput, err := inputRoot.ValidateJSON(body)
	if err != nil {
		writeJSONERresponse(w, http.StatusBadRequest, fmt.Errorf("Failed validating input JSON file: %w", err))
	}

	result, err := validatedInput.CalculateSolution()
	if err != nil {
		writeJSONERresponse(w, http.StatusInternalServerError, fmt.Errorf("Failed calculating result: %w", err))
	}

	writeJSONERresponse(w, http.StatusOK, result)
}

func writeJSONERresponse(w http.ResponseWriter, status int, payload any) error {
	w.Header().Set("Allow", http.MethodPost)
	w.Header().Set("Content-type", "application/json; charset=utf-8")
	w.WriteHeader(status)

	if err, ok := payload.(error); ok {
		payload = map[string]string{"error": err.Error()}
	}

	return json.NewEncoder(w).Encode(payload)
}
