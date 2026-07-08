#pragma once

#include "resources.h"
#include <stdint.h>
#include <stdlib.h>

namespace AGI {

class LogicResource : public Resource {
public:
    LogicResource(uint8_t* data, size_t size);

    const char* str(int index);
private:
    size_t logic_size;
    size_t string_count;
    size_t string_table_idx;
};

}