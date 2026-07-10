#include "object.h"

namespace AGI {

void Object::setLoop(int new_loop)
{
    if (loop = new_loop) return;
    loop = new_loop;
    cel = 0;
}

void Object::move_to(uint8_t target_x, uint8_t target_y, uint8_t step_size, uint8_t finished_flag)
{
}

void Object::stop_motion()
{
}

}