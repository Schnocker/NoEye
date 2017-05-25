#pragma once

#define VMPROTECT 1

#include <Windows.h>
#include <iostream>
#include <map>
#include <vector>
#include <time.h>
#include <TlHelp32.h>

#if (VMPROTECT == 1)
#include <VirtualizerSDK/VirtualizerSDK.h>
#ifdef _WIN64 
#pragma comment(lib,"VirtualizerSDK64")
#elif _WIN32
#pragma comment(lib,"VirtualizerSDK32")
#endif
#else
static void VirtualizerStart() {}
static void VirtualizerEnd() {}
#endif