#include "xml_utils.h" // For function declarations and error codes
#include <string.h>    // For strlen, strstr, memcpy, memmove
#include <stdio.h>     // For snprintf
#include <stddef.h>    // For ptrdiff_t, size_t
#include <ctype.h>     // For isspace

const char* find_tag_value_start(const char* xml_content, const char* tag_name) {
    if (!xml_content || !tag_name || *tag_name == '\0') { // Empty tag name is invalid
        return NULL;
    }

    char open_tag_prefix[128];
    // Construct just the "<tag_name" part. Max tag name length around 120 for this buffer size.
    int written = snprintf(open_tag_prefix, sizeof(open_tag_prefix), "<%s", tag_name);
    // Ensure snprintf succeeded and the full prefix fit.
    // written should be exactly strlen("<") + strlen(tag_name).
    // If written is too large, it means tag_name was too long for open_tag_prefix buffer.
    if (written < 0 || (size_t)written >= sizeof(open_tag_prefix)) {
        return NULL; // Tag name too long for our internal buffer or snprintf error
    }

    const char* current_pos = xml_content;
    while ((current_pos = strstr(current_pos, open_tag_prefix)) != NULL) {
        const char* after_prefix = current_pos + written; // Point right after the constructed "<tag_name"

        // Check character immediately after "<tag_name"
        // It must be whitespace, '>', or '/' (for self-closing tags like <tag/> or <tag /> )
        // This check prevents matching "<tag" in "<tag_other>" or "<tagAndMore>".
        if (*after_prefix != '>' && !isspace((unsigned char)*after_prefix) && *after_prefix != '/') {
             current_pos = after_prefix; // Advance search position past this partial match
             continue;
        }

        // We've found "<tag_name" followed by a valid character (space, '>', or '/').
        // Now, scan forward from this point to find the closing '>' of this opening tag.
        const char* tag_end_char = strchr(after_prefix, '>');
        if (tag_end_char) {
            // We found the end of the opening tag. The value starts right after this '>'.
            return tag_end_char + 1;
        } else {
            // Malformed XML: opening tag part found (e.g. "<tag ") but no subsequent '>'
            return NULL;
        }
    }

    return NULL; // Tag prefix not found at all after scanning the whole content
}

const char* find_closing_tag_pos(const char* search_start_pos, const char* tag_name) {
    if (!search_start_pos || !tag_name || *tag_name == '\0') {
        return NULL; // Invalid parameters
    }

    char closing_tag_str[128]; // Buffer for "</tag_name>"
    // Max tag_name length for this buffer: sizeof(closing_tag_str) - strlen("</>") - 1 (for null terminator)
    // e.g., 128 - 3 - 1 = 124 characters for tag_name
    int written = snprintf(closing_tag_str, sizeof(closing_tag_str), "</%s>", tag_name);

    if (written < 0 || (size_t)written >= sizeof(closing_tag_str)) {
        // Error in constructing closing_tag_str (e.g., tag_name too long or snprintf error)
        return NULL;
    }

    return strstr(search_start_pos, closing_tag_str);
}

int get_tag_string(const char* xml_content, const char* tag_name, char* buffer, int buffer_size) {
    if (!xml_content || !tag_name || !buffer || buffer_size <= 0) {
        if (buffer && buffer_size > 0) {
             buffer[0] = '\0';
        }
        return XML_UTIL_ERROR_BAD_PARAMS;
    }
    buffer[0] = '\0';

    const char* value_start = find_tag_value_start(xml_content, tag_name);
    if (!value_start) {
        return XML_UTIL_ERROR_TAG_NOT_FOUND;
    }

    // Use the new helper function to find the closing tag position
    const char* value_end = find_closing_tag_pos(value_start, tag_name);
    if (!value_end) {
        // If find_closing_tag_pos returns NULL, it could be due to:
        // 1. Closing tag genuinely not found (malformed XML from this point).
        // 2. tag_name was invalid for snprintf inside find_closing_tag_pos (internal error).
        // Given find_tag_value_start succeeded, tag_name was valid for its snprintf.
        // So, this is more likely malformed XML or the closing tag is missing.
        return XML_UTIL_ERROR_MALFORMED_XML;
    }

    if (value_start == value_end) { // Should not happen if value_end points to '<' of closing tag
                                   // and value_start points after '>' of opening tag.
                                   // This condition implies an empty value if value_end pointed *after* closing tag.
                                   // However, find_closing_tag_pos returns start of "</tag_name>".
                                   // If value is truly empty, like <tag></tag>, value_start points to first '<' of closing.
                                   // So value_start == value_end means empty.
        return XML_UTIL_SUCCESS; // buffer is already '\0' terminated and empty
    }

    ptrdiff_t value_len = value_end - value_start;
    if (value_len < 0) {
        return XML_UTIL_ERROR_INTERNAL;
    }

    if (value_len >= buffer_size) {
        return XML_UTIL_ERROR_BUFFER_TOO_SMALL;
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
        return XML_UTIL_ERROR_TAG_NOT_FOUND;
    }
    char* value_start = xml_content + (value_start_const - xml_content);

    // Use the new helper function to find the closing tag position
    char* value_end = (char*)find_closing_tag_pos(value_start, tag_name);
    if (!value_end) {
        // As in get_tag_string, this implies malformed XML or internal error from find_closing_tag_pos.
        return XML_UTIL_ERROR_MALFORMED_XML;
    }

    size_t old_value_len = value_end - value_start;
    size_t new_value_len = strlen(new_value);
    ptrdiff_t len_diff = new_value_len - old_value_len;

    if (len_diff > 0) {
        size_t current_total_xml_len = strlen(xml_content);
        size_t projected_new_total_xml_len = current_total_xml_len + len_diff;
        if ((projected_new_total_xml_len + 1) > xml_buffer_total_size) {
            return -2; // Specific overflow error for set_tag_string
        }
    }

    if (len_diff != 0) {
        // value_end points to the '<' of "</tag_name>". We need to move this entire closing tag.
        size_t closing_tag_len = strlen(value_end); // This will be strlen("</tag_name>")
        memmove(value_start + new_value_len, value_end, closing_tag_len + 1); // +1 for null terminator
    }
    memcpy(value_start, new_value, new_value_len);
    return XML_UTIL_SUCCESS;
}
