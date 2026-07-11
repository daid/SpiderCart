#include "object.h"
#include "agi.h"

namespace AGI {

void Object::update(Engine& engine)
{
    switch(motion)
    {
    case Motion::MoveTo:
        {
            auto dx = target_x - x;
            auto dy = target_y - y;
            if (dx == 0 && dy == 0) {
                engine.flag[move_finished_flag] = true;
                direction = 0;
                motion = Motion::Normal;
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
        break;
    case Motion::Wander:
        if (wander_delay == 0) {
            wander_delay = 255;
            direction = rand() % 9;
        } else {
            wander_delay--;
        }
        break;
    }
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
    if (!(flags & Object::flag_fix_loop)) {
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
    if ((flags & Object::flag_cycling) && cycle_time) {
        cycle_counter += 1;
        if (cycle_counter >= cycle_time) {
            cycle_counter = 0;
            switch(cycle_mode) {
            case CycleMode::Normal:
                cel = (cel + 1) % engine.res_view[view]->celCount(loop);
                break;
            case CycleMode::NormalOnce:
                if (cel < engine.res_view[view]->celCount(loop) - 1) {
                    cel += 1;
                }
                if (cel == engine.res_view[view]->celCount(loop) - 1) {
                    flags &=~Object::flag_cycling;
                    direction = 0;
                    cycle_mode = CycleMode::Normal;
                    engine.flag[cycle_finished_flag] = true;
                }
                break;
            case CycleMode::Reverse:
                if (cel == 0)
                    cel = engine.res_view[view]->celCount(loop) - 1;
                else
                    cel -= 1;
                break;
            case CycleMode::ReverseOnce:
                if (cel > 0)
                    cel -= 1;
                if (cel == 0) {
                    flags &=~Object::flag_cycling;
                    direction = 0;
                    cycle_mode = CycleMode::Normal;
                    engine.flag[cycle_finished_flag] = true;
                }
                break;
            }
        }
    }
    if (!(flags & Object::flag_fixed_priority)) {
        if (y < 48) priority = 4;
        else if (y < 60) priority = 5;
        else if (y < 72) priority = 6;
        else if (y < 84) priority = 7;
        else if (y < 96) priority = 8;
        else if (y < 108) priority = 9;
        else if (y < 120) priority = 10;
        else if (y < 132) priority = 11;
        else if (y < 144) priority = 12;
        else if (y < 156) priority = 13;
        else if (y < 168) priority = 14;
        else priority = 15;
    }
}

void Object::setView(int new_view)
{
    if (view == new_view) return;
    view = new_view;
    loop = 0;
    cel = 0;
    cycle_counter = 0;
}

void Object::setLoop(int new_loop)
{
    if (loop == new_loop) return;
    loop = new_loop;
    cel = 0;
    cycle_counter = 0;
}

void Object::fixPosition(Engine& engine)
{
    //TODO: Ensure object is within screen bounds, and move out of other objects and "priority" collision
}

void Object::animate()
{
    if (flags & Object::flag_anim) return;
    flags |= Object::flag_anim;
    flags |= Object::flag_update;
    flags |= Object::flag_cycling;
    motion = Motion::Normal;
    cycle_mode = CycleMode::Normal;
    direction = 0;
}

void Object::move_to(uint8_t target_x, uint8_t target_y, uint8_t step_size, uint8_t finished_flag)
{
    this->target_x = target_x;
    this->target_y = target_y;
    this->step_size = step_size;
    this->move_finished_flag = finished_flag;
    flags |= Object::flag_update; 
    motion = Motion::MoveTo;
}

void Object::stop_motion()
{
}

bool Object::inBox(uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1)
{
    return false;
}

}