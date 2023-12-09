#pragma once

#include "../../../../Proj8315Common/src/Common.h"

#include "game/objects/Action.h"
#include "game/objects/Object.h"


namespace world
{
    namespace objects
    {
        namespace actions
        {
            class Move : public Action
            {
            private:
                // Movement direction
                GC_ubyte _dir;
            public:
                Move(GC_ubyte direction);
                virtual GC_ubyte run(ObjectInstanceData* obj, uint64_t* worldState, int worldWidth);
            };

            class MoveVertical : public Action
            {
            private:
                // Movement direction
                GC_ubyte _dir;
            public:
                MoveVertical(GC_ubyte direction);
                virtual GC_ubyte run(ObjectInstanceData* obj, uint64_t* worldState, int worldWidth);
            };
        }
    }
}
