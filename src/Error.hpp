#pragma once
#include <stdio.h>

#define LOG_ERR(msg, ...) printf(msg "\n", __VA_ARGS__)
