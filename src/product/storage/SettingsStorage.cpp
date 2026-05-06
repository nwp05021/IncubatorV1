#include "SettingsStorage.h"

namespace incubator::storage
{
    using namespace incubator::domain;

    bool SettingsStorage::begin()
    {
        return true;
    }

    bool SettingsStorage::load(
        AppSettings& settings)
    {
        // TODO:
        // NVS Read

        if (!validate(settings))
        {
            applyDefaults(settings);

            return false;
        }

        return true;
    }

    bool SettingsStorage::save(
        const AppSettings& settings)
    {
        if (!validate(settings))
        {
            return false;
        }

        // TODO:
        // NVS Write

        return true;
    }

    bool SettingsStorage::validate(
        const AppSettings& settings)
    {
        if (settings.tempHysteresis < 0.1f ||
            settings.tempHysteresis > 2.0f)
        {
            return false;
        }

        if (settings.humidityHysteresis < 1.0f ||
            settings.humidityHysteresis > 10.0f)
        {
            return false;
        }

        if (settings.telemetryIntervalMs < 10000)
        {
            return false;
        }

        return true;
    }

    void SettingsStorage::applyDefaults(
        AppSettings& settings)
    {
        settings = AppSettings();
    }
}