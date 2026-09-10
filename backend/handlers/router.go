package handlers

import (
	"fmt"
	"net/http"
)

// the main handler which determines either json format output is expected or xlsx table
func OutputTypesRouter(w http.ResponseWriter, req *http.Request) {
	switch req.URL.Query().Get("type") {
	case "", "json":
		calculateJSON(w, req)
	case "excel":
		calculateXlsx(w, req)
	default:
		writeJSONERresponse(w, http.StatusBadRequest,
			fmt.Errorf("Unexpected output data type %q. Available types are json and xlsx"))
	}
}
