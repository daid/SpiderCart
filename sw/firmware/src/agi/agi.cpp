#include "agi.h"
#include <sstream>


namespace AGI {

Engine* Engine::instance = nullptr;

Engine::Engine()
{
    instance = this;
    flag[FLAG_LOGIC0_FIRST_TIME] = true;
    flag[FLAG_ROOM_FIRST_TIME] = true;
    new_room_nr = -1;
}

int Engine::step()
{
    flag[2] = false;
    flag[4] = false;
    if (player_control)
        object[0].direction = var[VAR_PLAYER_DIRECTION];
    else
        var[VAR_PLAYER_DIRECTION] = object[0].direction;
    var[VAR_PLAYER_BORDER_TOUCH] = 0;
    var[VAR_OTHER_BORDER_TOUCH_OBJ] = 0;
    var[VAR_OTHER_BORDER_TOUCH] = 0;

    for(auto& obj : object) {
        if ((obj.flags & Object::flag_anim) && (obj.flags & Object::flag_update))
            obj.update();
    }
    int res = runLogic(res_logic[0]);
    var[4] = 0;
    var[5] = 0;
    var[VAR_PLAYER_DIRECTION] = object[0].direction;
    flag[FLAG_ROOM_FIRST_TIME] = false;
    flag[FLAG_LOGIC0_FIRST_TIME] = false; // Not in documentation, but, I think we should?
    flag[FLAG_RESTART_GAME] = false;
    flag[12] = false;
    any_key_pressed = false;

    //TODO? Update objects on screen
    if (new_room_nr >= 0) {
        //TODO: More stuff needs to be done here.
        for(auto& obj : object) {
            obj.flags &=~(Object::flag_anim | Object::flag_draw);
            obj.flags |= Object::flag_update;
            obj.motion = Object::Motion::Normal;
            //TODO: obj.step_time = 1;
            //TODO: obj.step_counter = 1;
            obj.cycle_time = 1;
            obj.cycle_counter = 1;
            obj.step_size = 1;
        }
        switch(var[VAR_PLAYER_BORDER_TOUCH]) {
        case 1: object[0].y = 167; break;
        case 2: object[0].x = 0; break;
        case 3: object[0].y = horizon + 1; break;
        case 4: object[0].x = 159 - res_view[object[0].view]->info(object[0].loop, object[0].cel).width; break;
        }
        //TODO: More stuff needs to be done here.
        horizon = 36;
        var[VAR_PREV_ROOM] = var[VAR_CURRENT_ROOM];
        var[VAR_CURRENT_ROOM] = new_room_nr;
        new_room_nr = -1;
        var[VAR_PLAYER_BORDER_TOUCH] = 0;
        var[VAR_OTHER_BORDER_TOUCH_OBJ] = 0;
        var[VAR_OTHER_BORDER_TOUCH] = 0;
        var[VAR_PLAYER_VIEW] = object[0].view;
        var[9] = 0;
        flag[2] = false;
        flag[5] = true;
        return step();
    }
    return res;
}

#define VAR_ARG(n) trace_stream << " v" << int(logic->data[pc+n]) << "=" << int(var[logic->data[pc+n]])
#define NUM_ARG(n) trace_stream << " " << int(logic->data[pc+n])
#define FLAG_ARG(n) trace_stream << " f" << int(logic->data[pc+n]) << "=" << int(flag[logic->data[pc+n]])
#define OBJ_ARG(n) trace_stream << " o" << int(logic->data[pc+n])
#define ITEM_ARG(n) trace_stream << " o" << int(logic->data[pc+n])
#define MSG_ARG(n) trace_stream << " (" << int(logic->data[pc+n]) << ")\"" << logic->str(logic->data[pc+n]) << "\""
#define STR_ARG(n) trace_stream << " s" << int(logic->data[pc+n])
#define WORD_ARG(n) trace_stream << " w" << int(logic->data[pc+n])
#define CTR_ARG(n) trace_stream << " c" << int(logic->data[pc+n])
#define SAID_ARG() do { char said_word_buffer[64]; for(int n=0; n<logic->data[pc]; n++) { words.getWord(logic->data[pc+1+n*2] | (logic->data[pc+2+n*2] << 8), said_word_buffer, sizeof(said_word_buffer)); trace_stream << " " << said_word_buffer; } } while(0)
#define LOGIC_TRACE(name, ...) do { std::ostringstream trace_stream; trace_stream << name;  __VA_ARGS__; if (true) printf("%s\n", trace_stream.str().c_str()); } while(0)
#define CONDITION_TRACE_START(name, ...) std::ostringstream trace_stream; trace_stream << name; __VA_ARGS__;
#define CONDITION_TRACE(name, ...) do { trace_stream << name;  __VA_ARGS__; } while(0)
#define CONDITION_TRACE_END(name, ...) do { trace_stream << name; __VA_ARGS__; if (true) printf("%s\n", trace_stream.str().c_str()); } while(0)
#define UNIMPLEMENTED(); printf("Unimplemented.\n"); return -1;

#define N(n) logic->data[pc+(n)]
#define V(n) var[N(n)]

int random(int a, int b)
{
    return a + rand() % (b - a + 1);
}

int Engine::runLogic(LogicResource* logic)
{
    size_t pc = 2;
    while(true)
    {
        switch(logic->data[pc++]) {
        case 0x00: LOGIC_TRACE("return"); return 0;
        case 0x01: if (V(0) < 0xFF) V(0) += 1; LOGIC_TRACE("increment", VAR_ARG(0)); pc += 1; break;
        case 0x02: if (V(0) > 0x00) V(0) -= 1; LOGIC_TRACE("decrement", VAR_ARG(0)); pc += 1; break;
        case 0x03: V(0) = N(1); LOGIC_TRACE("assignn", VAR_ARG(0), NUM_ARG(1)); pc += 2; break;
        case 0x04: V(0) = V(1); LOGIC_TRACE("assignv", VAR_ARG(0), VAR_ARG(1)); pc += 2; break;
        case 0x05: V(0) += N(1); LOGIC_TRACE("addn", VAR_ARG(0), NUM_ARG(1)); pc += 2; break;
        case 0x06: V(0) += V(1); LOGIC_TRACE("addv", VAR_ARG(0), VAR_ARG(1)); pc += 2; break;
        case 0x07: V(0) -= N(1); LOGIC_TRACE("subn", VAR_ARG(0), NUM_ARG(1)); pc += 2; break;
        case 0x08: V(0) -= V(1); LOGIC_TRACE("subv", VAR_ARG(0), VAR_ARG(1)); pc += 2; break;
        case 0x09: var[V(0)] = V(1); LOGIC_TRACE("lindirectv", VAR_ARG(0), VAR_ARG(1)); pc += 2; break;
        case 0x0A: V(0) = var[V(1)]; LOGIC_TRACE("rindirect", VAR_ARG(0), VAR_ARG(1)); pc += 2; break;
        case 0x0B: var[V(0)] = N(1); LOGIC_TRACE("lindirectn", VAR_ARG(0), NUM_ARG(1)); pc += 2; break;
        case 0x0C: flag[N(0)] = true; LOGIC_TRACE("set", FLAG_ARG(0)); pc += 1; break;
        case 0x0D: flag[N(0)] = false; LOGIC_TRACE("reset", FLAG_ARG(0)); pc += 1; break;
        case 0x0E: flag[N(0)] = !flag[N(0)]; LOGIC_TRACE("toggle", FLAG_ARG(0)); pc += 1; break;
        case 0x0F: flag[V(0)] = true; LOGIC_TRACE("set.v", VAR_ARG(0)); pc += 1; break;
        case 0x10: flag[V(0)] = false; LOGIC_TRACE("reset.v", VAR_ARG(0)); pc += 1; break;
        case 0x11: flag[V(0)] = !flag[V(0)]; LOGIC_TRACE("toggle.v", VAR_ARG(0)); pc += 1; break;
        case 0x12: new_room_nr = N(0); LOGIC_TRACE("new.room", NUM_ARG(0)); pc += 1; return 1;
        case 0x13: new_room_nr = V(0); LOGIC_TRACE("new.room.v", VAR_ARG(0)); pc += 1; return 1;
        case 0x14: res_logic.load(N(0)); LOGIC_TRACE("load.logics", NUM_ARG(0)); pc += 1; break;
        case 0x15: res_logic.load(V(0)); LOGIC_TRACE("load.logics.v", VAR_ARG(0)); pc += 1; break;
        case 0x16: LOGIC_TRACE("call", NUM_ARG(0)); if (auto ret = runLogic(res_logic.load(N(0)))) return ret; pc += 1; break;
        case 0x17: LOGIC_TRACE("call.v", VAR_ARG(0)); if (auto ret = runLogic(res_logic.load(V(0)))) return ret; pc += 1; break;
        case 0x18: res_picture.load(V(0)); LOGIC_TRACE("load.pic", VAR_ARG(0)); pc += 1; break;
        case 0x19: screen.clear(); res_picture[V(0)]->draw(screen); LOGIC_TRACE("draw.pic", VAR_ARG(0)); pc += 1; break;
        case 0x1A: /* TODO: this should update the screen (why is this decoupled?) */ LOGIC_TRACE("show.pic"); break;
        case 0x1B: res_picture.unload(V(0)); LOGIC_TRACE("discard.pic", VAR_ARG(0)); pc += 1; break;
        case 0x1C: LOGIC_TRACE("overlay.pic", VAR_ARG(0)); pc += 1; UNIMPLEMENTED(); break;
        case 0x1D: LOGIC_TRACE("show.pri.screen"); UNIMPLEMENTED(); break;
        case 0x1E: res_view.load(N(0)); LOGIC_TRACE("load.view", NUM_ARG(0)); pc += 1; break;
        case 0x1F: res_view.load(V(0)); LOGIC_TRACE("load.view.v", VAR_ARG(0)); pc += 1; break;
        case 0x20: res_view.unload(N(0)); LOGIC_TRACE("discard.view", NUM_ARG(0)); pc += 1; break;
        case 0x21: object[N(0)].animate(); LOGIC_TRACE("animate.obj", OBJ_ARG(0)); pc += 1; break;
        case 0x22: for(auto& obj : object) obj.flags &=~(Object::flag_anim | Object::flag_draw); LOGIC_TRACE("unanimate.all"); break;
        case 0x23: object[N(0)].flags |= Object::flag_draw; object[N(0)].fixPosition(); LOGIC_TRACE("draw", OBJ_ARG(0)); pc += 1; break;
        case 0x24: object[N(0)].flags &=~Object::flag_draw; LOGIC_TRACE("erase", OBJ_ARG(0)); pc += 1; break;
        case 0x25: object[N(0)].x = N(1); object[N(0)].y = N(2); LOGIC_TRACE("position", OBJ_ARG(0), NUM_ARG(1), NUM_ARG(2)); pc += 3; break;
        case 0x26: object[N(0)].x = V(1); object[N(0)].y = V(2); LOGIC_TRACE("position.v", OBJ_ARG(0), VAR_ARG(1), VAR_ARG(2)); pc += 3; break;
        case 0x27: V(1) = object[N(0)].x; V(2) = object[N(0)].y; LOGIC_TRACE("get.posn", OBJ_ARG(0), VAR_ARG(1), VAR_ARG(2)); pc += 3; break;
        case 0x28: object[N(0)].x += int8_t(V(1)); object[N(0)].y += int8_t(V(2)); object[N(0)].fixPosition(); LOGIC_TRACE("reposition", OBJ_ARG(0), VAR_ARG(1), VAR_ARG(2)); pc += 3; break;
        case 0x29: object[N(0)].setView(N(1)); LOGIC_TRACE("set.view", OBJ_ARG(0), NUM_ARG(1)); pc += 2; break;
        case 0x2A: object[N(0)].setView(V(1)); LOGIC_TRACE("set.view.v", OBJ_ARG(0), VAR_ARG(1)); pc += 2; break;
        case 0x2B: object[N(0)].setLoop(N(1)); LOGIC_TRACE("set.loop", OBJ_ARG(0), NUM_ARG(1)); pc += 2; break;
        case 0x2C: object[N(0)].setLoop(V(1)); LOGIC_TRACE("set.loop.v", OBJ_ARG(0), VAR_ARG(1)); pc += 2; break;
        case 0x2D: object[N(0)].flags |= Object::flag_fix_loop; LOGIC_TRACE("fix.loop", OBJ_ARG(0)); pc += 1; break;
        case 0x2E: object[N(0)].flags &=~Object::flag_fix_loop; LOGIC_TRACE("release.loop", OBJ_ARG(0)); pc += 1; break;
        case 0x2F: object[N(0)].flags &=~Object::flag_dont_update; object[N(0)].cel = N(1); LOGIC_TRACE("set.cel", OBJ_ARG(0), NUM_ARG(1)); pc += 2; break;
        case 0x30: object[N(0)].flags &=~Object::flag_dont_update; object[N(0)].cel = V(1); LOGIC_TRACE("set.cel.v", OBJ_ARG(0), VAR_ARG(1)); pc += 2; break;
        case 0x31: V(1) = res_view[object[N(0)].view]->celCount(object[N(0)].loop) - 1; LOGIC_TRACE("last.cel", OBJ_ARG(0), VAR_ARG(1)); pc += 2; break;
        case 0x32: V(1) = object[N(0)].cel; LOGIC_TRACE("current.cel", OBJ_ARG(0), VAR_ARG(1)); pc += 2; break;
        case 0x33: V(1) = object[N(0)].loop; LOGIC_TRACE("current.loop", OBJ_ARG(0), VAR_ARG(1)); pc += 2; break;
        case 0x34: V(1) = object[N(0)].view; LOGIC_TRACE("current.view", OBJ_ARG(0), VAR_ARG(1)); pc += 2; break;
        case 0x35: V(1) = res_view[object[N(0)].view]->loopCount(); LOGIC_TRACE("number.of.loops", OBJ_ARG(0), VAR_ARG(1)); pc += 2; break;
        case 0x36: object[N(0)].flags |= Object::flag_fixed_priority; object[N(0)].priority = N(1); LOGIC_TRACE("set.priority", OBJ_ARG(0), NUM_ARG(1)); pc += 2; break;
        case 0x37: object[N(0)].flags |= Object::flag_fixed_priority; object[N(0)].priority = V(1); LOGIC_TRACE("set.priority.v", OBJ_ARG(0), VAR_ARG(1)); pc += 2; break;
        case 0x38: object[N(0)].flags &=~Object::flag_fixed_priority; LOGIC_TRACE("release.priority", OBJ_ARG(0)); pc += 1; break;
        case 0x39: V(1) = object[N(0)].priority; LOGIC_TRACE("get.priority", OBJ_ARG(0), VAR_ARG(1)); pc += 2; break;
        case 0x3A: object[N(0)].flags &=~Object::flag_update; LOGIC_TRACE("stop.update", OBJ_ARG(0)); pc += 1; break;
        case 0x3B: object[N(0)].flags |= Object::flag_update; LOGIC_TRACE("start.update", OBJ_ARG(0)); pc += 1; break;
        case 0x3C: LOGIC_TRACE("force.update", OBJ_ARG(0)); pc += 1; UNIMPLEMENTED(); break;
        case 0x3D: object[N(0)].flags |= Object::flag_ignore_horizon; LOGIC_TRACE("ignore.horizon", OBJ_ARG(0)); pc += 1; break;
        case 0x3E: object[N(0)].flags &=~Object::flag_ignore_horizon; LOGIC_TRACE("observe.horizon", OBJ_ARG(0)); pc += 1; break;
        case 0x3F: horizon = N(0); LOGIC_TRACE("set.horizon", NUM_ARG(0)); pc += 1; break;
        case 0x40: object[N(0)].flags |= Object::flag_on_water; LOGIC_TRACE("object.on.water", OBJ_ARG(0)); pc += 1; break;
        case 0x41: object[N(0)].flags |= Object::flag_on_land; LOGIC_TRACE("object.on.land", OBJ_ARG(0)); pc += 1; break;
        case 0x42: object[N(0)].flags &=~(Object::flag_on_water | Object::flag_on_land); LOGIC_TRACE("object.on.anything", OBJ_ARG(0)); pc += 1; break;
        case 0x43: object[N(0)].flags |= Object::flag_ignore_objs; LOGIC_TRACE("ignore.objs", OBJ_ARG(0)); pc += 1; break;
        case 0x44: object[N(0)].flags &=~Object::flag_ignore_objs; LOGIC_TRACE("observe.objs", OBJ_ARG(0)); pc += 1; break;
        case 0x45: V(1) = object[N(0)].distance(object[N(1)]); LOGIC_TRACE("distance", OBJ_ARG(0), OBJ_ARG(1), VAR_ARG(2)); pc += 3; break;
        case 0x46: object[N(0)].flags &=~Object::flag_cycling; LOGIC_TRACE("stop.cycling", OBJ_ARG(0)); pc += 1; break;
        case 0x47: object[N(0)].flags |= Object::flag_cycling; LOGIC_TRACE("start.cycling", OBJ_ARG(0)); pc += 1; break;
        case 0x48: object[N(0)].cycle_mode = Object::CycleMode::Normal; object[N(0)].flags |= Object::flag_cycling; LOGIC_TRACE("normal.cycle", OBJ_ARG(0)); pc += 1; break;
        case 0x49: object[N(0)].cycle_mode = Object::CycleMode::NormalOnce; object[N(0)].flags |= (Object::flag_update | Object::flag_cycling); object[N(0)].cycle_finished_flag = N(1); flag[N(1)] = false; LOGIC_TRACE("end.of.loop", OBJ_ARG(0), FLAG_ARG(1)); pc += 2; break;
        case 0x4A: object[N(0)].cycle_mode = Object::CycleMode::Reverse; object[N(0)].flags |= Object::flag_cycling; LOGIC_TRACE("reverse.cycle", OBJ_ARG(0)); pc += 1; break;
        case 0x4B: object[N(0)].cycle_mode = Object::CycleMode::ReverseOnce; object[N(0)].flags |= (Object::flag_update | Object::flag_cycling); object[N(0)].cycle_finished_flag = N(1); flag[N(1)] = false; LOGIC_TRACE("reverse.loop", OBJ_ARG(0), FLAG_ARG(1)); pc += 2; break;
        case 0x4C: object[N(0)].cycle_time = V(1); LOGIC_TRACE("cycle.time", OBJ_ARG(0), VAR_ARG(1)); pc += 2; break;
        case 0x4D: if (N(0) == 0) player_control = false; object[N(0)].stopMotion(); LOGIC_TRACE("stop.motion", OBJ_ARG(0)); pc += 1; break;
        case 0x4E: if (N(0) == 0) { player_control = true; object[N(0)].direction = 0; } object[N(0)].motion = Object::Motion::Normal; LOGIC_TRACE("start.motion", OBJ_ARG(0)); pc += 1; break;
        case 0x4F: object[N(0)].step_size = V(1); LOGIC_TRACE("step.size", OBJ_ARG(0), VAR_ARG(1)); pc += 2; break;
        case 0x50: LOGIC_TRACE("step.time", OBJ_ARG(0), VAR_ARG(1)); pc += 2; UNIMPLEMENTED(); break;
        case 0x51: if (N(0) == 0) player_control = false; object[N(0)].move_to(N(1), N(2), N(3), N(4)); LOGIC_TRACE("move.obj", OBJ_ARG(0), NUM_ARG(1), NUM_ARG(2), NUM_ARG(3), FLAG_ARG(4)); pc += 5; break;
        case 0x52: if (N(0) == 0) player_control = false; object[N(0)].move_to(V(1), V(2), V(3), N(4)); LOGIC_TRACE("move.obj.v", OBJ_ARG(0), VAR_ARG(1), VAR_ARG(2), VAR_ARG(3), FLAG_ARG(4)); pc += 5; break;
        case 0x53: object[N(0)].flags |= Object::flag_update; object[N(0)].motion = Object::Motion::FollowPlayer; flag[N(1)] = false; object[N(2)].step_size = N(1); object[N(2)].move_finished_flag = N(2); LOGIC_TRACE("follow.ego", OBJ_ARG(0), NUM_ARG(1), FLAG_ARG(2)); pc += 3; break;
        case 0x54: if (N(0) == 0) player_control = false; object[N(0)].motion = Object::Motion::Wander; object[N(0)].flags |= Object::flag_update; LOGIC_TRACE("wander", OBJ_ARG(0)); pc += 1; break;
        case 0x55: object[N(0)].motion = Object::Motion::Normal; LOGIC_TRACE("normal.motion", OBJ_ARG(0)); pc += 1; break;
        case 0x56: LOGIC_TRACE("set.dir", OBJ_ARG(0), VAR_ARG(1)); pc += 2; UNIMPLEMENTED(); break;
        case 0x57: LOGIC_TRACE("get.dir", OBJ_ARG(0), VAR_ARG(1)); pc += 2; UNIMPLEMENTED(); break;
        case 0x58: object[N(0)].flags |= Object::flag_ignore_blocks; LOGIC_TRACE("ignore.blocks", OBJ_ARG(0)); pc += 1; break;
        case 0x59: object[N(0)].flags &=~Object::flag_ignore_blocks; LOGIC_TRACE("observe.blocks", OBJ_ARG(0)); pc += 1; break;
        case 0x5A: LOGIC_TRACE("block", NUM_ARG(0), NUM_ARG(1), NUM_ARG(2), NUM_ARG(3)); pc += 4; UNIMPLEMENTED(); break;
        case 0x5B: LOGIC_TRACE("unblock"); UNIMPLEMENTED(); break;
        case 0x5C: LOGIC_TRACE("get", ITEM_ARG(0)); pc += 1; UNIMPLEMENTED(); break;
        case 0x5D: LOGIC_TRACE("get.v", VAR_ARG(0)); pc += 1; UNIMPLEMENTED(); break;
        case 0x5E: LOGIC_TRACE("drop", ITEM_ARG(0)); pc += 1; UNIMPLEMENTED(); break;
        case 0x5F: LOGIC_TRACE("put", ITEM_ARG(0)); pc += 2; UNIMPLEMENTED(); break;
        case 0x60: LOGIC_TRACE("put.v", VAR_ARG(0), VAR_ARG(1)); pc += 2; UNIMPLEMENTED(); break;
        case 0x61: LOGIC_TRACE("get.room.v", VAR_ARG(0), VAR_ARG(1)); pc += 2; UNIMPLEMENTED(); break;
        case 0x62: /* TODO: sound */ LOGIC_TRACE("load.sound", NUM_ARG(0)); pc += 1; break;
        case 0x63: /* TODO: sound */ LOGIC_TRACE("sound", NUM_ARG(0), FLAG_ARG(1)); pc += 2; break;
        case 0x64: /* TODO: sound */ LOGIC_TRACE("stop.sound"); break;
        case 0x65: printf("## %s\n", logic->str(N(0))); LOGIC_TRACE("print", MSG_ARG(0)); pc += 1; break;
        case 0x66: LOGIC_TRACE("print.v", VAR_ARG(0)); pc += 1; UNIMPLEMENTED(); break;
        case 0x67: /*TODO: Text mode */ LOGIC_TRACE("display", NUM_ARG(0), NUM_ARG(1), MSG_ARG(2)); pc += 3; break;
        case 0x68: /*TODO: Text mode */ LOGIC_TRACE("display.v", VAR_ARG(0), VAR_ARG(1), VAR_ARG(2)); pc += 3; break;
        case 0x69: /*TODO: Text mode */ LOGIC_TRACE("clear.lines", NUM_ARG(0), NUM_ARG(1), NUM_ARG(2)); pc += 3; break;
        case 0x6A: /*TODO: Text mode */ LOGIC_TRACE("text.screen"); break;
        case 0x6B: /*TODO: Text mode */ LOGIC_TRACE("graphics"); break;
        case 0x6C: LOGIC_TRACE("set.cursor.char", MSG_ARG(0)); pc += 1; break;
        case 0x6D: LOGIC_TRACE("set.text.attribute", NUM_ARG(0), NUM_ARG(1)); pc += 2; UNIMPLEMENTED(); break;
        case 0x6E: /*TODO: screen shake */ LOGIC_TRACE("shake.screen", NUM_ARG(0)); pc += 1; break;
        case 0x6F: LOGIC_TRACE("configure.screen", NUM_ARG(0), NUM_ARG(1), NUM_ARG(2)); pc += 3; break;
        case 0x70: show_status = true; LOGIC_TRACE("status.line.on"); break;
        case 0x71: show_status = false; LOGIC_TRACE("status.line.off"); break;
        case 0x72: if (N(0) < 12) str[N(0)] = logic->str(N(1)); LOGIC_TRACE("set.string", STR_ARG(0), MSG_ARG(1)); pc += 2; break;
        case 0x73: /*TODO: Text input mode */ str[N(0)] = "get.string result"; LOGIC_TRACE("get.string", STR_ARG(0), MSG_ARG(1), NUM_ARG(2), NUM_ARG(3), NUM_ARG(4)); pc += 5; break;
        case 0x74: LOGIC_TRACE("word.to.string", WORD_ARG(0), STR_ARG(1)); pc += 2; UNIMPLEMENTED(); break;
        case 0x75: /*TODO: Text input mode */ LOGIC_TRACE("parse", STR_ARG(0)); pc += 1; break;
        case 0x76: LOGIC_TRACE("get.num", STR_ARG(0), VAR_ARG(1)); pc += 2; UNIMPLEMENTED(); break;
        case 0x77: input_enabled = false; LOGIC_TRACE("prevent.input"); break;
        case 0x78: input_enabled = true; LOGIC_TRACE("accept.input"); break;
        case 0x79: /* TODO? */ LOGIC_TRACE("set.key", NUM_ARG(0), NUM_ARG(1), NUM_ARG(2)); pc += 3; break;
        case 0x7A: screen.drawView(Engine::instance->res_view[N(0)]->info(N(1), N(2)), N(3), N(4), N(5), N(6)); LOGIC_TRACE("add.to.pic", NUM_ARG(0), NUM_ARG(1), NUM_ARG(2), NUM_ARG(3), NUM_ARG(4), NUM_ARG(5), NUM_ARG(6)); pc += 7; break;
        case 0x7B: screen.drawView(Engine::instance->res_view[V(0)]->info(V(1), V(2)), V(3), V(4), V(5), V(6)); LOGIC_TRACE("add.to.pic.v", VAR_ARG(0), VAR_ARG(1), VAR_ARG(2), VAR_ARG(3), VAR_ARG(4), VAR_ARG(5), VAR_ARG(6)); pc += 7; break;
        case 0x7C: LOGIC_TRACE("status"); UNIMPLEMENTED(); break;
        case 0x7D: LOGIC_TRACE("save.game"); UNIMPLEMENTED(); break;
        case 0x7E: LOGIC_TRACE("restore.game"); UNIMPLEMENTED(); break;
        case 0x7F: LOGIC_TRACE("init.disk"); UNIMPLEMENTED(); break;
        case 0x80: LOGIC_TRACE("restart.game"); UNIMPLEMENTED(); break;
        case 0x81: LOGIC_TRACE("show.obj", NUM_ARG(0)); pc += 1; UNIMPLEMENTED(); break;
        case 0x82: V(2) = random(N(1), N(2)); LOGIC_TRACE("random", NUM_ARG(0), NUM_ARG(1), VAR_ARG(2)); pc += 3; break;
        case 0x83: player_control = false; LOGIC_TRACE("program.control"); break;
        case 0x84: player_control = true; object[0].motion = Object::Motion::Normal; LOGIC_TRACE("player.control"); break;
        case 0x85: LOGIC_TRACE("obj.status.v", VAR_ARG(0)); pc += 1; UNIMPLEMENTED(); break;
        case 0x86: LOGIC_TRACE("quit", NUM_ARG(0)); pc += 1; UNIMPLEMENTED(); break;
        case 0x87: LOGIC_TRACE("show.mem"); UNIMPLEMENTED(); break;
        case 0x88: LOGIC_TRACE("pause"); UNIMPLEMENTED(); break;
        case 0x89: LOGIC_TRACE("echo.line"); UNIMPLEMENTED(); break;
        case 0x8A: LOGIC_TRACE("cancel.line"); UNIMPLEMENTED(); break;
        case 0x8B: LOGIC_TRACE("init.joy"); UNIMPLEMENTED(); break;
        case 0x8C: LOGIC_TRACE("toggle.monitor"); UNIMPLEMENTED(); break;
        case 0x8D: LOGIC_TRACE("version"); UNIMPLEMENTED(); break;
        case 0x8E: LOGIC_TRACE("script.size", NUM_ARG(0)); pc += 1; break;
        case 0x8F: LOGIC_TRACE("set.game.id", MSG_ARG(0)); pc += 1; break;
        case 0x90: LOGIC_TRACE("log", MSG_ARG(0)); pc += 1; UNIMPLEMENTED(); break;
        case 0x91: LOGIC_TRACE("set.scan.start"); UNIMPLEMENTED(); break;
        case 0x92: LOGIC_TRACE("reset.scan.start"); UNIMPLEMENTED(); break;
        case 0x93: LOGIC_TRACE("reposition.to", OBJ_ARG(0), NUM_ARG(1), NUM_ARG(2)); pc += 3; UNIMPLEMENTED(); break;
        case 0x94: LOGIC_TRACE("reposition.to.v", OBJ_ARG(0), VAR_ARG(1), VAR_ARG(2)); pc += 3; UNIMPLEMENTED(); break;
        case 0x95: LOGIC_TRACE("trace.on"); UNIMPLEMENTED(); break;
        case 0x96: /* TODO? */ LOGIC_TRACE("trace.info", NUM_ARG(0), NUM_ARG(1), NUM_ARG(2)); pc += 3; break;
        case 0x97: LOGIC_TRACE("print.at", MSG_ARG(0), NUM_ARG(1), NUM_ARG(2), NUM_ARG(3)); pc += 4; UNIMPLEMENTED(); break;
        case 0x98: LOGIC_TRACE("print.at.v", VAR_ARG(0), NUM_ARG(1), NUM_ARG(2), NUM_ARG(3)); pc += 4; UNIMPLEMENTED(); break;
        case 0x99: res_view.unload(V(0)); LOGIC_TRACE("discard.view.v", VAR_ARG(0)); pc += 1; break;
        case 0x9A: LOGIC_TRACE("clear.text.rect", NUM_ARG(0), NUM_ARG(1), NUM_ARG(2), NUM_ARG(3), NUM_ARG(4)); pc += 5; UNIMPLEMENTED(); break;
        case 0x9B: LOGIC_TRACE("set.upper.left", NUM_ARG(0), NUM_ARG(1)); pc += 2; UNIMPLEMENTED(); break;
        case 0x9C: LOGIC_TRACE("set.menu", MSG_ARG(0)); pc += 1; break;
        case 0x9D: LOGIC_TRACE("set.menu.member", MSG_ARG(0), CTR_ARG(1)); pc += 2; break;
        case 0x9E: LOGIC_TRACE("submit.menu"); break;
        case 0x9F: LOGIC_TRACE("enable.item", CTR_ARG(0)); pc += 1; break;
        case 0xA0: LOGIC_TRACE("disable.item", CTR_ARG(0)); pc += 1; break;
        case 0xA1: LOGIC_TRACE("menu.input"); UNIMPLEMENTED(); break;
        case 0xA2: LOGIC_TRACE("show.obj.v", VAR_ARG(0)); pc += 1; UNIMPLEMENTED(); break;
        case 0xA3: /*TODO: Text input mode */ LOGIC_TRACE("open.dialogue"); break;
        case 0xA4: /*TODO: Text input mode */ LOGIC_TRACE("close.dialogue"); break;
        case 0xA5: V(0) *= N(1); LOGIC_TRACE("mul.n", VAR_ARG(0), NUM_ARG(1)); pc += 2; break;
        case 0xA6: V(0) *= V(1); LOGIC_TRACE("mul.v", VAR_ARG(0), VAR_ARG(1)); pc += 2; break;
        case 0xA7: V(0) /= N(1); LOGIC_TRACE("div.n", VAR_ARG(0), NUM_ARG(1)); pc += 2; break;
        case 0xA8: V(0) /= V(1); LOGIC_TRACE("div.v", VAR_ARG(0), VAR_ARG(1)); pc += 2; break;
        case 0xA9: LOGIC_TRACE("close.window"); UNIMPLEMENTED(); break;
        case 0xAA: LOGIC_TRACE("set.simple"); pc += 1; UNIMPLEMENTED(); break;
        case 0xAB: LOGIC_TRACE("push.script"); UNIMPLEMENTED(); break;
        case 0xAC: LOGIC_TRACE("pop.script"); UNIMPLEMENTED(); break;
        case 0xAD: LOGIC_TRACE("hold.key"); UNIMPLEMENTED(); break;
        case 0xAE: LOGIC_TRACE("set.pri.base", NUM_ARG(0)); pc += 1; UNIMPLEMENTED(); break;
        case 0xAF: LOGIC_TRACE("discard.sound", NUM_ARG(0)); pc += 1; UNIMPLEMENTED(); break;
        case 0xB0: LOGIC_TRACE("hide.mouse"); UNIMPLEMENTED(); break;
        case 0xB1: LOGIC_TRACE("allow.menu"); pc += 1; UNIMPLEMENTED(); break;
        case 0xB2: LOGIC_TRACE("show.mouse"); UNIMPLEMENTED(); break;
        case 0xB3: LOGIC_TRACE("fence.mouse", NUM_ARG(0), NUM_ARG(1), NUM_ARG(2), NUM_ARG(3)); pc += 4; UNIMPLEMENTED(); break;
        case 0xB4: LOGIC_TRACE("mouse.posn", VAR_ARG(0), VAR_ARG(1)); pc += 2; UNIMPLEMENTED(); break;
        case 0xB5: LOGIC_TRACE("release.key"); UNIMPLEMENTED(); break;
        case 0xB6: LOGIC_TRACE("adj.ego.move.to.xy"); UNIMPLEMENTED(); break;

        case 0xFE: // JUMP
            LOGIC_TRACE("JUMP ", trace_stream << " " << logic->s16(pc));
            pc += logic->s16(pc) + 2;
            break;
        case 0xFF: // IF
            {
                CONDITION_TRACE_START("IF");
                bool result = true;
                int invert = 0;
                bool in_or = false;
                bool or_result = false;
                while(logic->data[pc] != 0xFF) {
                    bool value = false;
                    switch(logic->data[pc++]) {
                    case 0x01: value = V(0) == N(1); CONDITION_TRACE(" EQ", VAR_ARG(0), NUM_ARG(1)); pc += 2; break;
                    case 0x02: value = V(0) == V(1); CONDITION_TRACE(" EQ", VAR_ARG(0), VAR_ARG(1)); pc += 2; break;
                    case 0x03: value = V(0) < N(1); CONDITION_TRACE(" LT", VAR_ARG(0), NUM_ARG(1)); pc += 2; break;
                    case 0x04: value = V(0) < V(1); CONDITION_TRACE(" LT", VAR_ARG(0), VAR_ARG(1)); pc += 2; break;
                    case 0x05: value = V(0) > N(1); CONDITION_TRACE(" GT", VAR_ARG(0), NUM_ARG(1)); pc += 2; break;
                    case 0x06: value = V(0) > V(1); CONDITION_TRACE(" GT", VAR_ARG(0), VAR_ARG(1)); pc += 2; break;
                    case 0x07: value = flag[N(0)]; CONDITION_TRACE(" FLAG", FLAG_ARG(0)); pc += 1; break;
                    case 0x08: value = flag[V(0)]; CONDITION_TRACE(" FLAG", VAR_ARG(0)); pc += 1; break;
                    case 0x09: value = item_room[N(0)] == 255; CONDITION_TRACE(" ITEM", ITEM_ARG(0)); pc += 1; break;
                    case 0x0B: value = object[N(0)].inArea(N(1), N(3), N(2), N(4), Object::AreaCheckType::Left); CONDITION_TRACE(" POSN", OBJ_ARG(0), NUM_ARG(1), NUM_ARG(2), NUM_ARG(3), NUM_ARG(4)); pc += 5; break;
                    case 0x0C: value = false; /* TODO: Key pressed/menu item selection */ CONDITION_TRACE(" CONTROLLER", CTR_ARG(0)); pc += 1; break;
                    case 0x0D: value = any_key_pressed; CONDITION_TRACE(" HAVE.KEY"); break;
                    case 0x0E: value = false; CONDITION_TRACE(" SAID", SAID_ARG()); pc += N(0) * 2 + 1; /* TODO: said ... */ break;
                    case 0x10: value = object[N(0)].inBox(N(1), N(3), N(2), N(4)); CONDITION_TRACE(" OBJ.IN.BOX", OBJ_ARG(0), NUM_ARG(1), NUM_ARG(2), NUM_ARG(3), NUM_ARG(4)); pc += 5; break;
                    case 0x11: value = object[N(0)].inArea(N(1), N(3), N(2), N(4), Object::AreaCheckType::Middle); CONDITION_TRACE(" POSN", OBJ_ARG(0), NUM_ARG(1), NUM_ARG(2), NUM_ARG(3), NUM_ARG(4)); pc += 5; break;
                    case 0x12: value = object[N(0)].inArea(N(1), N(3), N(2), N(4), Object::AreaCheckType::Right); CONDITION_TRACE(" POSN", OBJ_ARG(0), NUM_ARG(1), NUM_ARG(2), NUM_ARG(3), NUM_ARG(4)); pc += 5; break;
                    case 0xFC: in_or = !in_or; if (in_or) or_result = false; value = or_result; CONDITION_TRACE(" OR"); break;
                    case 0xFD: value = !in_or; invert = 2; CONDITION_TRACE(" NOT"); break;
                    default:
                        CONDITION_TRACE_END(": ");
                        printf("Unknown logic condition: %02X\n", logic->data[pc-1]);
                        return -1;
                    }
                    if (invert) { if (invert == 1) value = !value; invert -= 1; }
                    if (in_or) {
                        or_result = or_result || value;
                    } else {
                        result = result && value;
                    }
                }
                pc++; // Skip the FF
                if (!result) {
                    CONDITION_TRACE_END(": FALSE");
                    pc += logic->u16(pc);
                } else {
                    CONDITION_TRACE_END(": TRUE");
                }
                pc += 2;
            }
            break;
        default:
            printf("Unknown logic opcode: %02X\n", logic->data[pc-1]);
            return -1;
        }
    }
}

}