#pragma once

#include "../domain/RuntimeState.h"
#include "../domain/AppSettings.h"

#include "CommandQueue.h"

namespace incubator::app
{
    class AppController
    {
    public:
        AppController(
            incubator::domain::RuntimeState& runtime,
            incubator::domain::AppSettings& settings,
            CommandQueue& queue);

    public:
        void tick();

    private:
        void processCommand(
            const Command& cmd);

        void handleSetTargetTemp(
            const Command& cmd);

        void handleSetTargetHumidity(
            const Command& cmd);

        void handleUiRotateLeft();

        void handleUiRotateRight();

        incubator::domain::UiFocusId nextFocus(
            incubator::domain::UiFocusId current) const;

        incubator::domain::UiFocusId previousFocus(
            incubator::domain::UiFocusId current) const;

    private:
        incubator::domain::RuntimeState& m_runtime;

        incubator::domain::AppSettings& m_settings;

        CommandQueue& m_queue;
    };
}