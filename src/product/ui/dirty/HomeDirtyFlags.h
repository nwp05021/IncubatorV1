#pragma once

namespace incubator::ui
{
    struct HomeDirtyFlags
    {
        bool statusBar = true;

        bool temperature = true;

        bool humidity = true;

        bool outputs = true;

        bool progress = true;

        bool overlay = true;

        bool any() const
        {
            return statusBar ||
                   temperature ||
                   humidity ||
                   outputs ||
                   progress ||
                   overlay;
        }

        void clear()
        {
            statusBar = false;
            temperature = false;
            humidity = false;
            outputs = false;
            progress = false;
            overlay = false;
        }

        void markAll()
        {
            statusBar = true;
            temperature = true;
            humidity = true;
            outputs = true;
            progress = true;
            overlay = true;
        }
    };
}
