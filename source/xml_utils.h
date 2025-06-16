#ifndef XML_UTILS_H
#define XML_UTILS_H

#include <stddef.h> // For size_t

// Function to find the starting point of a tag's value within XML content.
// Returns a pointer to the start of the value, or NULL if tag not found.
const char* find_tag_value_start(const char* xml_content, const char* tag_name);

// Function to extract the string value of a given tag.
// Copies the value into `buffer`.
// Returns `buffer` on success, or NULL if tag not found, value doesn't fit, or other error.
// Ensures `buffer` is null-terminated (as an empty string if value doesn't fit or on other errors before returning NULL).
char* get_tag_string(const char* xml_content, const char* tag_name, char* buffer, int buffer_size);

// Function to modify the value of a given tag within mutable XML content.
// `xml_buffer_total_size` is the total capacity of the `xml_content` buffer.
// Returns 0 on success, -1 for general errors (e.g., tag not found, invalid parameters),
// or -2 if the new value would cause an overflow.
int set_tag_string(char* xml_content, size_t xml_buffer_total_size, const char* tag_name, const char* new_value);

#endif // XML_UTILS_H
