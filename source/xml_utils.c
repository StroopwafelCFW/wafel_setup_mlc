#include "xml_utils.h" // For function declarations
#include <string.h>    // For strlen, strstr, memcpy, memmove
#include <stdio.h>     // For snprintf (used in find_tag_value_start and set_tag_string to construct tag strings)
#include <stddef.h>    // For ptrdiff_t, size_t (though often available via other headers like string.h)

// XML helper functions (find_tag_value_start, get_tag_string, set_tag_string)
// These are public, declared in xml_utils.h
const char* find_tag_value_start(const char* xml_content, const char* tag_name) {
    if (!xml_content || !tag_name) {
        return NULL;
    }
    char open_tag[128];
    int written = snprintf(open_tag, sizeof(open_tag), "<%s>", tag_name);
    if (written < 0 || (size_t)written >= sizeof(open_tag)) {
        return NULL; // Error during tag construction or tag_name too long
    }
    const char* tag_start = strstr(xml_content, open_tag);
    if (tag_start) {
        return tag_start + strlen(open_tag);
    }
    return NULL;
}

char* get_tag_string(const char* xml_content, const char* tag_name, char* buffer, int buffer_size) {
    if (!xml_content || !tag_name || !buffer || buffer_size <= 0) {
        return NULL;
    }
    buffer[0] = '\0'; // Initialize buffer to empty string for safety
    const char* value_start = find_tag_value_start(xml_content, tag_name);
    if (!value_start) {
        return NULL; // Opening tag not found
    }
    char close_tag[128];
    int written = snprintf(close_tag, sizeof(close_tag), "</%s>", tag_name);
    if (written < 0 || (size_t)written >= sizeof(close_tag)) {
        return NULL; // Error during tag construction or tag_name too long
    }
    const char* value_end = strstr(value_start, close_tag);
    if (!value_end) {
        return NULL; // Closing tag not found
    }
    if (value_start == value_end) { // Empty tag value <tag></tag>
        return buffer; // buffer is already '\0' terminated
    }
    ptrdiff_t value_len = value_end - value_start;
    if (value_len < 0) { // Should not happen with valid XML and strstr logic
        return NULL;
    }

    // Check if the value (excluding null terminator) can fit in the buffer.
    // buffer_size includes space for the null terminator.
    // So, value_len must be less than buffer_size.
    if (value_len >= buffer_size) {
        // Value is too large for the provided buffer.
        // Return NULL; buffer[0] is already '\0'.
        return NULL;
    }

    // If we reach here, value_len < buffer_size, so it fits.
    memcpy(buffer, value_start, value_len);
    buffer[value_len] = '\0';
    return buffer;
}

int set_tag_string(char* xml_content, size_t xml_buffer_total_size, const char* tag_name, const char* new_value) {
    if (!xml_content || xml_buffer_total_size == 0 || !tag_name || !new_value) {
        return -1; // Invalid parameters
    }

    const char* value_start_const = find_tag_value_start(xml_content, tag_name);
    if (!value_start_const) {
        return -1; // Tag not found
    }
    // Calculate the mutable pointer 'value_start' from the const 'value_start_const'
    // by finding its offset from the beginning of xml_content.
    char* value_start = xml_content + (value_start_const - xml_content);

    char close_tag[128]; // Assuming tag names are reasonably short
    int written = snprintf(close_tag, sizeof(close_tag), "</%s>", tag_name);
    if (written < 0 || (size_t)written >= sizeof(close_tag)) {
        return -1; // Error constructing close_tag or tag_name too long
    }

    char* value_end = strstr(value_start, close_tag);
    if (!value_end) {
        return -1; // Closing tag not found
    }

    size_t old_value_len = value_end - value_start;
    size_t new_value_len = strlen(new_value);
    ptrdiff_t len_diff = new_value_len - old_value_len;

    // Check for potential overflow before memmove if the new value is larger
    if (len_diff > 0) {
        size_t current_total_xml_len = strlen(xml_content); // Length of current XML string content
        size_t projected_new_total_xml_len = current_total_xml_len + len_diff;

        if ((projected_new_total_xml_len + 1) > xml_buffer_total_size) {
            return -2; // Specific error code for overflow
        }
    }

    if (len_diff != 0) {
        char* rest_of_xml_start = value_end;
        size_t rest_len = strlen(rest_of_xml_start) + 1;
        char* dest = value_start + new_value_len;
        memmove(dest, rest_of_xml_start, rest_len);
    }

    memcpy(value_start, new_value, new_value_len);

    return 0; // Success
}
