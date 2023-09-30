#include "mini_mock.hpp"
#include "motors_logic.hpp"
#include "motors_hw.hpp"

static const int DEFAULT_RELAXING_PHASE_DURATION_MS = 500;

MINI_MOCK_FUNCTION(motors_hw_init, motor_hw_error_t, (), ());
MINI_MOCK_FUNCTION(motors_hw_stop, void, (), ());
MINI_MOCK_FUNCTION(motors_hw_move_one_step, void, (motors_direction_t direction), (direction));
MINI_MOCK_FUNCTION(motor_hw_get_state, motor_hw_state_t, (), ());



// Use some useful macros because :
// - it prints meaningful expected values instead of meaningless variable (print "ROLL" instead of "exected_top_motor_command")
// - it allows to avoid a lot of repetitions and make test code more readable

#define MOCK_HW_MEASURE(mocked_current_measure) MINI_MOCK_ON_CALL(motor_hw_measure_current, []() {return motors_current_t::mocked_current_measure;})

#define MOCK_HW_START(exected_motor_id,  exected_command) \
    MINI_MOCK_ON_CALL( \
    motors_hw_start, [](motor_hw_motor_id_t motor_id,motor_hw_command_t command) { \
        EXPECT(motor_id == motor_hw_motor_id_t::exected_motor_id); \
        EXPECT(command == motor_hw_command_t::exected_command); \
        return motor_hw_error_t::NO_ERROR; \
    }) \

#define EXPECT_STATUS(expected_state,expected_direction,expected_current) { \
        auto status = motors_logic_get_status(); \
        EXPECT(status.state == motors_state_t::expected_state); \
        EXPECT(status.direction == motors_direction_t::expected_direction); \
        EXPECT(status.current == motors_current_t::expected_current); \
    }

TEST(when_logic_is_initialized_then_hw_is_initialized, []()
{
    MINI_MOCK_ON_CALL(motors_hw_init, []() {
        return motor_hw_error_t::NO_ERROR;
    });
    motors_logic_init(DEFAULT_RELAXING_PHASE_DURATION_MS);
    EXPECT_STATUS(IDLE, NONE, UNKNWON);
});

TEST(when_logic_is_initialized_and_hw_returns_error_then_logic_is_in_error, []()
{
    MINI_MOCK_ON_CALL(motors_hw_init, []() {
        return motor_hw_error_t::CANNOT_USE_UART;
    });
    motors_logic_init(DEFAULT_RELAXING_PHASE_DURATION_MS);
    EXPECT_STATUS(ERROR, NONE, UNKNWON);
});
/*
DISABLED_TEST(when_logic_is_initialized_twice_then_motors_stop_and_logic_is_in_error, []()
{
    MINI_MOCK_ON_CALL(motors_hw_init, []() {
        return motor_hw_error_t::NO_ERROR;
    });
    MINI_MOCK_ON_CALL(motors_hw_stop, []() {
        return motor_hw_error_t::NO_ERROR;
    });
    motors_logic_init(DEFAULT_RELAXING_PHASE_DURATION_MS);
    motors_logic_init(DEFAULT_RELAXING_PHASE_DURATION_MS);
    EXPECT_STATUS(ERROR, NONE, UNKNWON);
});

// Test typical "moving" scenario as a single whole story, with nominal case and basic corner cases
DISABLED_TEST(typical_moving_scenario, []()
{
    static const int RELAXING_PHASE_DURATION_MS = 1000;

    // Initialize with specific relaxing phase duration
    MINI_MOCK_ON_CALL(motors_hw_init, []() {
        return motor_hw_error_t::NO_ERROR;
    });
    motors_logic_init(RELAXING_PHASE_DURATION_MS);

    // Start moving 'LEFT' must start "relaxing" phase : unroll 'DOWN_RIGHT' motor
    MOCK_HW_START(DOWN_RIGHT, UNROLL);
    motors_logic_start_move(motors_direction_t::LEFT, 1000);
    EXPECT_STATUS(MOVING, LEFT, UNKNWON);

    // On periodic updates, motors_logic phase will be adjusted depending on the measured current

    // Stay in "relaxing" phase for the configured phase duration (no hw command expected)
    // current is supposed to decrease in relax phase
    MOCK_HW_MEASURE(HIGH);
    motors_logic_periodic_update(1300);
    MOCK_HW_MEASURE(LOW);
    motors_logic_periodic_update(1600);
    EXPECT_STATUS(MOVING, LEFT, LOW);

    // Change to "tensing" phase after "relaxing" phase duration
    MOCK_HW_MEASURE(LOW);
    MOCK_HW_START(DOWN_LEFT, ROLL);
    motors_logic_periodic_update(2100);

    // If current is LOW : stay in "tensing" phase (no hw command expected)
    MOCK_HW_MEASURE(LOW);
    motors_logic_periodic_update(2400);
    MOCK_HW_MEASURE(LOW);
    motors_logic_periodic_update(2900);

    // If current is HIGH : change to "relaxing" phase
    MOCK_HW_MEASURE(HIGH);
    MOCK_HW_START(DOWN_RIGHT, UNROLL);
    motors_logic_periodic_update(3100);

    // Change direction during the relaxing phase

    // Start moving 'UP_RIGHT' must start "relaxing" phase : unroll 'DOWN_LEFT' motor
    MOCK_HW_START(DOWN_LEFT, UNROLL);
    motors_logic_start_move(motors_direction_t::UP_RIGHT, 3500);
    EXPECT_STATUS(MOVING, UP_RIGHT, UNKNWON);

    // Stay in "relaxing" phase for the configured phase duration (no hw command expected)
    // current is supposed to decrease in relax phase
    MOCK_HW_MEASURE(HIGH);
    motors_logic_periodic_update(3800);
    MOCK_HW_MEASURE(LOW);
    motors_logic_periodic_update(4000);
    EXPECT_STATUS(MOVING, UP_RIGHT, LOW);

    // Change to "tensing" phase after "relax" phase duration
    MOCK_HW_MEASURE(LOW);
    MOCK_HW_START(UP, ROLL);
    motors_logic_periodic_update(4600);

    // Change direction during the tensing phase

    // Start moving 'DOWN_LEFT' must start "relaxing" phase : unroll 'UP' motor
    MOCK_HW_START(UP, UNROLL);
    motors_logic_start_move(motors_direction_t::DOWN_LEFT, 5000);
    EXPECT_STATUS(MOVING, DOWN_LEFT, UNKNWON);

    // Stay in "relaxing" phase for the configured phase duration (no hw command expected)
    MOCK_HW_MEASURE(HIGH);
    motors_logic_periodic_update(5500);
    EXPECT_STATUS(MOVING, DOWN_LEFT, HIGH);

    // If HIGH current is measured at the end of "relax" phase : error should be detected
    MOCK_HW_MEASURE(HIGH);
    MINI_MOCK_ON_CALL(motors_hw_stop, []() {
        return motor_hw_error_t::NO_ERROR;
    });
    motors_logic_periodic_update(6100);
    EXPECT_STATUS(ERROR, NONE, UNKNWON);
});

// Test typical "moving_one_step" scenario as a single whole story, with nominal case and basic corner cases
DISABLED_TEST(typical_moving_one_step_scenario, []()
{
    static const int RELAXING_PHASE_DURATION_MS = 1000;

    // Initialize with specific relaxing phase duration
    MINI_MOCK_ON_CALL(motors_hw_init, []() {
        return motor_hw_error_t::NO_ERROR;
    });
    motors_logic_init(RELAXING_PHASE_DURATION_MS);

    // Start moving one step 'LEFT' must start "relaxing" phase : unroll 'DOWN_RIGHT' motor
    MOCK_HW_START(DOWN_RIGHT, UNROLL);
    motors_logic_start_move_one_step(motors_direction_t::LEFT, 1000);
    EXPECT_STATUS(MOVING_ONE_STEP, LEFT, UNKNWON);

    // On periodic updates, motors_logic phase will be adjusted depending on the measured current

    // Stay in "relaxing" phase for the configured phase duration (no hw command expected)
    // current is supposed to decrease in relax phase
    MOCK_HW_MEASURE(HIGH);
    motors_logic_periodic_update(1300);
    MOCK_HW_MEASURE(LOW);
    motors_logic_periodic_update(1600);
    EXPECT_STATUS(MOVING_ONE_STEP, LEFT, LOW);

    // Change to "tensing" phase after "relaxing" phase duration
    MOCK_HW_MEASURE(LOW);
    MOCK_HW_START(DOWN_LEFT, ROLL);
    motors_logic_periodic_update(2100);

    // If current is LOW : stay in "tensing" phase (no hw command expected)
    MOCK_HW_MEASURE(LOW);
    motors_logic_periodic_update(2400);
    MOCK_HW_MEASURE(LOW);
    motors_logic_periodic_update(2900);

    // If current is HIGH : stop motors and go to 'LOCKED' state
    MOCK_HW_MEASURE(HIGH);
    MINI_MOCK_ON_CALL(motors_hw_stop, []() {
        return motor_hw_error_t::NO_ERROR;
    });
    motors_logic_periodic_update(3100);
    EXPECT_STATUS(LOCKED, NONE, UNKNWON);

});
*/
CREATE_MAIN_ENTRY_POINT();


