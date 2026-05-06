#include "UiNavigator.h"

namespace incubator::ui
{
    UiNavigator::UiNavigator(
        UiStateMachine& state)
        :
        m_state(state)
    {
    }

    void UiNavigator::process(
        const InputEvent& event)
    {
        switch (event.type)
        {
            case InputEventType::RotateRight:
                navigateNext();
                break;

            case InputEventType::RotateLeft:
                navigatePrev();
                break;

            case InputEventType::Click:
                select();
                break;

            case InputEventType::LongClick:
                back();
                break;

            default:
                break;
        }
    }

    void UiNavigator::navigateNext()
    {
        switch (m_state.currentPage)
        {
            case UiPage::Home:
                m_state.currentPage =
                    UiPage::Progress;
                break;

            case UiPage::Progress:
                m_state.currentPage =
                    UiPage::Manual;
                break;

            case UiPage::Manual:
                m_state.currentPage =
                    UiPage::PlanEdit;
                break;

            case UiPage::PlanEdit:
                m_state.currentPage =
                    UiPage::System;
                break;

            default:
                break;
        }
    }

    void UiNavigator::navigatePrev()
    {
    }

    void UiNavigator::select()
    {
    }

    void UiNavigator::back()
    {
        m_state.currentOverlay =
            UiOverlay::None;
    }
}