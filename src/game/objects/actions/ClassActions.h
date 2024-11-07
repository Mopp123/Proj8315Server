#pragma once

#include "game/objects/Action.h"


namespace world
{
    namespace objects
    {
        class ObjectManager;

        namespace actions
        {
            class ClassAction0 : public Action
            {
            private:
                ObjectManager& _objManagerRef;
            public:
                ClassAction0(ObjectManager& objManager);
                virtual GC_ubyte run(ObjectInstanceData* obj, uint64_t* worldState, int worldWidth);
            };
        }
    }
}
