#include "DiagnosticUiModelBuilder.h"

#include "../../domain/RuntimeState.h"

#include "../../cloud/CloudState.h"

namespace incubator::ui
{
    using namespace incubator::domain;

    void DiagnosticUiModelBuilder::build(
        const RuntimeState& runtime,
        const incubator::cloud::CloudState& cloud,
        DiagnosticUiModel& model)
    {
        model.wifiConnected =
            cloud.wifiConnected;

        model.mqttConnected =
            cloud.mqttConnected;

        model.safeMode =
            runtime.safeMode;

        model.sensorHealthy =
            runtime.sensorHealthy;

        model.storageHealthy =
            runtime.storageHealthy;
    }
}