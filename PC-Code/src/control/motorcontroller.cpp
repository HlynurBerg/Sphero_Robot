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

TankSteering GetTankSteering(const Uint8* keyboard_state, SDL_Joystick * joystick, int distance, float max_speed) {

    float left_belt_float = 0.0, right_belt_float = 0.0;

    float stop = AutoStop(distance, 300, 500);

    //Checking keypresses
    bool is_up_pressed = keyboard_state[SDL_SCANCODE_W] || keyboard_state[SDL_SCANCODE_UP];
    bool is_down_pressed = keyboard_state[SDL_SCANCODE_S] || keyboard_state[SDL_SCANCODE_DOWN];
    bool is_left_pressed = keyboard_state[SDL_SCANCODE_A] || keyboard_state[SDL_SCANCODE_LEFT];
    bool is_right_pressed = keyboard_state[SDL_SCANCODE_D] || keyboard_state[SDL_SCANCODE_RIGHT];

    //Tune this variable to tune steering strength while driving
    float steer = 0.5;
    //Tilt of the joystick
    float input_strength = 1;
    //Speed of rotation
    float turn_speed = 0.5;

    //KBM inputs
    if(is_up_pressed || is_down_pressed || is_left_pressed || is_right_pressed) {

        // Adding up inputs
        if (is_up_pressed) { // AutoStop slows down the robot when close to objects
            left_belt_float = (left_belt_float +1)* stop;
            right_belt_float = (right_belt_float +1)* stop;
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
        std::cout << "joystick_x: " << joystick_x << " joystick_y: " << joystick_y << std::endl;
        if ((abs(joystick_x)/32767.0) < 0.1) { joystick_x = 0; }
        if ((abs(joystick_y)/32767.0) < 0.1) { joystick_y = 0; }


        input_strength = sqrt(joystick_x * joystick_x + joystick_y * joystick_y)/32768;
        if (input_strength > 1) {
            input_strength = 1;
        }

        //Dead-zone
        if (input_strength > 0.2) {
            if (joystick_y > 0) {stop = 1;} //allow reversing
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

TankSteering FollowMe(float difference, int distance, bool is_valid, float max_speed) {
    if (is_valid) {
        std::cout << distance << std::endl;
        float stop = AutoStop(distance, 500, 1000);
        float input_strength = 1;
        float turn_speed = 0.7;
        float left_belt_float = stop + difference;
        float right_belt_float = stop - difference;
        float new_max_speed = max_speed * (1-abs(difference));

        TankSteering result = NormalizeBelts(left_belt_float, right_belt_float, new_max_speed, input_strength, turn_speed);
        return result;
    }
    else {
        TankSteering result;
        result.left_belt_ = 10;
        result.right_belt_ = -10;
        return result;
    }
}
