#include "object.h"
#include "agi.h"

namespace AGI {

void Object::update(Engine& engine)
{
    if (motion == Motion::MoveTo) {
        auto dx = target_x - x;
        auto dy = target_y - y;
        if (dx == 0 && dy == 0) {
            engine.flag[move_finished_flag] = true;
            motion = Motion::None;
        } else if (dx == 0) {
            if (dy < 0) direction = 1; else direction = 5;
        } else if (dy == 0) {
            if (dx < 0) direction = 7; else direction = 3;
        } else {
            if (dx < 0 && dy < 0) direction = 8;
            else if (dx < 0) direction = 6;
            else if (dy < 0) direction = 2;
            else direction = 4;
        }
    }
    if (motion != Motion::None) {
        switch(direction) {
        case 1: y -= 1; break;
        case 2: x += 1; y -= 1; break;
        case 3: x += 1; break;
        case 4: x += 1; y += 1; break;
        case 5: y += 1; break;
        case 6: x -= 1; y += 1; break;
        case 7: x -= 1; break;
        case 8: x -= 1; y -= 1; break;
        }
    }
    if (flags & Object::flag_fix_loop) {
        auto loop_count = engine.res_view[view]->loopCount();
        if (loop_count >= 8) {
            loop = direction;
        } else if (loop_count >= 4) {
            switch(direction) {
            case 1: setLoop(3); break;
            case 2: setLoop(0); break;
            case 3: setLoop(0); break;
            case 4: setLoop(0); break;
            case 5: setLoop(2); break;
            case 6: setLoop(1); break;
            case 7: setLoop(1); break;
            case 8: setLoop(1); break;
            }
        } else if (loop_count >= 2) {
            switch(direction) {
            case 2: setLoop(0); break;
            case 3: setLoop(0); break;
            case 4: setLoop(0); break;
            case 6: setLoop(1); break;
            case 7: setLoop(1); break;
            case 8: setLoop(1); break;
            }
        }
    }
    if (flags & Object::flag_cycling) {
        cycle_counter += 1;
        if (cycle_counter >= cycle_time) {
            cel = (cel + 1) % engine.res_view[view]->celCount(loop);
            cycle_counter = 0;
        }
    }
}

void Object::setLoop(int new_loop)
{
    if (loop = new_loop) return;
    loop = new_loop;
    cel = 0;
    cycle_counter = 0;
}

void Object::move_to(uint8_t target_x, uint8_t target_y, uint8_t step_size, uint8_t finished_flag)
{
    this->target_x = target_x;
    this->target_y = target_y;
    this->step_size = step_size;
    this->move_finished_flag = finished_flag;
    motion = Motion::MoveTo;
}

void Object::stop_motion()
{
}

}