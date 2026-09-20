#include "mbr/ui_projector/ui_projector.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define ROWS(...) {__VA_ARGS__}

static const char *const searching_first[9] = ROWS("SEARCHING FIRST MOUSE","PRESS TO LEARN KEYS","WHILE WAIT CONNECTION","       JOY UP","  JOY    JOY    JOY","  LEFT  PRESS  RIGHT","      JOY DOWN"," KEY A         KEY X"," KEY B         KEY Y");
static const char *const first_connected[9] = ROWS("FIRST MOUSE CONNECTED","       JOY UP","  JOY    JOY    JOY","  LEFT  PRESS  RIGHT","      JOY DOWN","               KEY A","LOCK SCREEN    KEY B"," AND UNLOCK    KEY X","  OPEN HOME -> KEY Y");
static const char *const home_searching[9] = ROWS("SEARCHING SAVED MOUSE"," PAIR NEW MOUSE"," SAVED DEVICES"," LEARN THE KEYS","","KEY B: CANCEL SEARCH","JOY UP / DOWN: SELECT","JOY PRESS: ACCESS","KEY X: HELP");
static const char *const home_searching_help[9] = ROWS("HOME SEARCHING HELP","UNLESS IT IS CANCELED","THE SEARCH WILL TAKE","A FEW SECONDS AND","WILL BE TRIGGERED","EVERY TIME YOU ACCESS","THIS SCREEN.","","ANY KEY: BACK");
static const char *const home_retry[9] = ROWS("DEVICE NOT FOUND"," PAIR NEW MOUSE"," SAVED DEVICES"," LEARN THE KEYS","","KEY A: RETRY SEARCH","JOY UP / DOWN: SELECT","JOY PRESS: ACCESS","KEY X: HELP");
static const char *const home_retry_help[9] = ROWS("HOME RETRY HELP","THE MATCHING ATTEMPT","TOOK PLACE ONLY FOR","DEVICES ALREADY SAVED","IN THE PREFERENCES,","BUT NOT FOR DEVICES","THAT WERE NOT SAVED.","","ANY KEY: BACK");
static const char *const pair_new[9] = ROWS("PAIR NEW MOUSE","TRYING TO CONNECT","A NEW MOUSE THAT","IS NOT LISTED","IN SAVED DEVICES","","KEY B: CANCEL","KEY X: HELP","KEY Y: LOCK");
static const char *const help_pair_new[9] = ROWS("PAIR NEW DEVICE HELP","TO CONNECT A SAVED","DEVICE, FIRST UNPLUG","CURRENTLY CONNECTED","MOUSE AND PRESS THE","KEY B TO BACK UNTIL","SEARCHING APPEARS.","","ANY KEY: BACK");
static const char *const retry_pair_new[9] = ROWS("PAIR NEW MOUSE","NO NEW MOUSE OUTSIDE","THE LIST OF SAVED","DEVICES WAS FOUND","","KEY A: RETRY NEW PAIR","KEY B: BACK TRY SAVED","KEY X: HELP","KEY Y: LOCK");
static const char *const help_retry_pair_new[9] = ROWS("DEVICE NOT FOUND HELP","TO CONNECT A SAVED","DEVICE, FIRST UNPLUG","CURRENTLY CONNECTED","MOUSE AND PRESS THE","KEY B TO BACK UNTIL","SEARCHING APPEARS.","","ANY KEY: BACK");
static const char *const help_home_connected[9] = ROWS("HOME CONNECTED HELP","TO DISCONNECT THE","CURRENTLY CONNECTED","MOUSE, NAVIGATE TO:","SAVED DEVICES >","(MOUSE PAGE) > REMOVE","DEVICE > REMOVE","","ANY KEY: BACK");
static const char *const remapper_options[9] = ROWS("MOUSE OPTIONS"," PASSTHROUGH"," STANDARD REMAP"," ESCAPE REMAP"," CUSTOM REMAP","","JOY PRESS: ACCESS","JOY LEFT: BACK","KEY X: HELP");
static const char *const help_remapper_options[9] = ROWS("REMAPPER OPTIONS HELP","CHOOSE FROM THE","OPTIONS TO CHANGE THE","FUNCTIONS OF THE","MOUSE BUTTONS.","PASSTHROUGH IS THE","DEFAULT OPTION.","","ANY KEY: BACK");
static const char *const passthrough_active[9] = ROWS("PASSTHROUGH ACTIVE","ORIGINAL MOUSE","BUTTONS POSITION","ARE ACTIVE NOW","","","","KEY B: BACK","KEY Y: LOCK");
static const char *const passthrough_not_active[9] = ROWS("APPLY PASSTHROUGH","ORIGINAL MOUSE","BUTTONS POSITION","ARE NOT ACTIVE","","","KEY A: APPLY","KEY B: CANCEL","KEY Y: LOCK");
static const char *const standard_not_active[9] = ROWS("APPLY STANDARD REMAP","FORWARD IS LEFT","LEFT IS FORWARD","BACKWARD IS RIGHT","RIGHT IS BACKWARD","","KEY A: APPLY","KEY B: CANCEL","KEY Y: LOCK");
static const char *const standard_active[9] = ROWS("STANDARD REMAP ACTIVE","FORWARD IS LEFT","LEFT IS FORWARD","BACKWARD IS RIGHT","RIGHT IS BACKWARD","","","KEY B: BACK","KEY Y: LOCK");
static const char *const escape_not_active[9] = ROWS("APPLY ESCAPE REMAP","FORWARD IS LEFT","BACKWARD IS RIGHT","LEFT IS ESCAPE","RIGHT IS BACKWARD","MIDDLE IS FORWARD","","KEY A: APPLY","KEY B: CANCEL");
static const char *const escape_active[9] = ROWS("ESCAPE APPLIED ACTIVE","FORWARD IS LEFT","BACKWARD IS RIGHT","LEFT IS ESCAPE","RIGHT IS BACKWARD","MIDDLE IS FORWARD","","KEY B: BACK","JOY LEFT: GO TO HOME");
static const char *const help_remove_this[9] = ROWS("REMOVE MOUSE HELP","COMPLETELY REMOVE THE","AUTOMATIC CONNECTION","WHEN TURNING ON THE","DEVICE AND DELETE ITS","BUTTON REMAPPING","PROFILE.","","ANY KEY: BACK");
static const char *const learn_keys[9] = ROWS("PRESS TO LEARN KEYS","       JOY UP","  JOY    JOY    JOY","  LEFT  PRESS  RIGHT","      JOY DOWN","               KEY A","LOCK SCREEN    KEY B"," AND UNLOCK    KEY X","  OPEN HOME -> KEY Y");

static void row_set(mbr_ui_row_t *row, const char *text, mbr_ui_color_t color)
{
    if (row == NULL) return;
    memset(row, 0, sizeof(*row));
    if (text != NULL) (void)snprintf(row->text, sizeof(row->text), "%.*s", (int)MBR_UI_SEMANTIC_WIDTH, text);
    row->base_color = color;
}

static void add_span(mbr_ui_row_t *row, size_t start, size_t length, mbr_ui_color_t color)
{
    if (row == NULL || start >= strlen(row->text) || length == 0u || row->span_count >= MBR_UI_MAX_SPANS_PER_ROW) return;
    if (start + length > strlen(row->text)) length = strlen(row->text) - start;
    mbr_ui_span_t *span = &row->spans[row->span_count++];
    span->start = (uint8_t)start;
    span->length = (uint8_t)length;
    span->color = color;
}

static void frame_static(mbr_ui_frame_t *frame, mbr_screen_id_t screen, const char *const rows[9])
{
    frame->screen = screen;
    frame->row_count = MBR_UI_MAX_ROWS;
    for (size_t index = 0u; index < MBR_UI_MAX_ROWS; ++index)
        row_set(&frame->rows[index], rows[index], index == 0u ? MBR_UI_COLOR_TITLE : MBR_UI_COLOR_BODY);
}

void mbr_ui_project_name(const char *source, char output[MBR_UI_SEMANTIC_WIDTH + 1u])
{
    if (output == NULL) return;
    size_t out = 0u;
    bool non_space = false;
    if (source != NULL) {
        const unsigned char *cursor = (const unsigned char *)source;
        while (*cursor != '\0' && out < MBR_UI_SEMANTIC_WIDTH) {
            const unsigned char ch = *cursor++;
            if (ch < 0x20u || ch > 0x7eu) continue;
            output[out++] = (char)ch;
            if (!isspace(ch)) non_space = true;
        }
    }
    output[out] = '\0';
    if (!non_space) (void)snprintf(output, MBR_UI_SEMANTIC_WIDTH + 1u, "UNKNOWN MOUSE");
}

static void apply_selection(mbr_ui_frame_t *frame, size_t first_row, unsigned count, unsigned selection)
{
    for (unsigned index = 0u; index < count; ++index) {
        const size_t row = first_row + index;
        if (index == selection) frame->rows[row].base_color = MBR_UI_COLOR_WHITE;
        else if (frame->rows[row].base_color != MBR_UI_COLOR_CYAN) frame->rows[row].base_color = MBR_UI_COLOR_OPTION;
    }
}

static void apply_pressed_didactic(const mbr_ux_model_t *model, mbr_ui_frame_t *frame)
{
    if (!model->interaction.pressed) return;
    const mbr_control_t control = model->interaction.pressed_control;
    if (model->screen == MBR_SCREEN_SEARCHING_FIRST) {
        switch (control) {
        case MBR_CONTROL_JOY_UP: add_span(&frame->rows[3],7u,6u,MBR_UI_COLOR_WHITE); break;
        case MBR_CONTROL_JOY_LEFT: add_span(&frame->rows[4],2u,3u,MBR_UI_COLOR_WHITE); add_span(&frame->rows[5],2u,4u,MBR_UI_COLOR_WHITE); break;
        case MBR_CONTROL_JOY_PRESS: add_span(&frame->rows[4],9u,3u,MBR_UI_COLOR_WHITE); add_span(&frame->rows[5],8u,5u,MBR_UI_COLOR_WHITE); break;
        case MBR_CONTROL_JOY_RIGHT: add_span(&frame->rows[4],16u,3u,MBR_UI_COLOR_WHITE); add_span(&frame->rows[5],15u,5u,MBR_UI_COLOR_WHITE); break;
        case MBR_CONTROL_JOY_DOWN: add_span(&frame->rows[6],6u,8u,MBR_UI_COLOR_WHITE); break;
        case MBR_CONTROL_KEY_A: add_span(&frame->rows[7],1u,5u,MBR_UI_COLOR_WHITE); break;
        case MBR_CONTROL_KEY_X: add_span(&frame->rows[7],15u,5u,MBR_UI_COLOR_WHITE); break;
        case MBR_CONTROL_KEY_B: add_span(&frame->rows[8],1u,5u,MBR_UI_COLOR_WHITE); break;
        case MBR_CONTROL_KEY_Y: add_span(&frame->rows[8],15u,5u,MBR_UI_COLOR_WHITE); break;
        default: break;
        }
        return;
    }
    if (model->screen == MBR_SCREEN_FIRST_MOUSE_CONNECTED || model->screen == MBR_SCREEN_LEARN_THE_KEYS) {
        switch (control) {
        case MBR_CONTROL_JOY_UP: add_span(&frame->rows[1],7u,6u,MBR_UI_COLOR_WHITE); break;
        case MBR_CONTROL_JOY_LEFT: add_span(&frame->rows[2],2u,3u,MBR_UI_COLOR_WHITE); add_span(&frame->rows[3],2u,4u,MBR_UI_COLOR_WHITE); break;
        case MBR_CONTROL_JOY_PRESS: add_span(&frame->rows[2],9u,3u,MBR_UI_COLOR_WHITE); add_span(&frame->rows[3],8u,5u,MBR_UI_COLOR_WHITE); break;
        case MBR_CONTROL_JOY_RIGHT: add_span(&frame->rows[2],16u,3u,MBR_UI_COLOR_WHITE); add_span(&frame->rows[3],15u,5u,MBR_UI_COLOR_WHITE); break;
        case MBR_CONTROL_JOY_DOWN: add_span(&frame->rows[4],6u,8u,MBR_UI_COLOR_WHITE); break;
        case MBR_CONTROL_KEY_A: add_span(&frame->rows[5],15u,5u,MBR_UI_COLOR_WHITE); break;
        case MBR_CONTROL_KEY_B: add_span(&frame->rows[6],15u,5u,MBR_UI_COLOR_WHITE); break;
        case MBR_CONTROL_KEY_X: add_span(&frame->rows[7],15u,5u,MBR_UI_COLOR_WHITE); break;
        case MBR_CONTROL_KEY_Y: add_span(&frame->rows[8],15u,5u,MBR_UI_COLOR_WHITE); break;
        default: break;
        }
    }
}

static void mark_substring_white(mbr_ui_frame_t *frame, const char *needle)
{
    for (size_t row = 0u; row < frame->row_count; ++row) {
        char *found = strstr(frame->rows[row].text, needle);
        if (found != NULL) { add_span(&frame->rows[row], (size_t)(found-frame->rows[row].text), strlen(needle), MBR_UI_COLOR_WHITE); return; }
    }
}

static void apply_pressed_generic(const mbr_ux_model_t *model, mbr_ui_frame_t *frame)
{
    if (!model->interaction.pressed || model->screen == MBR_SCREEN_SEARCHING_FIRST || model->screen == MBR_SCREEN_FIRST_MOUSE_CONNECTED || model->screen == MBR_SCREEN_LEARN_THE_KEYS) return;
    switch (model->interaction.pressed_control) {
    case MBR_CONTROL_KEY_A: mark_substring_white(frame,"KEY A"); break;
    case MBR_CONTROL_KEY_B: mark_substring_white(frame,"KEY B"); break;
    case MBR_CONTROL_KEY_X: mark_substring_white(frame,"KEY X"); break;
    case MBR_CONTROL_KEY_Y: mark_substring_white(frame,"KEY Y"); break;
    case MBR_CONTROL_JOY_PRESS: mark_substring_white(frame,"JOY PRESS"); break;
    case MBR_CONTROL_JOY_LEFT: mark_substring_white(frame,"LEFT"); break;
    case MBR_CONTROL_JOY_RIGHT: mark_substring_white(frame,"RIGHT"); break;
    case MBR_CONTROL_JOY_UP: mark_substring_white(frame,"UP"); break;
    case MBR_CONTROL_JOY_DOWN: mark_substring_white(frame,"DOWN"); break;
    default: break;
    }
}

static void project_home_connected(const mbr_ux_model_t *model, mbr_ui_frame_t *frame)
{
    static const char *const base[9] = ROWS("MOUSE CONNECTED","",""," SAVED DEVICES"," LEARN THE KEYS","","JOY UP / DOWN: SELECT","JOY PRESS: ACCESS","KEY X: HELP TO REMOVE");
    frame_static(frame, MBR_SCREEN_HOME_CONNECTED, base);
    const mbr_saved_mouse_t *live = mbr_ux_live_mouse(model);
    char name[22]; mbr_ui_project_name(live == NULL ? NULL : live->name, name);
    row_set(&frame->rows[1], name, MBR_UI_COLOR_CYAN);
    char summary[22]; (void)snprintf(summary,sizeof(summary)," %s",mbr_profile_home_summary(live == NULL ? MBR_PROFILE_PASSTHROUGH : live->profile));
    row_set(&frame->rows[2], summary, MBR_UI_COLOR_CYAN);
    frame->rows[3].base_color = MBR_UI_COLOR_OPTION; frame->rows[4].base_color = MBR_UI_COLOR_OPTION;
    apply_selection(frame,2u,3u,model->selection);
}

static void project_remapper(const mbr_ux_model_t *model, mbr_ui_frame_t *frame)
{
    frame_static(frame,MBR_SCREEN_REMAPPER_OPTIONS,remapper_options);
    for(size_t row=1u;row<=4u;++row) frame->rows[row].base_color=MBR_UI_COLOR_OPTION;
    const mbr_saved_mouse_t *live=mbr_ux_live_mouse(model); const unsigned active=live==NULL?0u:(unsigned)live->profile;
    if(active<4u) frame->rows[1u+active].base_color=MBR_UI_COLOR_CYAN;
    apply_selection(frame,1u,4u,model->selection);
}

static void project_custom_edit(const mbr_ux_model_t *model,mbr_ui_frame_t *frame)
{
    frame->screen=MBR_SCREEN_CUSTOM_EDIT; frame->row_count=9u; row_set(&frame->rows[0],"EDIT CUSTOM REMAP",MBR_UI_COLOR_TITLE);
    for(unsigned source=0u;source<MBR_MOUSE_SOURCE_COUNT;++source){char row[22];(void)snprintf(row,sizeof(row)," %s IS %s",mbr_mouse_source_label((mbr_mouse_source_t)source),mbr_mouse_target_label(model->custom_draft.targets[source]));row_set(&frame->rows[1u+source],row,MBR_UI_COLOR_OPTION);}
    row_set(&frame->rows[6],"",MBR_UI_COLOR_BODY); row_set(&frame->rows[7],"JOY PRESS: ACCESS",MBR_UI_COLOR_OPTION); row_set(&frame->rows[8],"KEY A: APPLY CUSTOM",MBR_UI_COLOR_OPTION); apply_selection(frame,1u,MBR_MOUSE_SOURCE_COUNT,model->selection);
}

static void project_source_editor(const mbr_ux_model_t *model,mbr_ui_frame_t *frame)
{
    char title[22];(void)snprintf(title,sizeof(title),"%s WILL BECOME",mbr_mouse_source_label(model->custom_source)); frame->screen=model->screen;frame->row_count=9u;row_set(&frame->rows[0],title,MBR_UI_COLOR_TITLE);
    for(unsigned target=0u;target<MBR_MOUSE_TARGET_COUNT;++target){char row[22];(void)snprintf(row,sizeof(row)," %s",mbr_mouse_target_label((mbr_mouse_target_t)target));row_set(&frame->rows[1u+target],row,MBR_UI_COLOR_OPTION);} row_set(&frame->rows[7],"",MBR_UI_COLOR_BODY);row_set(&frame->rows[8],"KEY A: APPLY AND BACK",MBR_UI_COLOR_OPTION);apply_selection(frame,1u,MBR_MOUSE_TARGET_COUNT,model->selection);
}

static void project_saved_devices(const mbr_ux_model_t *model,mbr_ui_frame_t *frame)
{
    frame->screen=MBR_SCREEN_SAVED_DEVICES;frame->row_count=9u;const mbr_saved_mouse_t *page=mbr_ux_saved_page_mouse(model);char heading[22];const size_t page_number=model->registry.count==0u?0u:model->saved_page+1u;(void)snprintf(heading,sizeof(heading),"%zu OF %zu",page_number,model->registry.count);row_set(&frame->rows[0],heading,MBR_UI_COLOR_TITLE);char name[22];mbr_ui_project_name(page==NULL?NULL:page->name,name);const bool connected=page!=NULL&&model->live.occupied&&model->live.ready&&mbr_mouse_id_equal(page->mouse_id,model->live.session_id.mouse_id);row_set(&frame->rows[1],name,connected?MBR_UI_COLOR_CYAN:MBR_UI_COLOR_BODY);row_set(&frame->rows[2],connected?"STATUS: CONNECTED":"STATUS: DISCONNECTED",MBR_UI_COLOR_BODY);char profile[22];(void)snprintf(profile,sizeof(profile),"PROFILE: %s",mbr_profile_label(page==NULL?MBR_PROFILE_PASSTHROUGH:page->profile));row_set(&frame->rows[3],profile,MBR_UI_COLOR_BODY);row_set(&frame->rows[4]," REMOVE DEVICE",MBR_UI_COLOR_WHITE);row_set(&frame->rows[5],"",MBR_UI_COLOR_BODY);row_set(&frame->rows[6],"JOY RIGHT\\LEFT: PAGE",MBR_UI_COLOR_OPTION);row_set(&frame->rows[7],"JOY PRESS: ACCESS",MBR_UI_COLOR_OPTION);row_set(&frame->rows[8],"KEY B: BACK",MBR_UI_COLOR_OPTION);
}

static void project_remove_this(const mbr_ux_model_t *model,mbr_ui_frame_t *frame)
{
    static const char *const rows[9]=ROWS("REMOVE THIS MOUSE","","","PAIRING AND MAPPINGS","WILL BE DELETED","","KEY A: REMOVE","KEY B: CANCEL","KEY X: HELP");frame_static(frame,MBR_SCREEN_REMOVE_THIS,rows);const mbr_saved_mouse_t *record=mbr_mouse_registry_find(&model->registry,model->remove_mouse_id);char name[22];mbr_ui_project_name(record==NULL?NULL:record->name,name);row_set(&frame->rows[1],name,MBR_UI_COLOR_BODY);frame->rows[6].base_color=MBR_UI_COLOR_OPTION;frame->rows[7].base_color=MBR_UI_COLOR_OPTION;frame->rows[8].base_color=MBR_UI_COLOR_OPTION;
}

void mbr_ui_project(const mbr_ux_model_t *model,mbr_ui_frame_t *frame)
{
    if(model==NULL||frame==NULL)return;memset(frame,0,sizeof(*frame));switch(model->screen){
    case MBR_SCREEN_SEARCHING_FIRST:frame_static(frame,model->screen,searching_first);break;case MBR_SCREEN_FIRST_MOUSE_CONNECTED:frame_static(frame,model->screen,first_connected);break;
    case MBR_SCREEN_HOME_SEARCHING:frame_static(frame,model->screen,home_searching);apply_selection(frame,1u,3u,model->selection);break;case MBR_SCREEN_HOME_SEARCHING_HELP:frame_static(frame,model->screen,home_searching_help);break;
    case MBR_SCREEN_HOME_RETRY:frame_static(frame,model->screen,home_retry);apply_selection(frame,1u,3u,model->selection);break;case MBR_SCREEN_HOME_RETRY_HELP:frame_static(frame,model->screen,home_retry_help);break;
    case MBR_SCREEN_PAIR_NEW:frame_static(frame,model->screen,pair_new);break;case MBR_SCREEN_HELP_PAIR_NEW:frame_static(frame,model->screen,help_pair_new);break;case MBR_SCREEN_RETRY_PAIR_NEW:frame_static(frame,model->screen,retry_pair_new);break;case MBR_SCREEN_HELP_RETRY_PAIR_NEW:frame_static(frame,model->screen,help_retry_pair_new);break;
    case MBR_SCREEN_HOME_CONNECTED:project_home_connected(model,frame);break;case MBR_SCREEN_HELP_HOME_CONNECTED:frame_static(frame,model->screen,help_home_connected);break;case MBR_SCREEN_REMAPPER_OPTIONS:project_remapper(model,frame);break;case MBR_SCREEN_HELP_REMAPPER_OPTIONS:frame_static(frame,model->screen,help_remapper_options);break;
    case MBR_SCREEN_PASSTHROUGH_ACTIVE:frame_static(frame,model->screen,passthrough_active);break;case MBR_SCREEN_PASSTHROUGH_NOT_ACTIVE:frame_static(frame,model->screen,passthrough_not_active);break;case MBR_SCREEN_STANDARD_NOT_ACTIVE:frame_static(frame,model->screen,standard_not_active);break;case MBR_SCREEN_STANDARD_ACTIVE:frame_static(frame,model->screen,standard_active);break;case MBR_SCREEN_ESCAPE_NOT_ACTIVE:frame_static(frame,model->screen,escape_not_active);break;case MBR_SCREEN_ESCAPE_ACTIVE:frame_static(frame,model->screen,escape_active);break;
    case MBR_SCREEN_CUSTOM_EDIT:project_custom_edit(model,frame);break;case MBR_SCREEN_LEFT:case MBR_SCREEN_RIGHT:case MBR_SCREEN_MIDDLE:case MBR_SCREEN_FORWARD:case MBR_SCREEN_BACKWARD:project_source_editor(model,frame);break;case MBR_SCREEN_SAVED_DEVICES:project_saved_devices(model,frame);break;case MBR_SCREEN_REMOVE_THIS:project_remove_this(model,frame);break;case MBR_SCREEN_HELP_REMOVE_THIS:frame_static(frame,model->screen,help_remove_this);break;case MBR_SCREEN_LEARN_THE_KEYS:frame_static(frame,model->screen,learn_keys);break;default:frame_static(frame,MBR_SCREEN_SEARCHING_FIRST,searching_first);break;}
    apply_pressed_didactic(model,frame);apply_pressed_generic(model,frame);
}

mbr_ui_color_t mbr_ui_color_at(const mbr_ui_frame_t *frame,size_t row,size_t column)
{
    if(frame==NULL||row>=frame->row_count||column>=strlen(frame->rows[row].text))return MBR_UI_COLOR_BODY;const mbr_ui_row_t *ui_row=&frame->rows[row];mbr_ui_color_t color=ui_row->base_color;for(size_t index=0u;index<ui_row->span_count;++index){const mbr_ui_span_t *span=&ui_row->spans[index];if(column>=span->start&&column<(size_t)span->start+span->length)color=span->color;}return color;
}
