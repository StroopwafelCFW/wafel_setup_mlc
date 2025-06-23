#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <wafel/dynamic.h>
#include <wafel/ios_dynamic.h>
#include <wafel/utils.h>
#include <wafel/patch.h>
#include <wafel/ios/svc.h>

#include "setup.h"

// This fn runs before everything else in kernel mode.
// It should be used to do extremely early patches
// (ie to BSP and kernel, which launches before MCP)
// It jumps to the real IOS kernel entry on exit.
__attribute__((target("arm")))
void kern_main()
{
    // Make sure relocs worked fine and mappings are good
    debug_printf("we in here setup mlc plugin kern %p\n", kern_main);

    debug_printf("init_linking symbol at: %08x\n", wafel_find_symbol("init_linking"));

    // Disable panic in kernel
    ASM_PATCH_K(0x08129ce0, "bx lr\n");

    // format MLC if needed
    ASM_PATCH_K(0x05027D24, ".thumb\nnop\nnop\n");

    // Patch MCP_SetSysProdSettings debug mode check
    ASM_PATCH_K(0x05024648, ".thumb\nnop\nnop\n");

    // create all system directries if they don't exist
    ASM_T_PATCH_K(0x050155ea, "tst r2,r2\n");

    trampoline_t_hook_before(0x05027e9e, setup_hook);

    debug_printf("setup patches applied\n");
}

// This fn runs before MCP's main thread, and can be used
// to perform late patches and spawn threads under MCP.
// It must return.
void mcp_main()
{
    // Make sure relocs worked fine and mappings are good
	debug_printf("we in setup mlc MCP %p\n", mcp_main);
}
