#include <stdbool.h>
#include <stdint.h>

#include "boards.h"
#include "nrf_log.h"
#include "nrf_delay.h"
#ifdef SOFTDEVICE_PRESENT
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#endif
#include "nrf_drv_clock.h"
#if NRF_CRYPTO_ENABLED
#include "nrf_crypto.h"
#endif
#include "mem_manager.h"
#include "app_timer.h"
#include "app_button.h"

#if NRF_LOG_ENABLED
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_backend_uart.h"
#endif // NRF_LOG_ENABLED

#include <openthread/instance.h>
#include <openthread/thread.h>
#include <openthread/tasklet.h>
#include <openthread/link.h>
#include <openthread/dataset.h>
#include <openthread/error.h>
#include <openthread/icmp6.h>
#include <openthread/platform/openthread-system.h>
extern "C" {
#include <openthread/platform/platform-softdevice.h>
}

#include <Weave/DeviceLayer/WeaveDeviceLayer.h>
#include <Weave/DeviceLayer/ThreadStackManager.h>
#include <Weave/DeviceLayer/nRF5/GroupKeyStoreImpl.h>
#include <Weave/DeviceLayer/internal/testing/ConfigUnitTest.h>
#include <Weave/DeviceLayer/internal/testing/GroupKeyStoreUnitTest.h>
#include <Weave/DeviceLayer/internal/testing/SystemClockUnitTest.h>

using namespace ::nl;
using namespace ::nl::Inet;
using namespace ::nl::Weave;
using namespace ::nl::Weave::DeviceLayer;


// ================================================================================
// Test App Feature Config
// ================================================================================

#define TIMER_TEST_ENABLED 1
#define BUTTON_TEST_ENABLED 1
#define TEST_TASK_ENABLED 0
#define RUN_UNIT_TESTS 0
#define WOBLE_ENABLED 1
#define OPENTHREAD_ENABLED 1

// ================================================================================
// Logging Support
// ================================================================================

#if NRF_LOG_ENABLED

#define LOGGER_STACK_SIZE (800)
#define LOGGER_PRIORITY 3

static TaskHandle_t sLoggerTaskHandle;

static void LoggerTaskMain(void * arg)
{
    UNUSED_PARAMETER(arg);

    NRF_LOG_INFO("Logging task running");

    while (1)
    {
        NRF_LOG_FLUSH();

        // Wait for a signal that more logging output might be pending.
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}

extern "C" void vApplicationIdleHook( void )
{
    xTaskNotifyGive(sLoggerTaskHandle);
}

namespace nl {
namespace Weave {
namespace DeviceLayer {

/**
 * Called whenever a Weave or LwIP log message is emitted.
 */
void OnLogOutput(void)
{
    xTaskNotifyGive(sLoggerTaskHandle);
}

} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

#endif //NRF_LOG_ENABLED


// ================================================================================
// Test Task
// ================================================================================

#if TEST_TASK_ENABLED

#define TEST_TASK_STACK_SIZE (800)
#define TEST_TASK_PRIORITY 1

static TaskHandle_t sTestTaskHandle;

static void TestTaskAlive()
{
    bsp_board_led_invert(BSP_BOARD_LED_2);
}

static void TestTaskMain(void * pvParameter)
{
    NRF_LOG_INFO("Test task running");
    bsp_board_led_invert(BSP_BOARD_LED_1);

#if RUN_UNIT_TESTS
    Internal::RunSystemClockUnitTest();

    NRF_LOG_INFO("System clock test complete");

    // Test the core configuration interface
    Internal::NRF5Config::RunConfigUnitTest();

    NRF_LOG_INFO("NRF5Config test complete");

    // Test the group key store
    {
        Internal::GroupKeyStoreImpl groupKeyStore;
        err = groupKeyStore.Init();
        APP_ERROR_CHECK(err);
        Internal::RunGroupKeyStoreUnitTest<Internal::GroupKeyStoreImpl>(&groupKeyStore);
    }

    NRF_LOG_INFO("GroupKeyStore test complete");

    NRF_LOG_INFO("Unit tests done");
#endif


    while (true)
    {
        const TickType_t delay = pdMS_TO_TICKS(1000);
        vTaskDelay(delay);
        TestTaskAlive();
    }
}

#endif // TEST_TASK_ENABLED


// ================================================================================
// Test Timer
// ================================================================================

#if TIMER_TEST_ENABLED

#define TIMER_PERIOD_MS 1000

static void TimerEventHandler(void * p_context)
{
    bsp_board_led_invert(BSP_BOARD_LED_3);
}

static void InitTimerTest(void)
{
    ret_code_t ret;

    APP_TIMER_DEF(sTestTimer);

    ret = app_timer_init();
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("app_timer_init() failed");
        APP_ERROR_HANDLER(ret);
    }

    ret = app_timer_create(&sTestTimer, APP_TIMER_MODE_REPEATED, TimerEventHandler);
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("app_timer_create() failed");
        APP_ERROR_HANDLER(ret);
    }

    ret = app_timer_start(sTestTimer, pdMS_TO_TICKS(TIMER_PERIOD_MS), NULL);
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("app_timer_start() failed");
        APP_ERROR_HANDLER(ret);
    }

    NRF_LOG_INFO("Timer test enabled");
}

#endif // TIMER_TEST_ENABLED

// ================================================================================
// Button Test
// ================================================================================

#if BUTTON_TEST_ENABLED

#define TEST_BUTTON_PIN BUTTON_1
#define TEST_BUTTON_DEBOUNCE_PERIOD_MS 50

static void ButtonEventHandler(uint8_t pin_no, uint8_t button_action)
{
    int buttonNum;
    switch (pin_no)
    {
    case BUTTON_1:
        buttonNum = 1;
        break;
    case BUTTON_2:
        buttonNum = 2;
        break;
    case BUTTON_3:
        buttonNum = 4;
        break;
    case BUTTON_4:
        buttonNum = 4;
        break;
    default:
        buttonNum = -1;
        break;
    }
    NRF_LOG_INFO("Button %d %s", buttonNum, (button_action == APP_BUTTON_PUSH) ? "PUSH" : "RELEASE");
}

static void InitButtonTest(void)
{
    ret_code_t ret;

    static app_button_cfg_t sButtons[] =
    {
        { TEST_BUTTON_PIN, APP_BUTTON_ACTIVE_LOW, BUTTON_PULL, ButtonEventHandler }
    };

    ret = app_button_init(sButtons, ARRAY_SIZE(sButtons), pdMS_TO_TICKS(TEST_BUTTON_DEBOUNCE_PERIOD_MS));
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("app_button_init() failed");
        APP_ERROR_HANDLER(ret);
    }

    ret = app_button_enable();
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("app_button_enable() failed");
        APP_ERROR_HANDLER(ret);
    }

    NRF_LOG_INFO("Button test enabled");
}

#endif // BUTTON_TEST_ENABLED


// ================================================================================
// SoftDevice Support
// ================================================================================

#if defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT

static void OnSoCEvent(uint32_t sys_evt, void * p_context)
{
    UNUSED_PARAMETER(p_context);

#if OPENTHREAD_ENABLED

    otSysSoftdeviceSocEvtHandler(sys_evt);

#endif // OPENTHREAD_ENABLED
}

#endif // defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT


// ================================================================================
// J-Link Monitor Mode Debugging Support
// ================================================================================

#if JLINK_MMD

extern "C" void JLINK_MONITOR_OnExit(void)
{

}

extern "C" void JLINK_MONITOR_OnEnter(void)
{

}

extern "C" void JLINK_MONITOR_OnPoll(void)
{

}

#endif // JLINK_MMD



// ================================================================================
// Main Code
// ================================================================================

int main(void)
{
    ret_code_t ret;

#if JLINK_MMD
    NVIC_SetPriority(DebugMonitor_IRQn, _PRIO_SD_LOWEST);
#endif

    // Initialize clock driver.
    ret = nrf_drv_clock_init();
    APP_ERROR_CHECK(ret);

    nrf_drv_clock_lfclk_request(NULL);

    // Wait for the clock to be ready.
    while (!nrf_clock_lf_is_running()) { }

#if NRF_LOG_ENABLED

    // Initialize logging component
    ret = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(ret);

    // Initialize logging backends
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    // Start LOGGER task.
    if (xTaskCreate(LoggerTaskMain, "LOGGER", LOGGER_STACK_SIZE / sizeof(StackType_t), NULL, LOGGER_PRIORITY, &sLoggerTaskHandle) != pdPASS)
    {
        APP_ERROR_HANDLER(0);
    }

#endif

    NRF_LOG_INFO("==================================================");
    NRF_LOG_INFO("openweave-nrf52840-bringup starting");
    NRF_LOG_INFO("==================================================");

    // Configure LED-pins as outputs
    bsp_board_init(BSP_INIT_LEDS);

    bsp_board_led_invert(BSP_BOARD_LED_0);

#if defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT

    NRF_LOG_INFO("Enabling SoftDevice");

    ret = nrf_sdh_enable_request();
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("nrf_sdh_enable_request() failed");
        APP_ERROR_HANDLER(ret);
    }

    NRF_LOG_INFO("Waiting for SoftDevice to be enabled");

    while (!nrf_sdh_is_enabled()) { }

    // Register a handler for SOC events.
    NRF_SDH_SOC_OBSERVER(m_soc_observer, NRF_SDH_SOC_STACK_OBSERVER_PRIO, OnSoCEvent, NULL);

    NRF_LOG_INFO("SoftDevice enable complete");

#endif // defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT

    ret = nrf_mem_init();
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("nrf_mem_init() failed");
        APP_ERROR_HANDLER(ret);
    }

#if NRF_CRYPTO_ENABLED
    ret = nrf_crypto_init();
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("nrf_crypto_init() failed");
        APP_ERROR_HANDLER(ret);
    }
#endif

#if defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT

    {
        uint32_t appRAMStart = 0;

        // Configure the BLE stack using the default settings.
        // Fetch the start address of the application RAM.
        ret = nrf_sdh_ble_default_cfg_set(WEAVE_DEVICE_LAYER_BLE_CONN_CFG_TAG, &appRAMStart);
        APP_ERROR_CHECK(ret);

        // Enable BLE stack.
        ret = nrf_sdh_ble_enable(&appRAMStart);
        APP_ERROR_CHECK(ret);
    }

#endif // defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT

    NRF_LOG_INFO("Initializing Weave stack");

    ret = PlatformMgr().InitWeaveStack();
    if (ret != WEAVE_NO_ERROR)
    {
        NRF_LOG_INFO("PlatformMgr().InitWeaveStack() failed");
        APP_ERROR_HANDLER(ret);
    }

#if !WOBLE_ENABLED

    ret = ConnectivityMgr().SetWoBLEServiceMode(ConnectivityManager::kWoBLEServiceMode_Disabled);
    if (ret != WEAVE_NO_ERROR)
    {
        NRF_LOG_INFO("ConnectivityMgr().SetWoBLEServiceMode() failed");
        APP_ERROR_HANDLER(ret);
    }

#endif // WOBLE_ENABLED

#if OPENTHREAD_ENABLED

    NRF_LOG_INFO("Initializing OpenThread stack");

    otSysInit(0, NULL);

    ret = ThreadStackMgr().InitThreadStack();
    if (ret != WEAVE_NO_ERROR)
    {
        NRF_LOG_INFO("ThreadStackMgr().InitThreadStack() failed");
        APP_ERROR_HANDLER(ret);
    }

#endif // OPENTHREAD_ENABLED

    NRF_LOG_INFO("Starting Weave task");

    ret = PlatformMgr().StartEventLoopTask();
    if (ret != WEAVE_NO_ERROR)
    {
        NRF_LOG_INFO("PlatformMgr().StartEventLoopTask() failed");
        APP_ERROR_HANDLER(ret);
    }

#if OPENTHREAD_ENABLED

    NRF_LOG_INFO("Starting OpenThread task");

    // Start OpenThread task
    ret = ThreadStackMgrImpl().StartThreadTask();
    if (ret != WEAVE_NO_ERROR)
    {
        NRF_LOG_INFO("ThreadStackMgr().StartThreadTask() failed");
        APP_ERROR_HANDLER(ret);
    }

#endif // OPENTHREAD_ENABLED

#if TEST_TASK_ENABLED

    NRF_LOG_INFO("Starting test task");

    // Start Test task
    if (xTaskCreate(TestTaskMain, "TEST", TEST_TASK_STACK_SIZE / sizeof(StackType_t), NULL, TEST_TASK_PRIORITY, &sTestTaskHandle) != pdPASS)
    {
        NRF_LOG_INFO("Failed to create TEST task");
    }

#endif // TEST_TASK_ENABLED

#if TIMER_TEST_ENABLED
    InitTimerTest();
#endif

#if BUTTON_TEST_ENABLED
    InitButtonTest();
#endif

    // Activate deep sleep mode
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    NRF_LOG_INFO("Starting FreeRTOS scheduler");

    /* Start FreeRTOS scheduler. */
    vTaskStartScheduler();

    // Should never get here
    NRF_LOG_INFO("vTaskStartScheduler() failed");
    APP_ERROR_HANDLER(0);
}
