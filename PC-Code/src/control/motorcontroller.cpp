#include <control/motorcontroller.hpp>

TankSteering NormalizeBelts(float left_belt_float, float right_belt_float, float max_speed, float input_strength, float turn_speed) {
    //Make both belts have a value of between -1 and 1
    if ((fabs(left_belt_float) > fabs(right_belt_float)) and (round(left_belt_float) != 0)) {
        right_belt_float = right_belt_float /fabs(left_belt_float);
        left_belt_float = left_belt_float /fabs(left_belt_float);
    }
    else if (round(right_belt_float) != 0) {
        left_belt_float = left_belt_float /fabs(right_belt_float);
        right_belt_float = right_belt_float /fabs(right_belt_float);
    }
    //Make the left_belt_ and right_belt_ have integer values between -255 and 255
    TankSteering normalized;
    //turn slower when not driving. It was hard to control without this.
    if (left_belt_float == -right_belt_float) {
        normalized.left_belt_ = round(left_belt_float * 255 * max_speed * input_strength * turn_speed);
        normalized.right_belt_ = round(right_belt_float * 255 * max_speed * input_strength * turn_speed);
    }
    else {
        normalized.left_belt_ = round(left_belt_float * 255 * max_speed * input_strength);
        normalized.right_belt_ = round(right_belt_float * 255 * max_speed * input_strength);
    }
    return normalized;
}

float AutoStop(int value, int lower_bound, int upper_bound) {
    if (value < lower_bound) return 0.0;
    if (value > upper_bound) return 1.0;
    // Linear interpolation
    float stop = (value - lower_bound) / (upper_bound - lower_bound);
    return stop;
}


TankSteering GetTankSteering(const Uint8* keyboard_state, sdl_joystick* joystick, int distance) {

    float left_belt_float = 0.0, right_belt_float = 0.0;

    //Checking keypresses
    bool is_up_pressed = keyboard_state[SDL_SCANCODE_W] || keyboard_state[SDL_SCANCODE_UP];
    bool is_down_pressed = keyboard_state[SDL_SCANCODE_S] || keyboard_state[SDL_SCANCODE_DOWN];
    bool is_left_pressed = keyboard_state[SDL_SCANCODE_A] || keyboard_state[SDL_SCANCODE_LEFT];
    bool is_right_pressed = keyboard_state[SDL_SCANCODE_D] || keyboard_state[SDL_SCANCODE_RIGHT];

    //Tune this variable to tune steering strength while driving
    float steer = 0.5;
    //Max speed of the robot
    float max_speed = 0.3;
    //Tilt of the joystick
    float input_strength = 1;
    //Speed of rotation
    float turn_speed = 0.5;

    //KBM inputs
    if(is_up_pressed || is_down_pressed || is_left_pressed || is_right_pressed) {

        // Adding up inputs
        if (is_up_pressed) { // AutoStop slows down the robot when close to objects
            left_belt_float = (left_belt_float +1)* AutoStop(distance, 100, 500);
            right_belt_float = (right_belt_float +1)* AutoStop(distance, 100, 500);
        }
        if (is_down_pressed) {
            left_belt_float--;
            right_belt_float--;
        }
        if (is_left_pressed) {
            left_belt_float = left_belt_float - steer;
            right_belt_float = right_belt_float + steer;
        }
        if (is_right_pressed) {
            left_belt_float = left_belt_float + steer;
            right_belt_float = right_belt_float - steer;
        }
    }

    //Joystick input
    else {
        int joystick_x = SDL_JoystickGetAxis(joystick, 0);
        int joystick_y = SDL_JoystickGetAxis(joystick, 1);

        input_strength = sqrt(joystick_x * joystick_x + joystick_y * joystick_y)/32768;
        if (input_strength > 1) {
            input_strength = 1;
        }

        //Deadzone
        if (input_strength > 0.2) {
            float stop = 1;
            if (joystick_y < 0) {
                stop = AutoStop(distance, 100, 500);
            }
            left_belt_float = -joystick_y *stop + steer* joystick_x;
            right_belt_float = -joystick_y *stop - steer* joystick_x;
        }
        else {
            left_belt_float = 0;
            right_belt_float = 0;
        }
    }
    return NormalizeBelts(left_belt_float, right_belt_float, max_speed, input_strength, turn_speed);
}

TankSteering FollowMe(float difference, int distance, bool is_valid) {
    //parameters for how the robot should drive while autonomous
    if (is_valid) {
        float stop = AutoStop(distance, 100, 200);
        float input_strength = 1;
        float max_speed = 0.1;
        float turn_speed = 0.7;
        float left_belt_float = stop + difference;
        float right_belt_float = stop - difference;

        TankSteering result = NormalizeBelts(left_belt_float, right_belt_float, max_speed, input_strength, turn_speed);
        return result;
    }
    else {
        TankSteering result;
        result.left_belt_ = 10;
        result.right_belt_ = -10;
        return result;
    }
}