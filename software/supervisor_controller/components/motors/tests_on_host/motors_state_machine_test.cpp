// Copyright (C) 2024 Rémi Peuchot (https://remipch.github.io/)
// This code is distributed under GNU GPL v3 license

#include "mini_mock.hpp"
#include "motors_hw.hpp"
#include "motors_state_machine.hpp"

MINI_MOCK_FUNCTION(motors_hw_init, motor_hw_error_t, (), ());
MINI_MOCK_FUNCTION(motors_hw_stop, void, (), ());
MINI_MOCK_FUNCTION(motors_hw_start_move,
                   void,
                   (motors_direction_t direction, bool continuous),
                   (direction, continuous));
MINI_MOCK_FUNCTION(motor_hw_get_state, motor_hw_state_t, (), ());

TEST(initialize, []() {
    // Nominal case : no hw error
    MINI_MOCK_ON_CALL(motors_hw_init, []() { return motor_hw_error_t::NO_ERROR; });
    motors_state_t state =
        motors_state_machine_update(motors_state_t::UNINITIALIZED, motors_transition_t::NONE, motors_direction_t::NONE);
    EXPECT(state == motors_state_t::STOPPED);

    // With hardware error : report error
    MINI_MOCK_ON_CALL(motors_hw_init, []() { return motor_hw_error_t::CANNOT_USE_UART; });
    state =
        motors_state_machine_update(motors_state_t::UNINITIALIZED, motors_transition_t::NONE, motors_direction_t::NONE);
    EXPECT(state == motors_state_t::ERROR);
});

// Test typical "move" scenario as a single whole story, with nominal case and basic corner cases
TEST(move, []() {
    motors_direction_t DIRECTION_1 = motors_direction_t::UP_LEFT;
    motors_direction_t DIRECTION_2 = motors_direction_t::UP;
    motors_direction_t DIRECTION_3 = motors_direction_t::DOWN_RIGHT;

    // Start move from STOPPED state
    MINI_MOCK_ON_CALL(motors_hw_start_move, [&](motors_direction_t direction, bool continuous) {
        EXPECT(direction == DIRECTION_1);
        EXPECT(!continuous);
    });
    motors_state_t state =
        motors_state_machine_update(motors_state_t::STOPPED, motors_transition_t::START_MOVE_ONE_STEP, DIRECTION_1);
    EXPECT(state == motors_state_t::MOVING);

    // Change direction while already moving
    MINI_MOCK_ON_CALL(motors_hw_start_move, [&](motors_direction_t direction, bool continuous) {
        EXPECT(direction == DIRECTION_2);
        EXPECT(!continuous);
    });
    state = motors_state_machine_update(motors_state_t::MOVING, motors_transition_t::START_MOVE_ONE_STEP, DIRECTION_2);
    EXPECT(state == motors_state_t::MOVING);

    // Start moving continuous while already moving
    MINI_MOCK_ON_CALL(motors_hw_start_move, [&](motors_direction_t direction, bool continuous) {
        EXPECT(direction == DIRECTION_3);
        EXPECT(continuous);
    });
    state =
        motors_state_machine_update(motors_state_t::MOVING, motors_transition_t::START_MOVE_CONTINUOUS, DIRECTION_3);
    EXPECT(state == motors_state_t::MOVING);

    // Update move : motors are still moving -> stay in state, do nothing
    MINI_MOCK_ON_CALL(motor_hw_get_state, []() { return motor_hw_state_t::MOVING; });
    state = motors_state_machine_update(motors_state_t::MOVING, motors_transition_t::NONE, motors_direction_t::NONE);
    EXPECT(state == motors_state_t::MOVING);

    // Update move : motors are stopped -> stopped
    MINI_MOCK_ON_CALL(motor_hw_get_state, []() { return motor_hw_state_t::STOPPED; });
    state = motors_state_machine_update(motors_state_t::MOVING, motors_transition_t::NONE, motors_direction_t::NONE);
    EXPECT(state == motors_state_t::STOPPED);

    // Update move : motors do not reply -> go to error
    MINI_MOCK_ON_CALL(motor_hw_get_state, []() { return motor_hw_state_t::UNKNOWN; });
    state = motors_state_machine_update(motors_state_t::MOVING, motors_transition_t::NONE, motors_direction_t::NONE);
    EXPECT(state == motors_state_t::ERROR);

    // Stop move
    MINI_MOCK_ON_CALL(motors_hw_stop, []() {});
    state = motors_state_machine_update(motors_state_t::MOVING, motors_transition_t::STOP, motors_direction_t::NONE);
    EXPECT(state == motors_state_t::STOPPING);
});

TEST(stopping, []() {
    // Stopping : motors are still moving
    MINI_MOCK_ON_CALL(motor_hw_get_state, []() { return motor_hw_state_t::MOVING; });
    motors_state_t state =
        motors_state_machine_update(motors_state_t::STOPPING, motors_transition_t::NONE, motors_direction_t::NONE);
    EXPECT(state == motors_state_t::STOPPING);

    // Stopping : motors are stopped
    MINI_MOCK_ON_CALL(motor_hw_get_state, []() { return motor_hw_state_t::STOPPED; });
    state = motors_state_machine_update(motors_state_t::STOPPING, motors_transition_t::NONE, motors_direction_t::NONE);
    EXPECT(state == motors_state_t::STOPPED);

    // Stopping : motors do not reply -> go to error
    MINI_MOCK_ON_CALL(motor_hw_get_state, []() { return motor_hw_state_t::UNKNOWN; });
    state = motors_state_machine_update(motors_state_t::STOPPING, motors_transition_t::NONE, motors_direction_t::NONE);
    EXPECT(state == motors_state_t::ERROR);
});

CREATE_MAIN_ENTRY_POINT();
