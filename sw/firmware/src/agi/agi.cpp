#include "agi.h"


namespace AGI {

Engine::Engine()
{
    flag[FLAG_LOGIC0_FIRST_TIME] = true;
    flag[FLAG_ROOM_FIRST_TIME] = true;
}

void Engine::step()
{
    flag[2] = false;
    flag[4] = false;
    //TODO: var[6] update
    //TODO: For all objects for which command animate.obj, start_update and draw were carried out, the recalculation of the direction of movement is performed.
    //TODO: call logic 0
    //update dir of motion ego to var[6]
    var[5] = 0;
    var[4] = 0;
    flag[FLAG_ROOM_FIRST_TIME] = false;
    flag[FLAG_LOGIC0_FIRST_TIME] = false; // Not in documentation, but, I think we should?
    flag[6] = false;
    flag[12] = false;
    //TODO: Update objects on screen
    //TODO: If new room, load it
}

}