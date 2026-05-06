#pragma once

#include "../model/HomeUiModel.h"
#include "../dirty/HomeUiDiff.h"
#include "../render/GraphicHomeRenderer.h"

namespace incubator::ui
{
    class GraphicHomeScreen
    {
    public:
        explicit GraphicHomeScreen(
            incubator::devices::IDisplayDevice& display);

    public:
        void render(
            const HomeUiModel& current);

        void invalidate();

    private:
        HomeUiModel m_previous;

        HomeUiDiff m_diff;

        GraphicHomeRenderer m_renderer;

        bool m_firstRender = true;
    };
}
