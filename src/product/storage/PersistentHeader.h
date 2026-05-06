#pragma once

#include <stdint.h>

namespace incubator::storage
{
    struct PersistentHeader
    {
        uint32_t magic = 0x50494E43;

        uint16_t schemaVersion = 1;

        uint16_t size = 0;
    };
}