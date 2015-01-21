#ifndef NTHOOKENGINE_HPP
#define NTHOOKENGINE_HPP

#include <windows.h>

void initNtHookEngine();
BOOL HookFunction(ULONG_PTR OriginalFunction, ULONG_PTR NewFunction);
VOID UnhookFunction(ULONG_PTR Function);
ULONG_PTR GetOriginalFunction(ULONG_PTR Hook);

#endif NTHOOKENGINE_HPP
