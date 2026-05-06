#pragma once

namespace incubator::ui
{
    class HoldConfirmDialog
    {
    public:
        void open();

        void close();

        bool update(
            bool holding,
            uint32_t elapsedMs);

    private:
        bool m_opened = false;

        static constexpr uint32_t HoldMs =
            3000;
    };
}