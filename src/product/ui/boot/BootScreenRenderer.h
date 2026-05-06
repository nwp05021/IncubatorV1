#pragma once

namespace incubator::ui
{
    class BootScreenRenderer
    {
    public:
        void showProgress(
            int percent,
            const char* message);
    };
}