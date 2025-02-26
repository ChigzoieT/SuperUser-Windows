#ifndef UPLOADDATA_H
#define UPLOADDATA_H

#include <string>

// UploadData sends a JSON payload via HTTP POST to the server endpoint.
// The JSON object includes the following keys:
//   - "userkey": The user key.
//   - "text":    Some associated text.
//   - "filename": The file path (used as the filename).
//   - "file":    The raw binary content of the file read from filePath.
// Returns "success" if the POST request was sent successfully,
// otherwise returns "failed".
void UploadData(const std::wstring &userkey,
                       const std::wstring &text,
                       const std::wstring &filePath);

#endif // UPLOADDATA_H
