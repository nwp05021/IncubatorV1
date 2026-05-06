#include "InputRuntimeTask.h"

#include <Arduino.h>

namespace incubator::input
{
    using incubator::app::Command;
    using incubator::app::CommandSource;
    using incubator::app::CommandType;

    InputRuntimeTask::InputRuntimeTask(
        IEc11Device& device,
        InputEventQueue& inputQueue,
        incubator::app::CommandQueue& commandQueue)
        :
        m_device(device),
        m_inputQueue(inputQueue),
        m_commandQueue(commandQueue)
    {
    }

    void InputRuntimeTask::begin()
    {
        m_device.begin();
    }

    void InputRuntimeTask::tick(uint32_t nowMs)
    {
        InputEvent event;

        if (m_device.poll(nowMs, event))
        {
            m_inputQueue.enqueue(event);
        }

        while (m_inputQueue.dequeue(event))
        {
            publishCommand(event);
            printDiagnostics(event);
        }
    }

    void InputRuntimeTask::setDiagnosticsEnabled(bool enabled)
    {
        m_diagnosticsEnabled = enabled;
    }

    void InputRuntimeTask::publishCommand(const InputEvent& event)
    {
        Command cmd;
        cmd.source = CommandSource::UI;

        switch (event.type)
        {
            case InputEventType::RotateLeft:
                cmd.type = CommandType::UiRotateLeft;
                cmd.setUInt.value = 1;
                break;

            case InputEventType::RotateRight:
                cmd.type = CommandType::UiRotateRight;
                cmd.setUInt.value = 1;
                break;

            case InputEventType::ButtonClick:
                cmd.type = CommandType::UiClick;
                cmd.setUInt.value = 1;
                break;

            case InputEventType::ButtonDown:
                cmd.type = CommandType::UiButtonDown;
                cmd.setUInt.value = 1;
                break;

            case InputEventType::ButtonUp:
                cmd.type = CommandType::UiButtonUp;
                cmd.setUInt.value = 1;
                break;

            default:
                return;
        }

        m_commandQueue.enqueue(cmd);
    }

    void InputRuntimeTask::printDiagnostics(const InputEvent& event) const
    {
        if (!m_diagnosticsEnabled)
        {
            return;
        }

        const char* name = "None";

        switch (event.type)
        {
            case InputEventType::RotateLeft:
                name = "RotateLeft";
                break;

            case InputEventType::RotateRight:
                name = "RotateRight";
                break;

            case InputEventType::ButtonClick:
                name = "ButtonClick";
                break;

            case InputEventType::ButtonDown:
                name = "ButtonDown";
                break;

            case InputEventType::ButtonUp:
                name = "ButtonUp";
                break;

            default:
                break;
        }

        Serial.printf(
            "[INPUT] EC11 %s delta=%d at=%lu\n",
            name,
            static_cast<int>(event.delta),
            static_cast<unsigned long>(event.timestampMs));
    }
}
