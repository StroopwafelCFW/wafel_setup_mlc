#include "sysprod.h"
#include "setup.h" // For iosAlloc/Free magic numbers, if used by allocIobuf
#include "xml_utils.h" // Include for XML helper function declarations

#include <string.h> // For strlen, memcpy, memset, strcmp (used in modify_sys_prod_xml)
#include <stdio.h>  // For snprintf (used in modify_sys_prod_xml)
#include <stddef.h> // For ptrdiff_t, size_t (though often included by string.h/stdio.h)
#include <wafel/services/fsa.h> // For FSA_OpenFile, ReadFile, etc.
#include <wafel/ios/svc.h>      // For iovec_s, iosIoctlv, iosAlloc, iosFree

// Define a reasonable max size for the XML file.
#define SYS_PROD_XML_MAX_SIZE (4 * 1024) // 4KB

// Helper functions for memory allocation, moved to the top.
static void* allocIobuf(size_t size)
{
    void* ptr = iosAlloc(0xCAFF, size);
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
    if (fsa_handle < 0) {
        return -1;
    }

    const char* xml_path = "/vol/system/sys/config/sys_prod.xml";
    int file_handle = -1;
    char* xml_buffer = NULL;
    int result = -1;

    xml_buffer = (char*)allocIobuf(SYS_PROD_XML_MAX_SIZE);
    if (!xml_buffer) {
        return -2;
    }

    if (FSA_OpenFile(fsa_handle, xml_path, "r", &file_handle) < 0) {
        freeIobuf(xml_buffer);
        return -3;
    }

    int bytes_read = FSA_ReadFile(fsa_handle, xml_buffer, 1, SYS_PROD_XML_MAX_SIZE - 1, file_handle, 0);
    FSA_CloseFile(fsa_handle, file_handle);
    file_handle = -1;

    if (bytes_read < 0) {
        freeIobuf(xml_buffer);
        return -4;
    }
    if (bytes_read == 0) {
        freeIobuf(xml_buffer);
        return -4;
    }
    if (bytes_read == (SYS_PROD_XML_MAX_SIZE - 1)) {
        freeIobuf(xml_buffer);
        return -9;
    }
    xml_buffer[bytes_read] = '\0';

    char current_pa_value[16];
    if (get_tag_string(xml_buffer, "product_area", current_pa_value, sizeof(current_pa_value)) == NULL) {
        freeIobuf(xml_buffer);
        return -10;
    }
    if (strcmp(current_pa_value, "1") != 0 && strcmp(current_pa_value, "2") != 0 &&
        strcmp(current_pa_value, "4") != 0 && strcmp(current_pa_value, "119") != 0) {
        freeIobuf(xml_buffer);
        return -11;
    }

    char pa_str[12];
    snprintf(pa_str, sizeof(pa_str), "%d", product_area_val);
    if (set_tag_string(xml_buffer, SYS_PROD_XML_MAX_SIZE, "product_area", pa_str) != 0) {
        freeIobuf(xml_buffer);
        return -5;
    }

    char current_gr_value[16];
    if (get_tag_string(xml_buffer, "game_region", current_gr_value, sizeof(current_gr_value)) == NULL) {
        freeIobuf(xml_buffer);
        return -12;
    }
    if (strcmp(current_gr_value, "1") != 0 && strcmp(current_gr_value, "2") != 0 &&
        strcmp(current_gr_value, "4") != 0 && strcmp(current_gr_value, "119") != 0) {
        freeIobuf(xml_buffer);
        return -13;
    }

    char gr_str[12];
    snprintf(gr_str, sizeof(gr_str), "%d", game_region_val);
    if (set_tag_string(xml_buffer, SYS_PROD_XML_MAX_SIZE, "game_region", gr_str) != 0) {
        freeIobuf(xml_buffer);
        return -6;
    }

    if (FSA_OpenFile(fsa_handle, xml_path, "w", &file_handle) < 0) {
        freeIobuf(xml_buffer);
        return -7;
    }

    size_t new_xml_len = strlen(xml_buffer);
    int items_written = FSA_WriteFile(fsa_handle, xml_buffer, 1, (u32)new_xml_len, file_handle, 0);

    FSA_FlushFile(fsa_handle, file_handle);
    FSA_CloseFile(fsa_handle, file_handle);
    file_handle = -1;

    if (items_written <= 0 || (size_t)items_written != new_xml_len) {
        result = -8;
    } else {
        result = 0;
    }

    freeIobuf(xml_buffer);
    return result;
}

// MCP functions
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
    freeIobuf(buf);
    return res;
}

// Implementations for find_tag_value_start, get_tag_string, and set_tag_string
// have been moved to xml_utils.c
// They are now declared in xml_utils.h, which is included at the top of this file.
