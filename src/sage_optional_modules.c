#include <exec/types.h>

#include <sage/sage_error.h>

/*
 * The project currently builds all SAGE modules except SAGE 3D and network.
 * Official SAGE_Init()/SAGE_Exit() still reference every optional module entry
 * point, so these temporary stubs satisfy the linker until those modules are
 * added to the build.
 */

BOOL SAGE_Init3DModule(VOID)
{
  SAGE_SetError(SERR_NOT_AVAILABLE);
  return FALSE;
}

BOOL SAGE_Release3DModule(VOID)
{
  return TRUE;
}

BOOL SAGE_InitNetworkModule(VOID)
{
  SAGE_SetError(SERR_NOT_AVAILABLE);
  return FALSE;
}

BOOL SAGE_ReleaseNetworkModule(VOID)
{
  return TRUE;
}
