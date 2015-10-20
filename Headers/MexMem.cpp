#include "MexMem.hpp"

size_t MemCounter::MemUsageLimitVal = 0xFFFFFFFFFFFFFFFF;
const size_t & MemCounter::MemUsageLimit = MemCounter::MemUsageLimitVal;
size_t MemCounter::AccountOpeningKey = 0;
size_t MemCounter::MemUsageCount = 0;