/*
 *
 *    Copyright (c) 2019 Google LLC.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// ----- Memory Config -----

#define MEM_MANAGER_ENABLED 1
#define MEMORY_MANAGER_SMALL_BLOCK_COUNT 4
#define MEMORY_MANAGER_SMALL_BLOCK_SIZE 32
#define MEMORY_MANAGER_MEDIUM_BLOCK_COUNT 4
#define MEMORY_MANAGER_MEDIUM_BLOCK_SIZE 256
#define MEMORY_MANAGER_LARGE_BLOCK_COUNT 1
#define MEMORY_MANAGER_LARGE_BLOCK_SIZE 1024

// ----- Crypto Config -----

#define NRF_CRYPTO_ENABLED 0

// ----- Soft Device Config -----

#define SOFTDEVICE_PRESENT 1
#define NRF_SDH_ENABLED 1
#define NRF_SDH_SOC_ENABLED 1
#define NRF_SDH_BLE_ENABLED 1
#define NRF_SDH_BLE_PERIPHERAL_LINK_COUNT 1
#define NRF_SDH_BLE_VS_UUID_COUNT 2
#define NRF_BLE_GATT_ENABLED 1
#define NRF_SDH_BLE_GATT_MAX_MTU_SIZE 251
#define NRF_SDH_BLE_GAP_DATA_LENGTH 251

// ----- FDS / Flash Config -----

#define FDS_ENABLED 1
#define FDS_BACKEND NRF_FSTORAGE_SD
#define NRF_FSTORAGE_ENABLED 1

// ----- Logging Config -----

#define NRF_LOG_ENABLED 1
#define NRF_LOG_DEFAULT_LEVEL 4
#define NRF_LOG_STR_PUSH_BUFFER_SIZE 4096
#define NRF_LOG_DEFERRED 0

#define NRF_LOG_BACKEND_RTT_ENABLED 1
#define NRF_LOG_BACKEND_UART_ENABLED 0

#if NRF_LOG_BACKEND_UART_ENABLED

#define NRF_LOG_BACKEND_UART_TX_PIN 6
#define NRF_LOG_BACKEND_UART_BAUDRATE 30801920
#define NRF_LOG_BACKEND_UART_TEMP_BUFFER_SIZE 64

#define UART_ENABLED 1
#define UART0_ENABLED 1
#define NRFX_UARTE_ENABLED 1
#define NRFX_UART_ENABLED 1
#define UART_LEGACY_SUPPORT 0

#endif // NRF_LOG_BACKEND_UART_ENABLED

#if NRF_LOG_BACKEND_RTT_ENABLED

#define SEGGER_RTT_CONFIG_BUFFER_SIZE_UP 2048
#define SEGGER_RTT_CONFIG_MAX_NUM_UP_BUFFERS 2
#define SEGGER_RTT_CONFIG_BUFFER_SIZE_DOWN 16
#define SEGGER_RTT_CONFIG_MAX_NUM_DOWN_BUFFERS 2

// <0=> SKIP
// <1=> TRIM
// <2=> BLOCK_IF_FIFO_FULL
#define SEGGER_RTT_CONFIG_DEFAULT_MODE 1

#define NRF_LOG_BACKEND_RTT_TEMP_BUFFER_SIZE 64
#define NRF_LOG_BACKEND_RTT_TX_RETRY_DELAY_MS 1
#define NRF_LOG_BACKEND_RTT_TX_RETRY_CNT 3

#endif // NRF_LOG_BACKEND_RTT_ENABLED

// ----- Misc Config -----

#define NRF_CLOCK_ENABLED 1
#define NRF_FPRINTF_ENABLED 1
#define NRF_STRERROR_ENABLED 1
#define NRF_QUEUE_ENABLED 1
#define APP_TIMER_ENABLED 1
#define BUTTON_ENABLED 1

#define GPIOTE_ENABLED 1
#define GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS 2

#define APP_TIMER_CONFIG_OP_QUEUE_SIZE 10

// ---- Lock Example App Config ----

#define LOCK_BUTTON                             BUTTON_2
#define FUNCTION_BUTTON                         BUTTON_1
#define FUNCTION_BUTTON_DEBOUNCE_PERIOD_MS      50

#define SYSTEM_STATE_LED                        BSP_LED_0
#define LOCK_STATE_LED                          BSP_LED_1

// Time it takes in ms for the simulated actuator to move from one
// state to another.
#define ACTUATOR_MOVEMENT_PERIOS_MS             2000

#endif //APP_CONFIG_H
