#include "utils.h"
#include <cmath>

double ceilTwoAfterComa(double number) {
    return std::ceil(number * 100.0) / 100.0;
}
