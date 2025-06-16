#include "sysprod.h"
#include "setup.h" // For iosAlloc/Free magic numbers, if used by allocIobuf
#include <string.h> // For strstr, strlen, memcpy, memmove, memset
#include <stdio.h>  // For snprintf, NULL
#include <stddef.h> // For ptrdiff_t, size_t
#include <wafel/services/fsa.h> // For FSA_OpenFile, ReadFile, etc.
#include <wafel/ios/svc.h>      // For iovec_s, iosIoctlv, iosAlloc, iosFree

// Define a reasonable max size for the XML file.
#define SYS_PROD_XML_MAX_SIZE (4 * 1024) // 4KB

// Note: Removed static int fsaFd, static int init_fsa(), static void cleanup_fsa()
// and their forward declarations.

// Helper functions for memory allocation, moved to the top.
// These are static, so their definitions appearing before use by other functions
// in this file means forward declarations are not strictly needed within this file.
static void* allocIobuf(size_t size)
{
    void* ptr = iosAlloc(0xCAFF, size); // 0xCAFF is likely a heap ID for IOS
    if (ptr) {
        memset(ptr, 0x00, size);
    }
    return ptr;
}

static void freeIobuf(void* ptr)
{
	if (ptr) {
	    iosFree(0xCAFF, ptr);
	}
}

// This function is now public (declaration in sysprod.h) and takes fsa_handle.
int modify_sys_prod_xml(int fsa_handle, int product_area_val, int game_region_val) {
    // Check if the provided FSA handle is valid.
    if (fsa_handle < 0) {
        // printf("modify_sys_prod_xml: Invalid FSA handle provided (%d).\n", fsa_handle);
        return -1; // Or a more specific error for invalid handle
    }

    const char* xml_path = "/vol/system/sys/config/sys_prod.xml";
    int file_handle = -1; // For FSA_OpenFile
    char* xml_buffer = NULL;
    int result = -1; // Default to error status

    xml_buffer = (char*)allocIobuf(SYS_PROD_XML_MAX_SIZE);
    if (!xml_buffer) {
        // printf("modify_sys_prod_xml: Failed to allocate buffer for XML content.\n");
        return -2; // Allocation error
    }

    // 1. Open file for reading using the provided fsa_handle
    if (FSA_OpenFile(fsa_handle, xml_path, "r", &file_handle) < 0) {
        // printf("modify_sys_prod_xml: Failed to open XML file for reading: %s\n", xml_path);
        freeIobuf(xml_buffer);
        return -3; // Open for read error
    }

    // 2. Read file content
    int bytes_read = FSA_ReadFile(fsa_handle, xml_buffer, 1, SYS_PROD_XML_MAX_SIZE - 1, file_handle, 0);
    FSA_CloseFile(fsa_handle, file_handle);
    file_handle = -1;

    if (bytes_read < 0) {
        // printf("modify_sys_prod_xml: Failed to read XML file.\n");
        freeIobuf(xml_buffer);
        return -4; // Read error
    }
    if (bytes_read == 0) {
        // printf("modify_sys_prod_xml: XML file is empty.\n");
        freeIobuf(xml_buffer);
        return -4; // Read error or empty
    }
    if (bytes_read == (SYS_PROD_XML_MAX_SIZE - 1)) {
        // printf("modify_sys_prod_xml: Error: XML file content is too large for the buffer (read %d bytes, buffer size %d).\n", bytes_read, SYS_PROD_XML_MAX_SIZE);
        freeIobuf(xml_buffer);
        return -9; // Error code for file potentially too large
    }
    xml_buffer[bytes_read] = '\0'; // Safe to null-terminate

    // 3. Validate and Set Product Area
    char current_pa_value[16];
    if (get_tag_string(xml_buffer, "product_area", current_pa_value, sizeof(current_pa_value)) == NULL) {
        // Failed to read current product_area or buffer too small
        // printf("modify_sys_prod_xml: Failed to get current product_area.\n");
        freeIobuf(xml_buffer);
        return -10; // Error getting current product_area
    }
    if (strcmp(current_pa_value, "1") != 0 && strcmp(current_pa_value, "2") != 0 &&
        strcmp(current_pa_value, "4") != 0 && strcmp(current_pa_value, "119") != 0) {
        // printf("modify_sys_prod_xml: Current product_area ('%s') is not an allowed value for modification.\n", current_pa_value);
        freeIobuf(xml_buffer);
        return -11; // Invalid current product_area value
    }

    char pa_str[12]; // Buffer for the new product_area string
    snprintf(pa_str, sizeof(pa_str), "%d", product_area_val); // product_area_val is the new value
    // Update call to set_tag_string with xml_buffer_total_size
    if (set_tag_string(xml_buffer, SYS_PROD_XML_MAX_SIZE, "product_area", pa_str) != 0) {
        // printf("modify_sys_prod_xml: Failed to set product_area in XML buffer.\n");
        freeIobuf(xml_buffer);
        return -5; // Set product_area error (original error code)
    }

    // 4. Validate and Set Game Region
    char current_gr_value[16];
    if (get_tag_string(xml_buffer, "game_region", current_gr_value, sizeof(current_gr_value)) == NULL) {
        // printf("modify_sys_prod_xml: Failed to get current game_region.\n");
        freeIobuf(xml_buffer);
        return -12; // Error getting current game_region
    }
    if (strcmp(current_gr_value, "1") != 0 && strcmp(current_gr_value, "2") != 0 &&
        strcmp(current_gr_value, "4") != 0 && strcmp(current_gr_value, "119") != 0) {
        // printf("modify_sys_prod_xml: Current game_region ('%s') is not an allowed value for modification.\n", current_gr_value);
        freeIobuf(xml_buffer);
        return -13; // Invalid current game_region value
    }

    char gr_str[12]; // Buffer for the new game_region string
    snprintf(gr_str, sizeof(gr_str), "%d", game_region_val); // game_region_val is the new value
    // Update call to set_tag_string with xml_buffer_total_size
    if (set_tag_string(xml_buffer, SYS_PROD_XML_MAX_SIZE, "game_region", gr_str) != 0) {
        // printf("modify_sys_prod_xml: Failed to set game_region in XML buffer.\n");
        freeIobuf(xml_buffer);
        return -6; // Set game_region error (original error code)
    }

    // 5. Open file for writing (this will truncate/overwrite)
    if (FSA_OpenFile(fsa_handle, xml_path, "w", &file_handle) < 0) {
        freeIobuf(xml_buffer);
        return -7;
    }

    // 6. Write modified buffer back to file
    size_t new_xml_len = strlen(xml_buffer);
    int items_written = FSA_WriteFile(fsa_handle, xml_buffer, 1, (u32)new_xml_len, file_handle, 0);

    FSA_FlushFile(fsa_handle, file_handle);
    FSA_CloseFile(fsa_handle, file_handle);
    file_handle = -1;

    if (items_written <= 0 || (size_t)items_written != new_xml_len) {
        result = -8; // Write error
    } else {
        result = 0; // Success!
    }

    freeIobuf(xml_buffer);
    return result;
}

// MCP functions - MCP_SetSysProdSettings is reverted to its original form
int MCP_GetSysProdSettings(int fd, MCPSysProdSettings* out_sysProdSettings)
{
    uint8_t* buf = allocIobuf(sizeof(iovec_s) + sizeof(*out_sysProdSettings));
    if (!buf) {
        return -1;
    }

    iovec_s* vecs = (iovec_s*)buf;
    vecs[0].ptr = buf + sizeof(iovec_s);
    vecs[0].len = sizeof(*out_sysProdSettings);

    int res = iosIoctlv(fd, 0x40, 0, 1, vecs);
    if (res >= 0) {
        if (vecs[0].ptr && out_sysProdSettings) {
            memcpy(out_sysProdSettings, vecs[0].ptr, sizeof(*out_sysProdSettings));
        } else {
            res = -1;
        }
    }
    freeIobuf(buf);
    return res;
}

// Reverted MCP_SetSysProdSettings to its original implementation
int MCP_SetSysProdSettings(int fd, const MCPSysProdSettings* sysProdSettings)
{
    uint8_t* buf = allocIobuf(sizeof(iovec_s) + sizeof(*sysProdSettings));
    if (!buf) {
        return -1;
    }

    memcpy(&buf[sizeof(iovec_s)], sysProdSettings, sizeof(*sysProdSettings));

    iovec_s* vecs = (iovec_s*)buf;
    vecs[0].ptr = buf + sizeof(iovec_s);
    vecs[0].len = sizeof(*sysProdSettings);

    int res = iosIoctlv(fd, 0x41, 1, 0, vecs);
    freeIobuf(buf); // Original position of freeIobuf
    return res;
}

// XML helper functions (find_tag_value_start, get_tag_string, set_tag_string)
// These are public, declared in sysprod.h
const char* find_tag_value_start(const char* xml_content, const char* tag_name) {
    if (!xml_content || !tag_name) {
        return NULL;
    }
    char open_tag[128];
    int written = snprintf(open_tag, sizeof(open_tag), "<%s>", tag_name);
    if (written < 0 || (size_t)written >= sizeof(open_tag)) {
        return NULL;
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
    buffer[0] = '\0';
    const char* value_start = find_tag_value_start(xml_content, tag_name);
    if (!value_start) {
        return NULL;
    }
    char close_tag[128];
    int written = snprintf(close_tag, sizeof(close_tag), "</%s>", tag_name);
    if (written < 0 || (size_t)written >= sizeof(close_tag)) {
        return NULL;
    }
    const char* value_end = strstr(value_start, close_tag);
    if (!value_end) {
        return NULL;
    }
    if (value_start == value_end) {
        return buffer;
    }
    ptrdiff_t value_len = value_end - value_start;
    if (value_len < 0) { // Should not happen with valid XML and strstr logic
        // buffer[0] = '\0'; // Already done at the beginning of the function
        return NULL;
    }

    // Check if the value (excluding null terminator) can fit in the buffer.
    // buffer_size includes space for the null terminator.
    // So, value_len must be less than buffer_size.
    if (value_len >= buffer_size) {
        // Value is too large for the provided buffer.
        // As per new requirement, return NULL instead of truncating.
        // The initial buffer[0] = '\0' at the function start ensures it's a safe empty string.
        return NULL;
    }

    // If we reach here, value_len < buffer_size, so it fits.
    memcpy(buffer, value_start, value_len);
    buffer[value_len] = '\0';
    return buffer;
}

// Note: The call sites for this function will need to be updated to pass xml_buffer_total_size.
// That change is part of a subsequent subtask.
int set_tag_string(char* xml_content, size_t xml_buffer_total_size, const char* tag_name, const char* new_value) {
    if (!xml_content || xml_buffer_total_size == 0 || !tag_name || !new_value) {
        return -1; // Invalid parameters
    }

    // Cast needed as find_tag_value_start expects const char*
    // but we need a mutable pointer into xml_content if we are to modify it.
    // The actual modification happens after finding all necessary points.
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

        // The projected length is the string length. Add 1 for the null terminator.
        // This sum must be <= xml_buffer_total_size.
        if ((projected_new_total_xml_len + 1) > xml_buffer_total_size) {
            // Not enough space in the buffer for the expanded XML + null terminator.
            // printf("set_tag_string: Error - new value would overflow XML buffer.\n");
            // printf("Current len: %zu, diff: %td, projected len+null: %zu, buffer total size: %zu\n",
            //        current_total_xml_len, len_diff, projected_new_total_xml_len + 1, xml_buffer_total_size);
            return -2; // Specific error code for overflow
        }
    }

    // If len_diff is not 0 (can be positive or negative), shift the rest of the XML content
    if (len_diff != 0) {
        char* rest_of_xml_start = value_end; // This is the start of "</tag_name>"
        size_t rest_len = strlen(rest_of_xml_start) + 1; // Include null terminator in length to move

        char* dest = value_start + new_value_len; // Point after where the new value will be
        memmove(dest, rest_of_xml_start, rest_len);
    }

    // Copy the new value into place
    memcpy(value_start, new_value, new_value_len);

    return 0; // Success
}
