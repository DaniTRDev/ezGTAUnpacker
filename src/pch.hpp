#pragma once

#define TEST

//WINDOWS INCLUDES
#include <Windows.h>
#include <iostream>
#include <math.h>
#include <vector>
#include <map>
#include <functional>
#include <fstream>
#include <set>
#include <string_view>
#include <memory>
#include <format>
#include <filesystem>
#include <mutex>

//EXTERNAL LIBRARIES
#include "Pe/Pe.hpp"
#include "asmjit/asmjit.h"
#include "asmjit/x86.h"
#include "nlohmann/json.hpp"

//PROJECT FILES
#include "smartBuffer/smartBuffer.hpp"
#include "utils.hpp"
#include "patch/patch.hpp"

#include "SDK/SDK.hpp"

#include "unpacker/unpacker.hpp"

inline bool g_dumpInfo = false;