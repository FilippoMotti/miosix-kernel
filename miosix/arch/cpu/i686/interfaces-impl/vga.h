#pragma once
#include "stdint.h"

namespace miosix {

#define COLOR8_BLACK 0
#define COLOR8_LIGHT_GREY 7

#define WIDTH 80
#define HEIGHT 25

extern "C" {
void print(char s);
void scrollUp();
void newLine();
void Reset();
void hexToStr(unsigned int value, char *buf);
}
} // namespace miosix
