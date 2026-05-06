#pragma once

#include "../model/HomeUiModel.h"

#include "../viewmodel/HomeUiModelBuilder.h"
#include "../render/HomeRenderer.h"

namespace incubator::ui
{
    class HomeScreen
    {
    public:
        HomeScreen(
            const incubator::domain::RuntimeState& runtime);

    public:
        void tick();

    private:
        const incubator::domain::RuntimeState& m_runtime;

        HomeUiModel m_model;

        HomeUiModelBuilder m_builder;

        HomeRenderer m_renderer;
    };
}
