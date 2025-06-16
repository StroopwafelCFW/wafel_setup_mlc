#ifndef XML_UTILS_H
#define XML_UTILS_H

#include <stddef.h> // For size_t

// Error codes for XML utility functions
#define XML_UTIL_SUCCESS                   0
#define XML_UTIL_ERROR_GENERAL            -1 // For unspecified or truly general errors (use specific codes when possible)
#define XML_UTIL_ERROR_BUFFER_TOO_SMALL   -2 // Buffer provided by caller is too small for the value (specific to get_tag_string)
#define XML_UTIL_ERROR_TAG_NOT_FOUND      -3 // The requested XML tag was not found
#define XML_UTIL_ERROR_MALFORMED_XML      -4 // e.g., missing closing tag, or inconsistent structure
#define XML_UTIL_ERROR_BAD_PARAMS         -5 // Invalid parameters passed to the function
#define XML_UTIL_ERROR_INTERNAL           -6 // Internal error, e.g., snprintf failure for tag construction

// Function to find the starting point of a tag's value within XML content.
// Returns a pointer to the start of the value, or NULL if tag not found or error in tag construction.
const char* find_tag_value_start(const char* xml_content, const char* tag_name);

// Function to extract the string value of a given tag.
// Copies the value into `buffer`. `buffer` will be an empty string on error if buffer itself is valid.
// Returns XML_UTIL_SUCCESS (0) on success (including for an empty tag value).
// Returns XML_UTIL_ERROR_BAD_PARAMS if input parameters are invalid.
// Returns XML_UTIL_ERROR_TAG_NOT_FOUND if the opening tag is not found.
// Returns XML_UTIL_ERROR_MALFORMED_XML if the closing tag is not found after the opening tag.
// Returns XML_UTIL_ERROR_BUFFER_TOO_SMALL if the `buffer` is too small for the extracted value.
// Returns XML_UTIL_ERROR_INTERNAL for internal processing errors (e.g., constructing tag strings for close tag).
int get_tag_string(const char* xml_content, const char* tag_name, char* buffer, int buffer_size);

// Function to modify the value of a given tag within mutable XML content.
// `xml_buffer_total_size` is the total capacity of the `xml_content` buffer.
// Returns XML_UTIL_SUCCESS (0) on success.
// Returns XML_UTIL_ERROR_BAD_PARAMS for invalid parameters.
// Returns XML_UTIL_ERROR_TAG_NOT_FOUND if the opening tag is not found.
// Returns XML_UTIL_ERROR_MALFORMED_XML if the closing tag is not found after the opening tag.
// Returns XML_UTIL_ERROR_INTERNAL for internal processing errors (e.g., constructing tag strings).
// Returns -2 if the new value would cause an overflow of the `xml_content` buffer (this -2 is specific to set_tag_string).
int set_tag_string(char* xml_content, size_t xml_buffer_total_size, const char* tag_name, const char* new_value);

#endif // XML_UTILS_H
