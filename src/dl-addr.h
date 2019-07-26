#pragma once

#include "externc.h"

EXTERNC int get_module_base_address(void *address, char const **module_filename, void **module_base_address);
EXTERNC int get_module_base_address_dl(void *handle, char const *module_filename, void **module_base_address);