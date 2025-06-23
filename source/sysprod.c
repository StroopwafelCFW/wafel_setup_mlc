#include "sysprod.h"
#include "setup.h" // For iosAlloc/Free magic numbers, if used by allocIobuf

#include <string.h> // For strlen, memcpy, memset, strcmp (used in modify_sys_prod_xml)
#include <stdio.h>  // For snprintf (used in modify_sys_prod_xml)
#include <stddef.h> // For ptrdiff_t, size_t (though often included by string.h/stdio.h)
#include <wafel/services/fsa.h> // For FSA_OpenFile, ReadFile, etc.
#include <wafel/ios/svc.h>      // For iovec_s, iosIoctlv, iosAlloc, iosFree


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
