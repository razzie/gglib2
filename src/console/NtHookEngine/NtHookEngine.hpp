/**
 * Copyright (c) 2014-2016 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

/**
 * Original code: Daniel Pistelli
 * http://www.codeproject.com/Articles/21414/Powerful-x86-x64-Mini-Hook-Engine
 */

#ifndef NTHOOKENGINE_HPP
#define NTHOOKENGINE_HPP

#include <windows.h>

void initNtHookEngine();
BOOL HookFunction(ULONG_PTR OriginalFunction, ULONG_PTR NewFunction);
BOOL HookVtableFunction(VOID* pObj, ULONG_PTR NewFunction, unsigned VtableIndex);
VOID UnhookFunction(ULONG_PTR Function);
ULONG_PTR GetOriginalFunction(ULONG_PTR Hook);

#endif NTHOOKENGINE_HPP
