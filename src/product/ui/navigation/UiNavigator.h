#pragma once

#include "../input/InputEvent.h"
#include "UiStateMachine.h"

namespace incubator::ui
{
    class UiNavigator
    {
    public:
        UiNavigator(
            UiStateMachine& state);

    public:
        void process(
            const InputEvent& event);

    private:
        void navigateNext();

        void navigatePrev();

        void select();

        void back();

    private:
        UiStateMachine& m_state;
    };
}