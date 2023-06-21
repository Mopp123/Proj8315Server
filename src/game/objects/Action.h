#pragma once

#include <cstdint>
#include "../../../Proj8315Common/src/Common.h"

#define ACTION_STATUS_PENDING 0
#define ACTION_STATUS_SUCCESS 1
#define ACTION_STATUS_FAILURE 2


namespace world
{
    namespace objects
    {
        class ObjectInstanceData;

        class Action
        {
        public:
            Action()
            {}

            virtual ~Action()
            {}

            // Performs action for a single object instance.
            // Return values:
            //	0: pending
            //	1: success
            //	2: failure
            virtual GC_ubyte run(ObjectInstanceData* obj, uint64_t* worldState, int worldWidth) = 0;
        };
    }
}
