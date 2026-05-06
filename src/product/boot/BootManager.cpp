#include "BootManager.h"

namespace incubator::boot
{
    using namespace incubator::domain;

    BootManager::BootManager(
        RuntimeState& runtime,
        RecoveryState& recovery)
        :
        m_runtime(runtime),
        m_recovery(recovery)
    {
    }

    bool BootManager::boot()
    {
        if (!initializeStorage())
        {
            enterSafeBoot();

            return false;
        }

        if (!restoreSettings())
        {
            enterSafeBoot();

            return false;
        }

        if (!initializeDevices())
        {
            enterSafeBoot();

            return false;
        }

        if (!validateSystem())
        {
            enterSafeBoot();

            return false;
        }

        return true;
    }

    bool BootManager::initializeStorage()
    {
        return true;
    }

    bool BootManager::restoreSettings()
    {
        return true;
    }

    bool BootManager::initializeDevices()
    {
        return true;
    }

    bool BootManager::validateSystem()
    {
        return true;
    }

    void BootManager::enterSafeBoot()
    {
        m_runtime.safeMode = true;

        m_recovery.safeModeReason =
            SafeModeReason::StorageFail;
    }
}