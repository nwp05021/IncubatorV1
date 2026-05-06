#pragma once

#include "../../domain/RuntimeState.h"

#include "../../cloud/CloudState.h"

#include "../model/DiagnosticUiModel.h"

namespace incubator::ui
{
    class DiagnosticUiModelBuilder
    {
    public:
        void build(
            const incubator::domain::RuntimeState& runtime,
            const incubator::cloud::CloudState& cloud,
            DiagnosticUiModel& model);
    };
}