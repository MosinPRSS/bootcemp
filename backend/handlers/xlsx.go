package handlers

import (
	"bootcemp/server/module"
	"fmt"
	"io"
	"net/http"
	"path/filepath"
)

func calculateXlsx(w http.ResponseWriter, req *http.Request) {
	if req.Method != http.MethodPost {
		writeJSONERresponse(w, http.StatusMethodNotAllowed,
			fmt.Errorf("Only POST is Allowed for data uploading"))
		return
	}

	body, err := io.ReadAll(req.Body)
	if err != nil {
		writeJSONERresponse(w, http.StatusBadRequest,
			fmt.Errorf("Invalid requset: %w", err))
		return
	}
	defer req.Body.Close()

	inputRoot := &module.Root{}
	validatedInput, err := inputRoot.ValidateJSON(body)
	if err != nil {
		writeJSONERresponse(w, http.StatusBadRequest,
			fmt.Errorf("Failed validating input JSON file: %w", err))
	}

	result, err := validatedInput.CalculateSolution()
	if err != nil {
		writeJSONERresponse(w, http.StatusInternalServerError,
			fmt.Errorf("Failed calculating result: %w", err))
	}

	path, err := generateXLSX(*result)
	if err != nil {
		writeJSONERresponse(w, http.StatusInternalServerError,
			fmt.Errorf("Failed generating excel-table: %w", err))
	}

	downloadURL := fmt.Sprintf("%s://%s/files/%s", proto(req), req.Host, filepath.Base(path))
	writeJSONERresponse(w, http.StatusOK, map[string]string{"file_url": downloadURL})
}

func proto(r *http.Request) string {
	if r.TLS != nil {
		return "https"
	}
	return "http"
}
