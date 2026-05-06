#pragma once

#include "../domain/RuntimeState.h"
#include "../domain/RecoveryState.h"

namespace incubator::boot
{
    class BootManager
    {
    public:
        BootManager(
            incubator::domain::RuntimeState& runtime,
            incubator::domain::RecoveryState& recovery);

    public:
        bool boot();

    private:
        bool initializeStorage();

        bool restoreSettings();

        bool initializeDevices();

        bool validateSystem();

        void enterSafeBoot();

    private:
        incubator::domain::RuntimeState& m_runtime;

        incubator::domain::RecoveryState& m_recovery;
    };
}