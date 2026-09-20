#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mbr/ble_hogp/ble_hogp.h"

#define CHECK(expr) do {     if (!(expr)) {         fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__, __LINE__, #expr);         return 1;     } } while (0)

static const uint8_t k_mouse_report_map[] = {
    0x05, 0x01, 0x09, 0x02, 0xa1, 0x01,
    0x05, 0x09, 0x19, 0x01, 0x29, 0x05,
    0x15, 0x00, 0x25, 0x01, 0x75, 0x01,
    0x95, 0x05, 0x81, 0x02,
    0x75, 0x03, 0x95, 0x01, 0x81, 0x01,
    0x05, 0x01, 0x09, 0x30, 0x09, 0x31,
    0x09, 0x38, 0x15, 0x81, 0x25, 0x7f,
    0x75, 0x08, 0x95, 0x03, 0x81, 0x06,
    0x05, 0x0c, 0x0a, 0x38, 0x02,
    0x15, 0x81, 0x25, 0x7f, 0x75, 0x08,
    0x95, 0x01, 0x81, 0x06,
    0xc0
};

static const uint8_t k_keyboard_report_map[] = {
    0x05, 0x01, 0x09, 0x06, 0xa1, 0x01,
    0x05, 0x07, 0x19, 0xe0, 0x29, 0xe7,
    0x15, 0x00, 0x25, 0x01, 0x75, 0x01,
    0x95, 0x08, 0x81, 0x02,
    0xc0
};

static const uint8_t k_report_id_mouse_map[] = {
    0x05, 0x01, 0x09, 0x02, 0xa1, 0x01,
    0x85, 0x02,
    0x05, 0x09, 0x19, 0x01, 0x29, 0x05,
    0x15, 0x00, 0x25, 0x01, 0x75, 0x01,
    0x95, 0x05, 0x81, 0x02,
    0x75, 0x03, 0x95, 0x01, 0x81, 0x01,
    0x05, 0x01, 0x09, 0x30, 0x09, 0x31,
    0x09, 0x38, 0x15, 0x81, 0x25, 0x7f,
    0x75, 0x08, 0x95, 0x03, 0x81, 0x06,
    0xc0
};

typedef struct {
    mbr_mouse_event_t events[16];
    size_t count;
} sink_t;

static bool sink_emit(void *context, const mbr_mouse_event_t *event)
{
    sink_t *sink = context;
    if (sink == NULL || event == NULL || sink->count >= 16u) return false;
    sink->events[sink->count++] = *event;
    return true;
}

static int test_mouse_parser(void)
{
    mbr_ble_hogp_parser_t parser;
    CHECK(mbr_ble_hogp_parser_configure(
        &parser, k_mouse_report_map, sizeof(k_mouse_report_map)));
    CHECK(mbr_ble_hogp_parser_has_mouse(&parser));
    CHECK(parser.report_count == 1u);
    CHECK(parser.field_count == 9u);

    sink_t sink = {0};
    const uint8_t report[5] = {0x11u, 10u, (uint8_t)-5, 1u, (uint8_t)-2};
    CHECK(mbr_ble_hogp_parser_parse_report(
        &parser, 0u, report, sizeof(report), sink_emit, &sink));
    CHECK(sink.count == 4u);
    CHECK(sink.events[0].type == MBR_MOUSE_EVENT_BUTTON);
    CHECK(sink.events[0].data.button.button == MBR_MOUSE_BUTTON_LEFT);
    CHECK(sink.events[0].data.button.pressed);
    CHECK(sink.events[1].type == MBR_MOUSE_EVENT_BUTTON);
    CHECK(sink.events[1].data.button.button == MBR_MOUSE_BUTTON_FORWARD);
    CHECK(sink.events[1].data.button.pressed);
    CHECK(sink.events[2].type == MBR_MOUSE_EVENT_MOVE);
    CHECK(sink.events[2].data.move.dx == 10);
    CHECK(sink.events[2].data.move.dy == -5);
    CHECK(sink.events[3].type == MBR_MOUSE_EVENT_WHEEL);
    CHECK(sink.events[3].data.wheel.vertical == 1);
    CHECK(sink.events[3].data.wheel.horizontal == -2);

    const uint8_t released[5] = {0};
    sink.count = 0u;
    CHECK(mbr_ble_hogp_parser_parse_report(
        &parser, 0u, released, sizeof(released), sink_emit, &sink));
    CHECK(sink.count == 2u);
    CHECK(!sink.events[0].data.button.pressed);
    CHECK(!sink.events[1].data.button.pressed);
    return 0;
}

static int test_report_id_framing(void)
{
    mbr_ble_hogp_parser_t parser;
    CHECK(mbr_ble_hogp_parser_configure(
        &parser, k_report_id_mouse_map, sizeof(k_report_id_mouse_map)));

    const uint8_t framed[5] = {2u, 0x01u, 3u, (uint8_t)-4, 0u};
    const uint8_t *payload = NULL;
    size_t payload_len = 0u;
    CHECK(mbr_ble_hogp_parser_normalize_report(
        &parser, 2u, framed, sizeof(framed), &payload, &payload_len));
    CHECK(payload == &framed[1] && payload_len == 5u);

    sink_t sink = {0};
    CHECK(mbr_ble_hogp_parser_parse_report(
        &parser, 2u, framed, sizeof(framed), sink_emit, &sink));
    CHECK(sink.count == 1u);
    CHECK(sink.events[0].type == MBR_MOUSE_EVENT_MOVE);
    CHECK(sink.events[0].data.move.dx == 3);
    CHECK(sink.events[0].data.move.dy == -4);

    uint8_t bad[6];
    memcpy(bad, framed, sizeof(bad));
    bad[0] = 7u;
    CHECK(!mbr_ble_hogp_parser_normalize_report(
        &parser, 2u, bad, sizeof(bad), &payload, &payload_len));
    return 0;
}

static int test_reject_keyboard_and_malformed(void)
{
    mbr_ble_hogp_parser_t parser;
    CHECK(!mbr_ble_hogp_parser_configure(
        &parser, k_keyboard_report_map, sizeof(k_keyboard_report_map)));
    const uint8_t malformed[] = {0x05u};
    CHECK(!mbr_ble_hogp_parser_configure(
        &parser, malformed, sizeof(malformed)));
    return 0;
}

static int test_runtime_decode(void)
{
    mbr_bt_runtime_message_t message = {0};
    mbr_ble_hogp_event_t event = {0};
    message.channel = MBR_BLE_HOGP_RUNTIME_CHANNEL;
    message.type = MBR_BLE_HOGP_MESSAGE_MOUSE;
    message.length = sizeof(event);
    event.type = MBR_BLE_HOGP_EVENT_MOUSE;
    event.session_id.mouse_id.value = 123u;
    event.session_id.generation = 4u;
    event.mouse.type = MBR_MOUSE_EVENT_MOVE;
    event.mouse.data.move.dx = 3;
    memcpy(message.payload, &event, sizeof(event));
    CHECK(mbr_ble_hogp_decode_runtime_message(&message, &event));
    CHECK(event.session_id.mouse_id.value == 123u);
    CHECK(event.session_id.generation == 4u);
    CHECK(event.mouse.data.move.dx == 3);
    return 0;
}

int main(void)
{
    CHECK(test_mouse_parser() == 0);
    CHECK(test_report_id_framing() == 0);
    CHECK(test_reject_keyboard_and_malformed() == 0);
    CHECK(test_runtime_decode() == 0);
    puts("MBR-05 BLE HOGP Mouse: OK");
    return 0;
}
