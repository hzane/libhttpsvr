#pragma once
#include <cstdint>
#include <cassert>
#include <functional>
#include <memory>
#include <atomic>
#include <algorithm>
#include <string>

#define WINDOWS_LEAN_AND_MEAN
#define _WIN32_WINNT 0x600
#include <http.h>
#include <Windows.h>
