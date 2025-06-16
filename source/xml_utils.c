#include "xml_utils.h" // For function declarations and error codes
#include <string.h>    // For strlen, strstr, memcpy, memmove
#include <stdio.h>     // For snprintf
#include <stddef.h>    // For ptrdiff_t, size_t

const char* find_tag_value_start(const char* xml_content, const char* tag_name) {
    if (!xml_content || !tag_name) {
        // This function returns NULL on bad params or if tag construction fails.
        // The caller (e.g., get_tag_string) should interpret this.
        return NULL;
    }
    char open_tag[128]; // Assuming tag names are reasonably short for this buffer
    int written = snprintf(open_tag, sizeof(open_tag), "<%s>", tag_name);

    // If snprintf fails (returns negative) or truncates (written >= sizeof(open_tag)),
    // open_tag is not reliable or not correctly formed.
    if (written < 0 || (size_t)written >= sizeof(open_tag)) {
        return NULL; // Cannot construct tag, so effectively "tag not found" or "bad tag_name"
    }

    const char* tag_start = strstr(xml_content, open_tag);
    if (tag_start) {
        // strlen(open_tag) is safe because if written >= sizeof(open_tag), we'd have returned NULL.
        // If written is valid, open_tag is null-terminated by snprintf within its bounds.
        return tag_start + strlen(open_tag);
    }
    return NULL; // Tag not found
}

int get_tag_string(const char* xml_content, const char* tag_name, char* buffer, int buffer_size) {
    if (!xml_content || !tag_name || !buffer || buffer_size <= 0) {
        if (buffer && buffer_size > 0) { // Check if buffer is somewhat valid before writing
             buffer[0] = '\0';
        }
        return XML_UTIL_ERROR_BAD_PARAMS;
    }
    buffer[0] = '\0'; // Default to empty string for safety on subsequent errors

    const char* value_start = find_tag_value_start(xml_content, tag_name);
    if (!value_start) {
        // This can be due to tag_name being invalid for snprintf in find_tag_value_start,
        // or the tag simply not being present.
        return XML_UTIL_ERROR_TAG_NOT_FOUND;
    }

    char close_tag[128];
    int written = snprintf(close_tag, sizeof(close_tag), "</%s>", tag_name);
    if (written < 0 || (size_t)written >= sizeof(close_tag)) {
        return XML_UTIL_ERROR_INTERNAL; // Error constructing close_tag
    }

    const char* value_end = strstr(value_start, close_tag);
    if (!value_end) {
        return XML_UTIL_ERROR_MALFORMED_XML; // Closing tag not found
    }

    if (value_start == value_end) { // Empty tag value like <tag></tag>
        return XML_UTIL_SUCCESS; // buffer is already '\0' terminated and empty
    }

    ptrdiff_t value_len = value_end - value_start;
    if (value_len < 0) {
        // This case should ideally not be reached if strstr and pointer logic is correct.
        return XML_UTIL_ERROR_INTERNAL;
    }

    if (value_len >= buffer_size) {
        return XML_UTIL_ERROR_BUFFER_TOO_SMALL; // Buffer (for caller) is too small
    }

    memcpy(buffer, value_start, value_len);
    buffer[value_len] = '\0';
    return XML_UTIL_SUCCESS;
}

int set_tag_string(char* xml_content, size_t xml_buffer_total_size, const char* tag_name, const char* new_value) {
    if (!xml_content || xml_buffer_total_size == 0 || !tag_name || !new_value) {
        return XML_UTIL_ERROR_BAD_PARAMS;
    }

    const char* value_start_const = find_tag_value_start(xml_content, tag_name);
    if (!value_start_const) {
        // Failure in find_tag_value_start (e.g. bad tag_name for snprintf, or tag not in xml_content)
        return XML_UTIL_ERROR_TAG_NOT_FOUND;
    }
    char* value_start = xml_content + (value_start_const - xml_content);

    char close_tag[128];
    int written = snprintf(close_tag, sizeof(close_tag), "</%s>", tag_name);
    if (written < 0 || (size_t)written >= sizeof(close_tag)) {
        return XML_UTIL_ERROR_INTERNAL; // Error constructing close_tag
    }

    char* value_end = strstr(value_start, close_tag);
    if (!value_end) {
        return XML_UTIL_ERROR_MALFORMED_XML; // Closing tag for value not found
    }

    size_t old_value_len = value_end - value_start;
    size_t new_value_len = strlen(new_value);
    ptrdiff_t len_diff = new_value_len - old_value_len;

    if (len_diff > 0) {
        size_t current_total_xml_len = strlen(xml_content);
        size_t projected_new_total_xml_len = current_total_xml_len + len_diff;
        if ((projected_new_total_xml_len + 1) > xml_buffer_total_size) {
            // This is a specific error code for set_tag_string, indicating the overall XML buffer would overflow.
            // It's different from XML_UTIL_ERROR_BUFFER_TOO_SMALL which refers to the output buffer of get_tag_string.
            return -2;
        }
    }

    if (len_diff != 0) {
        char* rest_of_xml_start = value_end;
        size_t rest_len = strlen(rest_of_xml_start) + 1;
        memmove(value_start + new_value_len, rest_of_xml_start, rest_len);
    }
    memcpy(value_start, new_value, new_value_len);
    return XML_UTIL_SUCCESS;
}
