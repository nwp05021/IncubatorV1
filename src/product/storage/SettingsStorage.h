#pragma once

#include "../domain/AppSettings.h"

namespace incubator::storage
{
    class SettingsStorage
    {
    public:
        bool begin();

        bool load(
            incubator::domain::AppSettings& settings);

        bool save(
            const incubator::domain::AppSettings& settings);

    private:
        bool validate(
            const incubator::domain::AppSettings& settings);

        void applyDefaults(
            incubator::domain::AppSettings& settings);
    };
}