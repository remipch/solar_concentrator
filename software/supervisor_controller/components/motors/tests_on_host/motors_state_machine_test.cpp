#include "mini_mock.hpp"
#include "motors_hw.hpp"
#include "motors_state_machine.hpp"

MINI_MOCK_FUNCTION(motors_hw_init, motor_hw_error_t, (), ());
MINI_MOCK_FUNCTION(motors_hw_stop, void, (), ());
MINI_MOCK_FUNCTION(motors_hw_move_one_step, void, (motors_direction_t direction), (direction));
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

// Test typical "move_continuous" scenario as a single whole story, with nominal case and basic corner cases
TEST(move_continuous, []() {
    motors_direction_t DIRECTION_1 = motors_direction_t::UP_LEFT;
    motors_direction_t DIRECTION_2 = motors_direction_t::UP;
    motors_direction_t DIRECTION_3 = motors_direction_t::DOWN_RIGHT;

    // Start move continuous from STOPPED state
    MINI_MOCK_ON_CALL(motors_hw_move_one_step, [&](motors_direction_t direction) { EXPECT(direction == DIRECTION_1); });
    motors_state_t state =
        motors_state_machine_update(motors_state_t::STOPPED, motors_transition_t::START_MOVE_CONTINUOUS, DIRECTION_1);
    EXPECT(state == motors_state_t::MOVING_CONTINUOUS);

    // Change direction while already moving continuous
    MINI_MOCK_ON_CALL(motors_hw_move_one_step, [&](motors_direction_t direction) { EXPECT(direction == DIRECTION_2); });
    state = motors_state_machine_update(
        motors_state_t::MOVING_CONTINUOUS, motors_transition_t::START_MOVE_CONTINUOUS, DIRECTION_2);
    EXPECT(state == motors_state_t::MOVING_CONTINUOUS);

    // Start move continuous from moving one step
    MINI_MOCK_ON_CALL(motors_hw_move_one_step, [&](motors_direction_t direction) { EXPECT(direction == DIRECTION_3); });
    state = motors_state_machine_update(
        motors_state_t::MOVING_ONE_STEP, motors_transition_t::START_MOVE_CONTINUOUS, DIRECTION_3);
    EXPECT(state == motors_state_t::MOVING_CONTINUOUS);

    // Update move continuous : motors are still moving -> stay in state, do nothing
    MINI_MOCK_ON_CALL(motor_hw_get_state, []() { return motor_hw_state_t::MOVING; });
    state = motors_state_machine_update(
        motors_state_t::MOVING_CONTINUOUS, motors_transition_t::NONE, motors_direction_t::NONE);
    EXPECT(state == motors_state_t::MOVING_CONTINUOUS);

    // Update move continuous : motors are stopped -> stay in state, restart motors
    MINI_MOCK_ON_CALL(motor_hw_get_state, []() { return motor_hw_state_t::STOPPED; });
    MINI_MOCK_ON_CALL(motors_hw_move_one_step, [&](motors_direction_t direction) { EXPECT(direction == DIRECTION_3); });
    state = motors_state_machine_update(
        motors_state_t::MOVING_CONTINUOUS, motors_transition_t::NONE, motors_direction_t::NONE);
    EXPECT(state == motors_state_t::MOVING_CONTINUOUS);

    // Update move continuous : motors do not reply -> go to error
    MINI_MOCK_ON_CALL(motor_hw_get_state, []() { return motor_hw_state_t::UNKNOWN; });
    state = motors_state_machine_update(
        motors_state_t::MOVING_CONTINUOUS, motors_transition_t::NONE, motors_direction_t::NONE);
    EXPECT(state == motors_state_t::ERROR);

    // Stop move continuous
    MINI_MOCK_ON_CALL(motors_hw_stop, []() {});
    state = motors_state_machine_update(
        motors_state_t::MOVING_CONTINUOUS, motors_transition_t::STOP, motors_direction_t::NONE);
    EXPECT(state == motors_state_t::STOPPING);
});

// Test typical "move_one_step" scenario as a single whole story, with nominal case and basic corner cases
TEST(move_one_step, []() {
    motors_direction_t DIRECTION_1 = motors_direction_t::UP_LEFT;
    motors_direction_t DIRECTION_2 = motors_direction_t::UP;
    motors_direction_t DIRECTION_3 = motors_direction_t::DOWN_RIGHT;

    // Start move one_step from STOPPED state
    MINI_MOCK_ON_CALL(motors_hw_move_one_step, [&](motors_direction_t direction) { EXPECT(direction == DIRECTION_1); });
    motors_state_t state =
        motors_state_machine_update(motors_state_t::STOPPED, motors_transition_t::START_MOVE_ONE_STEP, DIRECTION_1);
    EXPECT(state == motors_state_t::MOVING_ONE_STEP);

    // Change direction while already moving one_step
    MINI_MOCK_ON_CALL(motors_hw_move_one_step, [&](motors_direction_t direction) { EXPECT(direction == DIRECTION_2); });
    state = motors_state_machine_update(
        motors_state_t::MOVING_ONE_STEP, motors_transition_t::START_MOVE_ONE_STEP, DIRECTION_2);
    EXPECT(state == motors_state_t::MOVING_ONE_STEP);

    // Start move one_step from moving continuous
    MINI_MOCK_ON_CALL(motors_hw_move_one_step, [&](motors_direction_t direction) { EXPECT(direction == DIRECTION_3); });
    state = motors_state_machine_update(
        motors_state_t::MOVING_CONTINUOUS, motors_transition_t::START_MOVE_ONE_STEP, DIRECTION_3);
    EXPECT(state == motors_state_t::MOVING_ONE_STEP);

    // Update move one_step : motors are still moving -> stay in state, do nothing
    MINI_MOCK_ON_CALL(motor_hw_get_state, []() { return motor_hw_state_t::MOVING; });
    state = motors_state_machine_update(
        motors_state_t::MOVING_ONE_STEP, motors_transition_t::NONE, motors_direction_t::NONE);
    EXPECT(state == motors_state_t::MOVING_ONE_STEP);

    // Update move one_step : motors are stopped -> stopped
    MINI_MOCK_ON_CALL(motor_hw_get_state, []() { return motor_hw_state_t::STOPPED; });
    state = motors_state_machine_update(
        motors_state_t::MOVING_ONE_STEP, motors_transition_t::NONE, motors_direction_t::NONE);
    EXPECT(state == motors_state_t::STOPPED);

    // Update move one_step : motors do not reply -> go to error
    MINI_MOCK_ON_CALL(motor_hw_get_state, []() { return motor_hw_state_t::UNKNOWN; });
    state = motors_state_machine_update(
        motors_state_t::MOVING_ONE_STEP, motors_transition_t::NONE, motors_direction_t::NONE);
    EXPECT(state == motors_state_t::ERROR);

    // Stop move one_step
    MINI_MOCK_ON_CALL(motors_hw_stop, []() {});
    state = motors_state_machine_update(
        motors_state_t::MOVING_ONE_STEP, motors_transition_t::STOP, motors_direction_t::NONE);
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
