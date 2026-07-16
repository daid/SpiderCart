#include "object.h"
#include "agi.h"

namespace AGI {

static int directionOf(int dx, int dy) {
    if (dx == 0 && dy == 0) {
        return 0;
    } else if (dx == 0) {
        if (dy < 0) return 1; else return 5;
    } else if (dy == 0) {
        if (dx < 0) return 7; else return 3;
    } else {
        if (dx < 0 && dy < 0) return 8;
        else if (dx < 0) return 6;
        else if (dy < 0) return 2;
        else return 4;
    }
}

void Object::updatePhysics()
{
    auto current_step_size = step_size;
    switch(motion)
    {
    case Motion::Normal:
        break;
    case Motion::MoveTo:
        {
            auto dx = target_x - x;
            auto dy = target_y - y;
            current_step_size = step_size_move_to;
            if (abs(dx) < current_step_size) dx = 0;
            if (abs(dy) < current_step_size) dy = 0;
            direction = directionOf(dx, dy);
            if (direction == 0) {
                destinationReached();
            }
        }
        break;
    case Motion::Wander:
        if (wander_delay == 0) {
            wander_delay = (rand() % 54) + 6;
            direction = rand() % 9;
        } else {
            wander_delay--;
        }
        break;
    case Motion::FollowPlayer:
        auto dx = Engine::instance->object[0].x - x;
        auto dy = Engine::instance->object[0].y - y;
        if (abs(dx) < current_step_size) dx = 0;
        if (abs(dy) < current_step_size) dy = 0;
        direction = directionOf(dx, dy);
        //TODO: This needs more logic to handle the "getting stuck" case
        if (direction == 0) {
            destinationReached();
        }
        break;
    }
    auto view_data = Engine::instance->res_view[view];
    if (!view_data) return;
    auto view_info = view_data->info(loop, cel);
    if (step_counter > 0)
        step_counter--;
    if (step_counter == 0) {
        step_counter = step_time;
        auto oldx = x;
        auto oldy = y;
        auto pre_in_block = checkBlockCollision();
        switch(direction) {
        case 1: y -= current_step_size; break;
        case 2: x += current_step_size; y -= current_step_size; break;
        case 3: x += current_step_size; break;
        case 4: x += current_step_size; y += current_step_size; break;
        case 5: y += current_step_size; break;
        case 6: x -= current_step_size; y += current_step_size; break;
        case 7: x -= current_step_size; break;
        case 8: x -= current_step_size; y -= current_step_size; break;
        }
        //Ensure object is within screen bounds
        int border = 0;
        if (x < 0) { x = 0; border = 4; }
        if (y < view_info.height - 1) { y = view_info.height - 1; border = 1; }
        if (x > 160 - view_info.width) { x = 160 - view_info.width; border = 2; }
        if (y > 167) { y = 167; border = 3; }
        if (!(flags & flag_ignore_horizon)) {
            if (y <= Engine::instance->horizon) { y = Engine::instance->horizon + 1; border = 1; }
        }
        if (pre_in_block != checkBlockCollision() || checkObjCollision() || checkPrioCollision()) {
            x = oldx; y = oldy;
            border = 0;
            wander_delay = 0;
        }
        if (isPlayer()) {
            Engine::instance->var[Engine::VAR_PLAYER_BORDER_TOUCH] = border;
        } else if (border) {
            Engine::instance->var[Engine::VAR_OTHER_BORDER_TOUCH_OBJ] = objIndex();
            Engine::instance->var[Engine::VAR_OTHER_BORDER_TOUCH] = border;
        }
        if (border && motion == Motion::MoveTo) {
            destinationReached();
        }
        fixPosition();
    }
}

void Object::updateAnimation()
{
    auto view_data = Engine::instance->res_view[view];
    if (!view_data) return;
    if (!(flags & Object::flag_fix_loop)) {
        auto loop_count = view_data->loopCount();
        if (loop_count > 8) {
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
                cel = (cel + 1) % view_data->celCount(loop);
                break;
            case CycleMode::NormalOnce:
                if (cel < view_data->celCount(loop) - 1) {
                    cel += 1;
                }
                if (cel == view_data->celCount(loop) - 1) {
                    flags &=~Object::flag_cycling;
                    direction = 0;
                    cycle_mode = CycleMode::Normal;
                    Engine::instance->flag[cycle_finished_flag] = true;
                }
                break;
            case CycleMode::Reverse:
                if (cel == 0)
                    cel = view_data->celCount(loop) - 1;
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
                    Engine::instance->flag[cycle_finished_flag] = true;
                }
                break;
            }
        }
    }
}

void Object::destinationReached()
{
    Engine::instance->flag[move_finished_flag] = true;
    motion = Motion::Normal;
    if (isPlayer())
        Engine::instance->player_control = true;
}

bool Object::checkBoundsCollision()
{
    auto view_data = Engine::instance->res_view[view];
    if (!view_data) return false;
    auto view_info = view_data->info(loop, cel);
    if (x < 0) return true;
    if (y < view_info.height - 1) return true;
    if (x > 160 - view_info.width) return true;
    if (y > 167) return true;
    if (!(flags & flag_ignore_horizon)) {
        if (y <= Engine::instance->horizon) return true;
    }
    return false;
}

bool Object::checkBlockCollision()
{
    if (flags & flag_ignore_blocks) return false;
    if (!Engine::instance->block_active) return false;
    if (x > Engine::instance->block_x1) return false;
    if (x < Engine::instance->block_x0) return false;
    if (y > Engine::instance->block_y1) return false;
    if (y < Engine::instance->block_y0) return false;
    return true;
}

bool Object::checkObjCollision()
{
    if (flags & flag_ignore_objs) return false;

    //TODO
    return false;
}

bool Object::checkPrioCollision()
{
    /* Before we check we need to update our priority. But, sometimes drawing depends on we having done this here as well */
    if (!(flags & Object::flag_fixed_priority)) {
        priority = std::max(4, (y / 12) + 1);
    }

    auto view_data = Engine::instance->res_view[view];
    if (!view_data) return false;
    auto view_info = view_data->info(loop, cel);

    auto bits = Engine::instance->screen.getPrioBits(x, y, view_info.width);
    if (priority != 0x0F) {
        if (isPlayer()) {
            Engine::instance->flag[Engine::FLAG_PLAYER_TOUCHED_TRIGGER] = (bits & 0x0004) == 0x0004;
            Engine::instance->flag[Engine::FLAG_PLAYER_ON_WATER] = bits == 0x0008;
        }

        if (flags & flag_on_water) {
            return bits != 0x0008;
        }
        if (flags & flag_on_land) {
            if (bits == 0x0008) return true;
        }

        if (bits & 0x0001) return true;
        if (!(flags & flag_ignore_blocks) && (bits & 0x0002)) return true;
    }
    return false;
}

void Object::setView(int new_view)
{
    if (view == new_view) return;
    view = new_view;
    if (loop >= Engine::instance->res_view[view]->loopCount())
        loop = 0;
    if (cel >= Engine::instance->res_view[view]->celCount(loop)) {
        cel = 0;
        cycle_counter = 0;
    }
}

void Object::setLoop(int new_loop)
{
    if (loop == new_loop) return;
    loop = new_loop;
    if (cel >= Engine::instance->res_view[view]->celCount(loop)) {
        cel = 0;
        cycle_counter = 0;
    }
}

void Object::fixPosition()
{
    auto view_data = Engine::instance->res_view[view];
    if (!view_data) return;
    auto view_info = view_data->info(loop, cel);
    //Ensure object is within screen bounds
    if (x < 0) x = 0;
    if (y < view_info.height - 1) y = view_info.height - 1;
    if (x > 160 - view_info.width) x = 160 - view_info.width;
    if (y > 167) y = 167;
    if (!(flags & flag_ignore_horizon)) {
        if (y <= Engine::instance->horizon) y = Engine::instance->horizon + 1;
    }
    int dir = 0;
    int count = 1;
    int size = 1;
    while(checkBoundsCollision() || checkObjCollision() || checkPrioCollision()) {
        switch(dir) {
        case 0: x--; if (--count) continue; dir++; break;
        case 1: y++; if (--count) continue; dir++; size++; break;
        case 2: x++; if (--count) continue; dir++; break;
        case 3: y--; if (--count) continue; dir=0; size++; break;
        }
        count = size;
    }
}

void Object::animate()
{
    if (flags & Object::flag_anim) return;
    flags = Object::flag_anim | Object::flag_update | Object::flag_cycling;
    motion = Motion::Normal;
    cycle_mode = CycleMode::Normal;
    direction = 0;
}

void Object::move_to(uint8_t target_x, uint8_t target_y, uint8_t step_size, uint8_t finished_flag)
{
    this->target_x = target_x;
    this->target_y = target_y;
    this->step_size_move_to = step_size ? step_size : this->step_size;
    this->move_finished_flag = finished_flag;
    flags |= Object::flag_update; 
    motion = Motion::MoveTo;
}

void Object::stopMotion()
{
    direction = 0;
    motion = Motion::Normal;
}

bool Object::inBox(uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1)
{
    auto view_data = Engine::instance->res_view[view];
    if (!view_data) return false;
    auto view_info = view_data->info(loop, cel);
    if (x + view_info.width <= x0) return false;
    if (x > x1) return false;
    if (y < y0) return false;
    if (y > y1) return false;
    return true;
}

bool Object::inArea(uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1, AreaCheckType type)
{
    int tx = x;
    switch(type) {
    case AreaCheckType::Left: break;    
    case AreaCheckType::Middle: tx += Engine::instance->res_view[view]->info(loop, cel).width / 2; break;
    case AreaCheckType::Right: tx += Engine::instance->res_view[view]->info(loop, cel).width - 1; break;
    }
    return x0 <= tx && tx <= x1 && y0 <= y && y <= y1;
}

int Object::distance(Object& other)
{
    if (!(flags & flag_draw)) return 255;
    if (!(other.flags & flag_draw)) return 255;
    auto dist = abs(x - other.x) + abs(y - other.y);
    if (dist >= 255) dist = 254;
    return dist;
}

bool Object::isPlayer()
{
    return this == &Engine::instance->object[0];
}

int Object::objIndex()
{
    return this - Engine::instance->object.data();
}

}