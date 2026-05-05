#pragma once
#include "domain/IncubationPlanTable.h"

namespace incubator::storage
{
    class PlanStorage
    {
    public:
        static constexpr const char* kMountPoint = "/spiffs";
        static constexpr const char* kPlanPath   = "/spiffs/plan.json";

        bool init();

        bool save(const domain::IncubationPlanTable& table);
        bool load(domain::IncubationPlanTable& table);
        bool exists() const;
        bool erase();

        bool isInitialized() const { return m_mounted; }

    private:
        bool m_mounted = false;
    };
}
