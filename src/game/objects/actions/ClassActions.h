#pragma once

#include "game/objects/Action.h"


namespace world
{
    namespace objects
    {
        class ObjectUpdater;

        namespace actions
        {
            class ClassAction0 : public Action
            {
            private:
                ObjectUpdater& _objUpdaterRef;
            public:
                ClassAction0(ObjectUpdater& objUpdater);
                virtual GC_ubyte run(ObjectInstanceData* obj, uint64_t* worldState, int worldWidth);
            };
        }
    }
}
