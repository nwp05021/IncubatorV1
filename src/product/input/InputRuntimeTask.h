#pragma once

#include <stdint.h>

#include "IEc11Device.h"
#include "InputEventQueue.h"
#include "../app/CommandQueue.h"

namespace incubator::input
{
    class InputRuntimeTask
    {
    public:
        InputRuntimeTask(
            IEc11Device& device,
            InputEventQueue& inputQueue,
            incubator::app::CommandQueue& commandQueue);

    public:
        void begin();
        void tick(uint32_t nowMs);
        void setDiagnosticsEnabled(bool enabled);

    private:
        void publishCommand(const InputEvent& event);
        void printDiagnostics(const InputEvent& event) const;

    private:
        IEc11Device& m_device;
        InputEventQueue& m_inputQueue;
        incubator::app::CommandQueue& m_commandQueue;
        bool m_diagnosticsEnabled = true;
    };
}
