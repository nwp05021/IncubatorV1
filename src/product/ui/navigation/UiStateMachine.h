#pragma once

namespace incubator::ui
{
    enum class UiPage
    {
        Home,
        Progress,
        Manual,
        PlanEdit,
        System
    };

    enum class UiOverlay
    {
        None,
        Dialog,
        Alarm,
        SafeMode
    };

    class UiStateMachine
    {
    public:
        UiPage currentPage =
            UiPage::Home;

        UiOverlay currentOverlay =
            UiOverlay::None;
    };
}