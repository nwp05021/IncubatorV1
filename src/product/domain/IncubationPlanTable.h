#pragma once

#include "PlanRow.h"

namespace incubator::domain
{
    class IncubationPlanTable
    {
    public:
        static constexpr uint16_t MaxRows = 32;

    public:
        bool getRow(
            uint16_t day,
            PlanRow& outRow) const;

    public:
        PlanRow rows[MaxRows];

        uint16_t rowCount = 0;
    };
}