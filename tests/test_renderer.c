#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "mbr/app/app.h"
#include "mbr/hat/hat.h"
#include "mbr/renderer/renderer.h"

typedef struct {
    unsigned fills;
    uint16_t fill_x[4],fill_y[4],fill_w[4],fill_h[4],fill_color[4];
    unsigned writes;
    uint16_t first_x,first_y,first_w,first_h;
    uint16_t first_pixel;
} fake_display_t;

static bool fake_fill(void *context,uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t color)
{
    fake_display_t *f=(fake_display_t *)context;
    if(f->fills<4u){f->fill_x[f->fills]=x;f->fill_y[f->fills]=y;f->fill_w[f->fills]=w;f->fill_h[f->fills]=h;f->fill_color[f->fills]=color;}
    ++f->fills;
    return true;
}

static bool fake_write(void *context,uint16_t x,uint16_t y,uint16_t w,uint16_t h,const uint16_t *pixels)
{
    fake_display_t *f=(fake_display_t *)context;
    if(f->writes==0u){f->first_x=x;f->first_y=y;f->first_w=w;f->first_h=h;f->first_pixel=pixels[0];}
    ++f->writes;
    return true;
}

static void help_frame(mbr_ui_frame_t *frame)
{
    mbr_saved_mouse_t storage[1];
    mbr_ux_model_t model;
    mbr_ux_model_init(&model,storage,1u);
    model.screen=MBR_SCREEN_HOME_SEARCHING_HELP;
    mbr_ui_project(&model,frame);
}

static void test_geometry_and_colors(void)
{
    mbr_ui_frame_t frame;
    fake_display_t fake={0};
    mbr_display_hal_t display={&fake,fake_fill,fake_write};
    help_frame(&frame);
    assert(!strcmp(frame.rows[0].text,"HOME SEARCHING HELP"));
    assert(!strcmp(frame.rows[1].text,"THE MATCHING ATTEMPT"));
    assert(!strcmp(frame.rows[2].text,"TOOK PLACE ONLY FOR"));
    assert(!strcmp(frame.rows[3].text,"DEVICES ALREADY SAVED"));
    assert(!strcmp(frame.rows[4].text,"IN THE PREFERENCES,"));
    assert(!strcmp(frame.rows[5].text,"BUT NOT FOR DEVICES"));
    assert(!strcmp(frame.rows[6].text,"THAT WERE NOT SAVED."));
    assert(frame.rows[7].text[0]=='\0');
    assert(!strcmp(frame.rows[8].text,"ANY KEY: BACK"));
    assert(mbr_renderer_tone_rgb565(MBR_UI_COLOR_TITLE)==MBR_COLOR_MAGENTA);
    assert(mbr_renderer_tone_rgb565(MBR_UI_COLOR_BODY)==MBR_COLOR_OFF_WHITE_YELLOW);
    assert(mbr_renderer_tone_rgb565(MBR_UI_COLOR_OPTION)==MBR_COLOR_LIGHT_GRAY);
    assert(mbr_renderer_tone_rgb565(MBR_UI_COLOR_WHITE)==MBR_COLOR_WHITE);
    assert(mbr_renderer_tone_rgb565(MBR_UI_COLOR_CYAN)==MBR_COLOR_CYAN);
    assert(mbr_renderer_background_rgb565(&frame,0u)==MBR_COLOR_BLACK);
    assert(mbr_renderer_background_rgb565(&frame,8u)==MBR_COLOR_DARK_MAGENTA);
    assert(mbr_renderer_separator_boundary_y(&frame)==203u);
    assert(mbr_renderer_text_y(&frame,0u)==8u);
    assert(mbr_renderer_text_y(&frame,1u)==39u);
    assert(mbr_renderer_text_y(&frame,6u)==169u);
    assert(mbr_renderer_text_y(&frame,8u)==214u);
    assert(mbr_renderer_render(&display,&frame));
    assert(fake.fills==2u);
    assert(fake.fill_x[0]==0u && fake.fill_y[0]==0u && fake.fill_w[0]==240u && fake.fill_h[0]==203u && fake.fill_color[0]==MBR_COLOR_BLACK);
    assert(fake.fill_x[1]==0u && fake.fill_y[1]==203u && fake.fill_w[1]==240u && fake.fill_h[1]==37u && fake.fill_color[1]==MBR_COLOR_DARK_MAGENTA);
    assert(fake.writes>0u);
    assert(fake.first_x==7u && fake.first_y==8u && fake.first_w==10u && fake.first_h==14u);
    assert(fake.first_pixel==MBR_COLOR_MAGENTA);
}

static void test_didactic_layout(void)
{
    mbr_saved_mouse_t storage[1];
    mbr_ux_model_t model;
    mbr_ui_frame_t frame;
    mbr_ux_model_init(&model,storage,1u);
    model.screen=MBR_SCREEN_LEARN_THE_KEYS;
    mbr_ui_project(&model,&frame);
    assert(mbr_renderer_separator_boundary_y(&frame)==240u);
    assert(mbr_renderer_background_rgb565(&frame,0u)==MBR_COLOR_DARK_MAGENTA);
    assert(mbr_renderer_text_y(&frame,1u)==39u);
    assert(mbr_renderer_text_y(&frame,8u)==214u);
}

static void test_hat_map(void)
{
    static const uint8_t pins[MBR_CONTROL_COUNT]={2u,18u,16u,20u,3u,15u,17u,19u,21u};
    for(unsigned i=0u;i<MBR_CONTROL_COUNT;++i){
        assert(mbr_hat_pin_for_control((mbr_control_t)i)==pins[i]);
        mbr_control_t control=MBR_CONTROL_COUNT;
        assert(mbr_hat_control_for_pin(pins[i],&control));
        assert(control==(mbr_control_t)i);
    }
    mbr_control_t invalid=MBR_CONTROL_JOY_UP;
    assert(!mbr_hat_control_for_pin(99u,&invalid));
}

int main(void)
{
    test_geometry_and_colors();
    test_didactic_layout();
    test_hat_map();
    puts("MBR-03 renderer/HAT host contract: PASS");
    return 0;
}