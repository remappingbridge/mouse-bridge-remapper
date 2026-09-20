#include "mbr/renderer/renderer.h"

#include <string.h>

#ifdef MBR_PLATFORM_PICO
#include <stddef.h>
#include <stdint.h>
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#endif

static const uint8_t k_alpha[26][7] = {
    {0x0e,0x11,0x11,0x1f,0x11,0x11,0x11},{0x1e,0x11,0x11,0x1e,0x11,0x11,0x1e},
    {0x0f,0x10,0x10,0x10,0x10,0x10,0x0f},{0x1e,0x11,0x11,0x11,0x11,0x11,0x1e},
    {0x1f,0x10,0x10,0x1e,0x10,0x10,0x1f},{0x1f,0x10,0x10,0x1e,0x10,0x10,0x10},
    {0x0f,0x10,0x10,0x17,0x11,0x11,0x0f},{0x11,0x11,0x11,0x1f,0x11,0x11,0x11},
    {0x1f,0x04,0x04,0x04,0x04,0x04,0x1f},{0x07,0x02,0x02,0x02,0x12,0x12,0x0c},
    {0x11,0x12,0x14,0x18,0x14,0x12,0x11},{0x10,0x10,0x10,0x10,0x10,0x10,0x1f},
    {0x11,0x1b,0x15,0x15,0x11,0x11,0x11},{0x11,0x19,0x15,0x13,0x11,0x11,0x11},
    {0x0e,0x11,0x11,0x11,0x11,0x11,0x0e},{0x1e,0x11,0x11,0x1e,0x10,0x10,0x10},
    {0x0e,0x11,0x11,0x11,0x15,0x12,0x0d},{0x1e,0x11,0x11,0x1e,0x14,0x12,0x11},
    {0x0f,0x10,0x10,0x0e,0x01,0x01,0x1e},{0x1f,0x04,0x04,0x04,0x04,0x04,0x04},
    {0x11,0x11,0x11,0x11,0x11,0x11,0x0e},{0x11,0x11,0x11,0x11,0x11,0x0a,0x04},
    {0x11,0x11,0x11,0x15,0x15,0x15,0x0a},{0x11,0x11,0x0a,0x04,0x0a,0x11,0x11},
    {0x11,0x11,0x0a,0x04,0x04,0x04,0x04},{0x1f,0x01,0x02,0x04,0x08,0x10,0x1f},
};
static const uint8_t k_digit[10][7] = {
    {0x0e,0x11,0x13,0x15,0x19,0x11,0x0e},{0x04,0x0c,0x04,0x04,0x04,0x04,0x0e},
    {0x0e,0x11,0x01,0x02,0x04,0x08,0x1f},{0x1e,0x01,0x01,0x0e,0x01,0x01,0x1e},
    {0x02,0x06,0x0a,0x12,0x1f,0x02,0x02},{0x1f,0x10,0x10,0x1e,0x01,0x01,0x1e},
    {0x0e,0x10,0x10,0x1e,0x11,0x11,0x0e},{0x1f,0x01,0x02,0x04,0x08,0x08,0x08},
    {0x0e,0x11,0x11,0x0e,0x11,0x11,0x0e},{0x0e,0x11,0x11,0x0f,0x01,0x01,0x0e},
};

static uint8_t glyph_row(char character, uint8_t row)
{
    if (row >= 7u) return 0u;
    if (character >= 'A' && character <= 'Z') return k_alpha[(uint8_t)(character - 'A')][row];
    if (character >= '0' && character <= '9') return k_digit[(uint8_t)(character - '0')][row];
    switch (character) {
    case '-': return row == 3u ? 0x1fu : 0u;
    case ':': return (row == 2u || row == 5u) ? 0x04u : 0u;
    case '.': return row == 6u ? 0x04u : 0u;
    case ',': return row == 6u ? 0x06u : 0u;
    case '=': return (row == 2u || row == 4u) ? 0x1fu : 0u;
    case '/': return row <= 4u ? (uint8_t)(1u << (4u - row)) : 0u;
    case '\\': return row <= 4u ? (uint8_t)(1u << row) : 0u;
    case '(': { static const uint8_t rows[7]={0x02,0x04,0x08,0x08,0x08,0x04,0x02}; return rows[row]; }
    case ')': { static const uint8_t rows[7]={0x08,0x04,0x02,0x02,0x02,0x04,0x08}; return rows[row]; }
    case '>': { static const uint8_t rows[7]={0x10,0x08,0x04,0x02,0x04,0x08,0x10}; return rows[row]; }
    case '<': { static const uint8_t rows[7]={0x01,0x02,0x04,0x08,0x04,0x02,0x01}; return rows[row]; }
    case '?': { static const uint8_t rows[7]={0x0e,0x11,0x01,0x02,0x04,0x00,0x04}; return rows[row]; }
    case '!': return (row < 5u || row == 6u) ? 0x04u : 0u;
    default: return 0u;
    }
}

static bool is_didactic_screen(mbr_screen_id_t screen)
{
    return screen == MBR_SCREEN_SEARCHING_FIRST ||
           screen == MBR_SCREEN_FIRST_MOUSE_CONNECTED ||
           screen == MBR_SCREEN_LEARN_THE_KEYS;
}

static bool starts_with(const char *text, const char *prefix)
{
    return text != NULL && prefix != NULL && strncmp(text, prefix, strlen(prefix)) == 0;
}

static uint8_t first_hint_row(const mbr_ui_frame_t *frame)
{
    if (frame == NULL) return MBR_RENDERER_TEXT_ROWS;
    for (uint8_t row = 1u; row < MBR_RENDERER_TEXT_ROWS; ++row) {
        const char *text = frame->rows[row].text;
        if (starts_with(text, "JOY ") || starts_with(text, "KEY ") || starts_with(text, "ANY KEY")) return row;
    }
    return MBR_RENDERER_TEXT_ROWS;
}

uint16_t mbr_renderer_tone_rgb565(mbr_ui_color_t color)
{
    switch (color) {
    case MBR_UI_COLOR_TITLE: return MBR_COLOR_MAGENTA;
    case MBR_UI_COLOR_BODY: return MBR_COLOR_OFF_WHITE_YELLOW;
    case MBR_UI_COLOR_OPTION: return MBR_COLOR_LIGHT_GRAY;
    case MBR_UI_COLOR_WHITE: return MBR_COLOR_WHITE;
    case MBR_UI_COLOR_CYAN: return MBR_COLOR_CYAN;
    default: return MBR_COLOR_LIGHT_GRAY;
    }
}

uint16_t mbr_renderer_background_rgb565(const mbr_ui_frame_t *frame, size_t row)
{
    if (frame == NULL) return MBR_COLOR_BLACK;
    if (is_didactic_screen(frame->screen)) return MBR_COLOR_DARK_MAGENTA;
    const uint8_t hint = first_hint_row(frame);
    return hint < MBR_RENDERER_TEXT_ROWS && row >= hint ? MBR_COLOR_DARK_MAGENTA : MBR_COLOR_BLACK;
}

static uint16_t standard_body_text_y(size_t row)
{
    const uint16_t first = (uint16_t)(MBR_RENDERER_TEXT_Y + MBR_RENDERER_GLYPH_HEIGHT + MBR_RENDERER_TITLE_BODY_GAP);
    const uint16_t advance = (uint16_t)(MBR_RENDERER_GLYPH_HEIGHT + MBR_RENDERER_BODY_LINE_GAP);
    return (uint16_t)(first + (uint16_t)(row - 1u) * advance);
}

static uint16_t standard_hint_text_y(size_t row)
{
    const uint16_t last = (uint16_t)(MBR_RENDERER_HEIGHT - MBR_RENDERER_HINT_BOTTOM_GAP - MBR_RENDERER_GLYPH_HEIGHT);
    const uint16_t advance = (uint16_t)(MBR_RENDERER_GLYPH_HEIGHT + MBR_RENDERER_HINT_LINE_GAP);
    return (uint16_t)(last - (uint16_t)(MBR_RENDERER_TEXT_ROWS - 1u - row) * advance);
}

uint16_t mbr_renderer_separator_boundary_y(const mbr_ui_frame_t *frame)
{
    if (frame == NULL || is_didactic_screen(frame->screen)) return MBR_RENDERER_HEIGHT;
    const uint8_t hint = first_hint_row(frame);
    if (hint == 0u || hint >= MBR_RENDERER_TEXT_ROWS) return MBR_RENDERER_HEIGHT;
    const uint16_t first_hint_y = standard_hint_text_y(hint);
    if (first_hint_y <= MBR_RENDERER_HINT_TOP_GAP) return 0u;
    return (uint16_t)(first_hint_y - MBR_RENDERER_HINT_TOP_GAP);
}

uint16_t mbr_renderer_text_y(const mbr_ui_frame_t *frame, size_t row)
{
    if (row >= MBR_RENDERER_TEXT_ROWS) return MBR_RENDERER_HEIGHT;
    if (frame != NULL && is_didactic_screen(frame->screen)) {
        if (row == 0u) return MBR_RENDERER_TEXT_Y;
        const uint16_t first = (uint16_t)(MBR_RENDERER_TEXT_Y + MBR_RENDERER_GLYPH_HEIGHT + MBR_RENDERER_TITLE_BODY_GAP);
        const uint16_t advance = (uint16_t)(MBR_RENDERER_GLYPH_HEIGHT + MBR_RENDERER_LEARN_LINE_GAP);
        return (uint16_t)(first + (uint16_t)(row - 1u) * advance);
    }
    if (frame != NULL) {
        const uint8_t hint = first_hint_row(frame);
        if (hint > 0u && hint < MBR_RENDERER_TEXT_ROWS) {
            const uint8_t separator_row = (uint8_t)(hint - 1u);
            if (row > 0u && row < separator_row) return standard_body_text_y(row);
            if (row >= hint) return standard_hint_text_y(row);
        }
    }
    return (uint16_t)(MBR_RENDERER_TEXT_Y + (uint16_t)row * MBR_RENDERER_LINE_ADVANCE);
}

static uint16_t cell_x(size_t column)
{
    return (uint16_t)(MBR_RENDERER_TEXT_X + (uint16_t)column * MBR_RENDERER_CHAR_ADVANCE);
}

static bool draw_cell(const mbr_display_hal_t *display, const mbr_ui_frame_t *frame, size_t row, size_t column)
{
    const char character = frame->rows[row].text[column];
    if (character == ' ') return true;
    uint16_t pixels[MBR_RENDERER_GLYPH_WIDTH * MBR_RENDERER_GLYPH_HEIGHT];
    const mbr_ui_color_t color = mbr_ui_color_at(frame,row,column);
    const uint16_t foreground = mbr_renderer_tone_rgb565(color);
    const uint16_t background = mbr_renderer_background_rgb565(frame,row);
    size_t out = 0u;
    for (uint8_t py = 0u; py < MBR_RENDERER_GLYPH_HEIGHT; ++py) {
        const uint8_t bits = glyph_row(character,(uint8_t)(py / MBR_RENDERER_GLYPH_SCALE));
        for (uint8_t px = 0u; px < MBR_RENDERER_GLYPH_WIDTH; ++px) {
            const uint8_t source_x = (uint8_t)(px / MBR_RENDERER_GLYPH_SCALE);
            const bool on = (bits & (uint8_t)(1u << (4u - source_x))) != 0u;
            pixels[out++] = on ? foreground : background;
        }
    }
    return display->write_rgb565(display->context,cell_x(column),mbr_renderer_text_y(frame,row),MBR_RENDERER_GLYPH_WIDTH,MBR_RENDERER_GLYPH_HEIGHT,pixels);
}

bool mbr_renderer_render(const mbr_display_hal_t *display, const mbr_ui_frame_t *frame)
{
    if (display == NULL || frame == NULL || display->fill_rect == NULL || display->write_rgb565 == NULL) return false;
    if (is_didactic_screen(frame->screen)) {
        if (!display->fill_rect(display->context,0u,0u,MBR_RENDERER_WIDTH,MBR_RENDERER_HEIGHT,MBR_COLOR_DARK_MAGENTA)) return false;
    } else {
        const uint16_t boundary = mbr_renderer_separator_boundary_y(frame);
        if (boundary > 0u && !display->fill_rect(display->context,0u,0u,MBR_RENDERER_WIDTH,boundary,MBR_COLOR_BLACK)) return false;
        if (boundary < MBR_RENDERER_HEIGHT && !display->fill_rect(display->context,0u,boundary,MBR_RENDERER_WIDTH,(uint16_t)(MBR_RENDERER_HEIGHT-boundary),MBR_COLOR_DARK_MAGENTA)) return false;
    }
    for (size_t row = 0u; row < MBR_RENDERER_TEXT_ROWS; ++row)
        for (size_t column = 0u; column < MBR_RENDERER_TEXT_COLS; ++column)
            if (!draw_cell(display,frame,row,column)) return false;
    return true;
}

#ifdef MBR_PLATFORM_PICO
#define LCD_SPI spi1
#define LCD_SPI_BAUD_HZ (10u * 1000u * 1000u)
#define LCD_PIN_DC 8u
#define LCD_PIN_CS 9u
#define LCD_PIN_SCK 10u
#define LCD_PIN_MOSI 11u
#define LCD_PIN_RST 12u
#define LCD_PIN_BL 13u

static void lcd_select(bool selected) { gpio_put(LCD_PIN_CS,!selected); }

static void lcd_command(uint8_t command)
{
    gpio_put(LCD_PIN_DC,false);
    lcd_select(true);
    (void)spi_write_blocking(LCD_SPI,&command,1u);
    lcd_select(false);
}

static void lcd_data(const uint8_t *data,size_t length)
{
    if (data == NULL || length == 0u) return;
    gpio_put(LCD_PIN_DC,true);
    lcd_select(true);
    (void)spi_write_blocking(LCD_SPI,data,length);
    lcd_select(false);
}

static void lcd_data_u8(uint8_t value) { lcd_data(&value,1u); }

static void lcd_window(uint16_t x,uint16_t y,uint16_t width,uint16_t height)
{
    const uint16_t x1=(uint16_t)(x+width-1u), y1=(uint16_t)(y+height-1u);
    uint8_t c[4];
    lcd_command(0x2au);
    c[0]=(uint8_t)(x>>8); c[1]=(uint8_t)x; c[2]=(uint8_t)(x1>>8); c[3]=(uint8_t)x1;
    lcd_data(c,sizeof(c));
    lcd_command(0x2bu);
    c[0]=(uint8_t)(y>>8); c[1]=(uint8_t)y; c[2]=(uint8_t)(y1>>8); c[3]=(uint8_t)y1;
    lcd_data(c,sizeof(c));
    lcd_command(0x2cu);
}

static bool valid_rect(uint16_t x,uint16_t y,uint16_t width,uint16_t height)
{
    return width>0u && height>0u && x<MBR_RENDERER_WIDTH && y<MBR_RENDERER_HEIGHT &&
           (uint32_t)x+width<=MBR_RENDERER_WIDTH && (uint32_t)y+height<=MBR_RENDERER_HEIGHT;
}

static bool pico_fill_rect(void *context,uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint16_t rgb565)
{
    (void)context;
    if (!valid_rect(x,y,width,height)) return false;
    static uint8_t row[MBR_RENDERER_WIDTH*2u];
    for (uint16_t column=0u;column<width;++column) {
        row[(size_t)column*2u]=(uint8_t)(rgb565>>8);
        row[(size_t)column*2u+1u]=(uint8_t)rgb565;
    }
    lcd_window(x,y,width,height);
    gpio_put(LCD_PIN_DC,true);
    lcd_select(true);
    for (uint16_t line=0u;line<height;++line) (void)spi_write_blocking(LCD_SPI,row,(size_t)width*2u);
    lcd_select(false);
    return true;
}

static bool pico_write_rgb565(void *context,uint16_t x,uint16_t y,uint16_t width,uint16_t height,const uint16_t *pixels)
{
    (void)context;
    if (pixels==NULL || !valid_rect(x,y,width,height)) return false;
    lcd_window(x,y,width,height);
    gpio_put(LCD_PIN_DC,true);
    lcd_select(true);
    uint8_t bytes[256u];
    const size_t count=(size_t)width*height;
    size_t offset=0u;
    while (offset<count) {
        size_t chunk=count-offset;
        if (chunk>128u) chunk=128u;
        for (size_t i=0u;i<chunk;++i) {
            const uint16_t p=pixels[offset+i];
            bytes[i*2u]=(uint8_t)(p>>8);
            bytes[i*2u+1u]=(uint8_t)p;
        }
        (void)spi_write_blocking(LCD_SPI,bytes,chunk*2u);
        offset+=chunk;
    }
    lcd_select(false);
    return true;
}

static void configure_panel(void)
{
    lcd_command(0x36u); lcd_data_u8(0x70u);
    lcd_command(0x3au); lcd_data_u8(0x05u);
    lcd_command(0xb2u); const uint8_t porch[]={0x0c,0x0c,0x00,0x33,0x33}; lcd_data(porch,sizeof(porch));
    lcd_command(0xb7u); lcd_data_u8(0x35u);
    lcd_command(0xbbu); lcd_data_u8(0x19u);
    lcd_command(0xc0u); lcd_data_u8(0x2cu);
    lcd_command(0xc2u); lcd_data_u8(0x01u);
    lcd_command(0xc3u); lcd_data_u8(0x12u);
    lcd_command(0xc4u); lcd_data_u8(0x20u);
    lcd_command(0xc6u); lcd_data_u8(0x0fu);
    lcd_command(0xd0u); const uint8_t power[]={0xa4,0xa1}; lcd_data(power,sizeof(power));
    lcd_command(0xe0u); const uint8_t gp[]={0xd0,0x04,0x0d,0x11,0x13,0x2b,0x3f,0x54,0x4c,0x18,0x0d,0x0b,0x1f,0x23}; lcd_data(gp,sizeof(gp));
    lcd_command(0xe1u); const uint8_t gn[]={0xd0,0x04,0x0c,0x11,0x13,0x2c,0x3f,0x44,0x51,0x2f,0x1f,0x1f,0x20,0x23}; lcd_data(gn,sizeof(gn));
    lcd_command(0x21u);
    lcd_command(0x11u);
    sleep_ms(120u);
    lcd_command(0x29u);
}

void mbr_renderer_set_backlight(bool enabled) { gpio_put(LCD_PIN_BL,enabled); }

bool mbr_renderer_init(mbr_display_hal_t *display)
{
    if (display==NULL) return false;
    (void)spi_init(LCD_SPI,LCD_SPI_BAUD_HZ);
    gpio_set_function(LCD_PIN_SCK,GPIO_FUNC_SPI);
    gpio_set_function(LCD_PIN_MOSI,GPIO_FUNC_SPI);
    const uint outputs[]={LCD_PIN_DC,LCD_PIN_CS,LCD_PIN_RST,LCD_PIN_BL};
    for (size_t i=0u;i<sizeof(outputs)/sizeof(outputs[0]);++i) {
        gpio_init(outputs[i]);
        gpio_set_dir(outputs[i],GPIO_OUT);
    }
    gpio_put(LCD_PIN_CS,true);
    gpio_put(LCD_PIN_DC,false);
    gpio_put(LCD_PIN_RST,true);
    mbr_renderer_set_backlight(false);
    sleep_ms(100u);
    gpio_put(LCD_PIN_RST,false);
    sleep_ms(100u);
    gpio_put(LCD_PIN_RST,true);
    sleep_ms(100u);
    configure_panel();
    display->context=NULL;
    display->fill_rect=pico_fill_rect;
    display->write_rgb565=pico_write_rgb565;
    return true;
}
#else
bool mbr_renderer_init(mbr_display_hal_t *display) { (void)display; return false; }
void mbr_renderer_set_backlight(bool enabled) { (void)enabled; }
#endif