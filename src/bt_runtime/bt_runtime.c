#include "mbr/bt_runtime/bt_runtime.h"

#include <stdatomic.h>
#include <string.h>

static mbr_bt_runtime_message_t g_queue[MBR_BT_RUNTIME_QUEUE_CAPACITY];
static atomic_uint g_write_sequence = ATOMIC_VAR_INIT(0u);
static atomic_uint g_read_sequence = ATOMIC_VAR_INIT(0u);
static atomic_bool g_overflowed = ATOMIC_VAR_INIT(false);

void mbr_bt_runtime_reset(void)
{
    atomic_store_explicit(&g_read_sequence, 0u, memory_order_relaxed);
    atomic_store_explicit(&g_write_sequence, 0u, memory_order_relaxed);
    atomic_store_explicit(&g_overflowed, false, memory_order_relaxed);
    memset(g_queue, 0, sizeof(g_queue));
}

bool mbr_bt_runtime_publish(uint16_t channel,
                            uint16_t type,
                            const void *payload,
                            uint16_t length)
{
    if (length > MBR_BT_RUNTIME_MESSAGE_PAYLOAD_SIZE ||
        (length > 0u && payload == NULL))
        return false;

    const unsigned int write_sequence =
        atomic_load_explicit(&g_write_sequence, memory_order_relaxed);
    const unsigned int read_sequence =
        atomic_load_explicit(&g_read_sequence, memory_order_acquire);
    if ((unsigned int)(write_sequence - read_sequence) >=
        MBR_BT_RUNTIME_QUEUE_CAPACITY) {
        atomic_store_explicit(&g_overflowed, true, memory_order_release);
        return false;
    }

    mbr_bt_runtime_message_t *message =
        &g_queue[write_sequence % MBR_BT_RUNTIME_QUEUE_CAPACITY];
    message->channel = channel;
    message->type = type;
    message->length = length;
    if (length > 0u)
        memcpy(message->payload, payload, length);
    if (length < MBR_BT_RUNTIME_MESSAGE_PAYLOAD_SIZE)
        memset(message->payload + length, 0,
               MBR_BT_RUNTIME_MESSAGE_PAYLOAD_SIZE - length);

    atomic_store_explicit(&g_write_sequence, write_sequence + 1u,
                          memory_order_release);
    return true;
}

bool mbr_bt_runtime_poll(mbr_bt_runtime_message_t *message)
{
    if (message == NULL) return false;

    const unsigned int read_sequence =
        atomic_load_explicit(&g_read_sequence, memory_order_relaxed);
    const unsigned int write_sequence =
        atomic_load_explicit(&g_write_sequence, memory_order_acquire);
    if (read_sequence == write_sequence) return false;

    *message = g_queue[read_sequence % MBR_BT_RUNTIME_QUEUE_CAPACITY];
    atomic_store_explicit(&g_read_sequence, read_sequence + 1u,
                          memory_order_release);
    return true;
}

bool mbr_bt_runtime_take_overflow(void)
{
    return atomic_exchange_explicit(&g_overflowed, false,
                                    memory_order_acq_rel);
}

#ifndef MBR_PLATFORM_PICO

bool mbr_bt_runtime_start(mbr_bt_runtime_session_setup_fn session_setup)
{
    (void)session_setup;
    return false;
}

#else

#include "btstack.h"
#include "g05_hog_host.h"
#include "pico/cyw43_arch.h"

static mbr_bt_runtime_session_setup_fn g_session_setup;
static bool g_started;

bool mbr_bt_runtime_start(mbr_bt_runtime_session_setup_fn session_setup)
{
    if (g_started || session_setup == NULL) return false;

    mbr_bt_runtime_reset();
    g_session_setup = session_setup;

    if (cyw43_arch_init() != 0) {
        g_session_setup = NULL;
        return false;
    }

    l2cap_init();
    sm_init();
    sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
    sm_set_authentication_requirements(SM_AUTHREQ_SECURE_CONNECTION |
                                       SM_AUTHREQ_BONDING);
    gatt_client_init();
    att_server_init(profile_data, NULL, NULL);

    g_session_setup();
    hci_power_control(HCI_POWER_ON);
    g_started = true;
    return true;
}

#endif
