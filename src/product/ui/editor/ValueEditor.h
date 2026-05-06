#pragma once

namespace incubator::ui
{
    class ValueEditor
    {
    public:
        float increase(
            float value,
            float step,
            float max);

        float decrease(
            float value,
            float step,
            float min);
    };
}