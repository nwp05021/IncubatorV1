#include "HomeScreen.h"

namespace incubator::ui
{
    HomeScreen::HomeScreen(
        const incubator::domain::RuntimeState& runtime)
        :
        m_runtime(runtime)
    {
    }

    void HomeScreen::tick()
    {
        m_builder.build(
            m_runtime,
            m_model);

        m_renderer.render(
            m_model);
    }
}
