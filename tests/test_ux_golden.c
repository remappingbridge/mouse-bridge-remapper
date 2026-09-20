#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "mbr/app/app.h"
#include "mbr/ui_projector/ui_projector.h"

#define ID(v) ((mbr_mouse_id_t){(uint64_t)(v)})
#define SESSION(v,g) ((mbr_mouse_session_id_t){ID(v),(uint32_t)(g)})
#define G(...) ((const char *const[9]){__VA_ARGS__})

static void expect(mbr_ux_model_t *m, mbr_screen_id_t s, const char *const r[9]) {
    mbr_ui_frame_t f; m->screen=s; mbr_ui_project(m,&f); assert(f.screen==s); assert(f.row_count==9u);
    for(size_t i=0;i<9u;i++){if(strcmp(f.rows[i].text,r[i])){fprintf(stderr,"%s row %zu: [%s] != [%s]\n",mbr_screen_name(s),i+1,f.rows[i].text,r[i]);assert(0);}assert(strlen(f.rows[i].text)<=21u);}
}

static void all_screens(void){
    mbr_saved_mouse_t storage[3]; mbr_ux_model_t m; mbr_ux_model_init(&m,storage,3u);
    assert(mbr_mouse_registry_add(&m.registry,(mbr_saved_mouse_t){ID(1),"LOGITECH LIFT",MBR_PROFILE_STANDARD}));
    assert(mbr_mouse_session_slot_promote(&m.live,SESSION(1,1))); m.remove_mouse_id=ID(1); m.saved_page=0;
    expect(&m,MBR_SCREEN_SEARCHING_FIRST,G("SEARCHING FIRST MOUSE","PRESS TO LEARN KEYS","WHILE WAIT CONNECTION","       JOY UP","  JOY    JOY    JOY","  LEFT  PRESS  RIGHT","      JOY DOWN"," KEY A         KEY X"," KEY B         KEY Y"));
    expect(&m,MBR_SCREEN_FIRST_MOUSE_CONNECTED,G("FIRST MOUSE CONNECTED","       JOY UP","  JOY    JOY    JOY","  LEFT  PRESS  RIGHT","      JOY DOWN","               KEY A","LOCK SCREEN    KEY B"," AND UNLOCK    KEY X","  OPEN HOME -> KEY Y"));
    expect(&m,MBR_SCREEN_HOME_SEARCHING,G("SEARCHING SAVED MOUSE"," PAIR NEW MOUSE"," SAVED DEVICES"," LEARN THE KEYS","","KEY B: CANCEL SEARCH","JOY UP / DOWN: SELECT","JOY PRESS: ACCESS","KEY X: HELP"));
    expect(&m,MBR_SCREEN_HOME_SEARCHING_HELP,G("HOME SEARCHING HELP","THE MATCHING ATTEMPT","TOOK PLACE ONLY FOR","DEVICES ALREADY SAVED","IN THE PREFERENCES,","BUT NOT FOR DEVICES","THAT WERE NOT SAVED.","","ANY KEY: BACK"));
    expect(&m,MBR_SCREEN_HOME_RETRY,G("DEVICE NOT FOUND"," PAIR NEW MOUSE"," SAVED DEVICES"," LEARN THE KEYS","","KEY A: RETRY SEARCH","JOY UP / DOWN: SELECT","JOY PRESS: ACCESS","KEY X: HELP"));
    expect(&m,MBR_SCREEN_HOME_RETRY_HELP,G("HOME RETRY HELP","THE MATCHING ATTEMPT","TOOK PLACE ONLY FOR","DEVICES ALREADY SAVED","IN THE PREFERENCES,","BUT NOT FOR DEVICES","THAT WERE NOT SAVED.","","ANY KEY: BACK"));
    expect(&m,MBR_SCREEN_PAIR_NEW,G("PAIR NEW MOUSE","TRYING TO CONNECT","A NEW MOUSE THAT","IS NOT LISTED","IN SAVED DEVICES","","KEY B: CANCEL","KEY X: HELP","KEY Y: LOCK"));
    expect(&m,MBR_SCREEN_HELP_PAIR_NEW,G("PAIR NEW DEVICE HELP","TO CONNECT A SAVED","DEVICE, FIRST UNPLUG","CURRENTLY CONNECTED","MOUSE AND PRESS THE","KEY B TO BACK UNTIL","SEARCHING APPEARS.","","ANY KEY: BACK"));
    expect(&m,MBR_SCREEN_RETRY_PAIR_NEW,G("PAIR NEW MOUSE","NO NEW MOUSE OUTSIDE","THE LIST OF SAVED","DEVICES WAS FOUND","","KEY A: RETRY NEW PAIR","KEY B: BACK TRY SAVED","KEY X: HELP","KEY Y: LOCK"));
    expect(&m,MBR_SCREEN_HELP_RETRY_PAIR_NEW,G("DEVICE NOT FOUND HELP","TO CONNECT A SAVED","DEVICE, FIRST UNPLUG","CURRENTLY CONNECTED","MOUSE AND PRESS THE","KEY B TO BACK UNTIL","SEARCHING APPEARS.","","ANY KEY: BACK"));
    m.selection=0; expect(&m,MBR_SCREEN_HOME_CONNECTED,G("LOGITECH LIFT"," PAIR NEW MOUSE"," REMAPPED TO STANDARD"," SAVED DEVICES"," LEARN THE KEYS","","JOY UP / DOWN: SELECT","JOY PRESS: ACCESS","KEY X: HELP TO REMOVE"));
    expect(&m,MBR_SCREEN_HELP_HOME_CONNECTED,G("HOME CONNECTED HELP","TO DISCONNECT THE","CURRENTLY CONNECTED","MOUSE, NAVIGATE TO:","SAVED DEVICES >","(MOUSE PAGE) > REMOVE","DEVICE > REMOVE","","ANY KEY: BACK"));
    m.selection=0; expect(&m,MBR_SCREEN_REMAPPER_OPTIONS,G("MOUSE OPTIONS"," PASSTHROUGH"," STANDARD REMAP"," ESCAPE REMAP"," CUSTOM REMAP","","JOY PRESS: ACCESS","JOY LEFT: BACK","KEY X: HELP"));
    expect(&m,MBR_SCREEN_HELP_REMAPPER_OPTIONS,G("REMAPPER OPTIONS HELP","CHOOSE FROM THE","OPTIONS TO CHANGE THE","FUNCTIONS OF THE","MOUSE BUTTONS.","PASSTHROUGH IS THE","DEFAULT OPTION.","","ANY KEY: BACK"));
    expect(&m,MBR_SCREEN_PASSTHROUGH_ACTIVE,G("PASSTHROUGH ACTIVE","ORIGINAL MOUSE","BUTTONS POSITION","ARE ACTIVE NOW","","","","KEY B: BACK","KEY Y: LOCK"));
    expect(&m,MBR_SCREEN_PASSTHROUGH_NOT_ACTIVE,G("APPLY PASSTHROUGH","ORIGINAL MOUSE","BUTTONS POSITION","ARE NOT ACTIVE","","","KEY A: APPLY","KEY B: CANCEL","KEY Y: LOCK"));
    expect(&m,MBR_SCREEN_STANDARD_NOT_ACTIVE,G("APPLY STANDARD REMAP","FORWARD IS LEFT","LEFT IS FORWARD","BACKWARD IS RIGHT","RIGHT IS BACKWARD","","KEY A: APPLY","KEY B: CANCEL","KEY Y: LOCK"));
    expect(&m,MBR_SCREEN_STANDARD_ACTIVE,G("STANDARD REMAP ACTIVE","FORWARD IS LEFT","LEFT IS FORWARD","BACKWARD IS RIGHT","RIGHT IS BACKWARD","","","KEY B: BACK","KEY Y: LOCK"));
    expect(&m,MBR_SCREEN_ESCAPE_NOT_ACTIVE,G("APPLY ESCAPE REMAP","FORWARD IS LEFT","BACKWARD IS RIGHT","LEFT IS ESCAPE","RIGHT IS BACKWARD","MIDDLE IS FORWARD","","KEY A: APPLY","KEY B: CANCEL"));
    expect(&m,MBR_SCREEN_ESCAPE_ACTIVE,G("ESCAPE APPLIED ACTIVE","FORWARD IS LEFT","BACKWARD IS RIGHT","LEFT IS ESCAPE","RIGHT IS BACKWARD","MIDDLE IS FORWARD","","KEY B: BACK","JOY LEFT: GO TO HOME"));
    m.selection=0; expect(&m,MBR_SCREEN_CUSTOM_EDIT,G("EDIT CUSTOM REMAP"," LEFT IS LEFT"," RIGHT IS RIGHT"," MIDDLE IS MIDDLE"," FORWARD IS FORWARD"," BACKWARD IS BACKWARD","","JOY PRESS: ACCESS","KEY A: APPLY CUSTOM"));
    for(unsigned s=0;s<5u;s++){m.custom_source=(mbr_mouse_source_t)s;m.selection=m.custom_draft.targets[s];static const mbr_screen_id_t ids[5]={MBR_SCREEN_LEFT,MBR_SCREEN_RIGHT,MBR_SCREEN_MIDDLE,MBR_SCREEN_FORWARD,MBR_SCREEN_BACKWARD};const char *title[5]={"LEFT WILL BECOME","RIGHT WILL BECOME","MIDDLE WILL BECOME","FORWARD WILL BECOME","BACKWARD WILL BECOME"};expect(&m,ids[s],G(title[s]," LEFT"," RIGHT"," MIDDLE"," ESCAPE"," FORWARD"," BACKWARD","","KEY A: APPLY AND BACK"));}
    expect(&m,MBR_SCREEN_SAVED_DEVICES,G("1 OF 1","LOGITECH LIFT","STATUS: CONNECTED","PROFILE: STANDARD"," REMOVE DEVICE","","JOY RIGHT\\LEFT: PAGE","JOY PRESS: ACCESS","KEY B: BACK"));
    expect(&m,MBR_SCREEN_REMOVE_THIS,G("REMOVE THIS MOUSE","LOGITECH LIFT","","PAIRING AND MAPPINGS","WILL BE DELETED","","KEY A: REMOVE","KEY B: CANCEL","KEY X: HELP"));
    expect(&m,MBR_SCREEN_HELP_REMOVE_THIS,G("REMOVE MOUSE HELP","COMPLETELY REMOVE THE","AUTOMATIC CONNECTION","WHEN TURNING ON THE","DEVICE AND DELETE ITS","BUTTON REMAPPING","PROFILE.","","ANY KEY: BACK"));
    expect(&m,MBR_SCREEN_LEARN_THE_KEYS,G("PRESS TO LEARN KEYS","       JOY UP","  JOY    JOY    JOY","  LEFT  PRESS  RIGHT","      JOY DOWN","               KEY A","LOCK SCREEN    KEY B"," AND UNLOCK    KEY X","  OPEN HOME -> KEY Y"));
}

static void policy_and_colors(void){
    char n[22];mbr_ui_project_name("123456789012345678901234",n);assert(!strcmp(n,"123456789012345678901"));mbr_ui_project_name("\x01\x02",n);assert(!strcmp(n,"UNKNOWN MOUSE"));mbr_ui_project_name("   ",n);assert(!strcmp(n,"UNKNOWN MOUSE"));
    for(unsigned s=0;s<MBR_SCREEN_COUNT;s++){const char *x=mbr_screen_name((mbr_screen_id_t)s);assert(!strstr(x,"keyboard"));assert(!strstr(x,"composite"));assert(!strstr(x,"other-devices"));assert(!strstr(x,"multi"));}
    mbr_saved_mouse_t st[2];mbr_ux_model_t m;mbr_ux_effects_t e;mbr_ui_frame_t f;mbr_ux_model_init(&m,st,2u);assert(mbr_mouse_registry_add(&m.registry,(mbr_saved_mouse_t){ID(1),"A",MBR_PROFILE_STANDARD}));assert(mbr_mouse_registry_add(&m.registry,(mbr_saved_mouse_t){ID(2),"B",MBR_PROFILE_ESCAPE}));assert(mbr_mouse_session_slot_promote(&m.live,SESSION(1,1)));
    m.screen=MBR_SCREEN_REMAPPER_OPTIONS;m.selection=0;mbr_ui_project(&m,&f);assert(f.rows[2].base_color==MBR_UI_COLOR_CYAN);m.selection=1;mbr_ui_project(&m,&f);assert(f.rows[2].base_color==MBR_UI_COLOR_WHITE);
    m.screen=MBR_SCREEN_HOME_CONNECTED;m.selection=0;mbr_ui_project(&m,&f);assert(!strcmp(f.rows[0].text,"LOGITECH LIFT"));assert(f.rows[0].base_color==MBR_UI_COLOR_TITLE);assert(!strcmp(f.rows[1].text," PAIR NEW MOUSE"));assert(f.rows[1].base_color==MBR_UI_COLOR_WHITE);assert(f.rows[2].base_color==MBR_UI_COLOR_CYAN);m.selection=1;mbr_ui_project(&m,&f);assert(f.rows[2].base_color==MBR_UI_COLOR_WHITE);
    m.screen=MBR_SCREEN_SAVED_DEVICES;m.saved_page=0;mbr_ui_project(&m,&f);assert(f.rows[1].base_color==MBR_UI_COLOR_CYAN);assert(!strcmp(f.rows[2].text,"STATUS: CONNECTED"));m.saved_page=1;mbr_ui_project(&m,&f);assert(f.rows[1].base_color==MBR_UI_COLOR_BODY);assert(!strcmp(f.rows[2].text,"STATUS: DISCONNECTED"));
    m.screen=MBR_SCREEN_SEARCHING_FIRST;mbr_ui_project(&m,&f);assert((size_t)(strstr(f.rows[3].text,"JOY UP")-f.rows[3].text)+1u==8u);assert((size_t)(strstr(f.rows[5].text,"LEFT")-f.rows[5].text)+1u==3u);assert((size_t)(strstr(f.rows[5].text,"PRESS")-f.rows[5].text)+1u==9u);assert((size_t)(strstr(f.rows[5].text,"RIGHT")-f.rows[5].text)+1u==16u);assert((size_t)(strstr(f.rows[6].text,"JOY DOWN")-f.rows[6].text)+1u==7u);
    mbr_ux_input(&m,MBR_CONTROL_KEY_X,true,&e);mbr_ui_project(&m,&f);assert(mbr_ui_color_at(&f,7u,15u)==MBR_UI_COLOR_WHITE);mbr_ux_input(&m,MBR_CONTROL_KEY_X,false,&e);assert(m.screen==MBR_SCREEN_SEARCHING_FIRST);
}

int main(void){assert(MBR_SCREEN_COUNT==30);all_screens();policy_and_colors();puts("MBR-02 golden UX: PASS");return 0;}
