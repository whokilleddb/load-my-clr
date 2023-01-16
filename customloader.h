#include <stdio.h>
#define CINTERFACE
#define COBJMACROS

#include <metahost.h>
#include <Unknwnbase.h>

#pragma comment(lib, "mscoree.lib")
#pragma once

void CleanUp();
HRESULT GetCLRInterface();
HRESULT StartRuntime();
HRESULT RunAssembly(const LPCWSTR assembly_path, const LPCWSTR namespace_class, const LPCWSTR function_name, const LPCWSTR cmd_arguments);
HRESULT StopRuntime();
