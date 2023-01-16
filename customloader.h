#include <stdio.h>
#define CINTERFACE
#define COBJMACROS

#include <metahost.h>
#include <Unknwnbase.h>

#pragma comment(lib, "mscoree.lib")
#pragma once

void CleanUp();
HRESULT CreateCLRInterface();
HRESULT StartRuntime();
HRESULT StopRuntime();
