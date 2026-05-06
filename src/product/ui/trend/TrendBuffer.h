#pragma once

#include "TrendPoint.h"

namespace incubator::ui
{
    class TrendBuffer
    {
    public:
        static constexpr uint16_t Capacity = 64;

    public:
        void push(
            const TrendPoint& point);

        uint16_t count() const;

        const TrendPoint& at(
            uint16_t index) const;

    private:
        TrendPoint m_points[Capacity];

        uint16_t m_head = 0;

        uint16_t m_count = 0;
    };
}