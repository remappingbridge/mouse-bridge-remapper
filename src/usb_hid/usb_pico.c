#include "mbr/usb_hid/usb_hid.h"
#include "tusb.h"
#include <string.h>
static MbrUsbQueue queue;
static bool send(void *ctx,unsigned i,const void *p,size_t n) { (void)ctx;return tud_hid_n_ready((uint8_t)i)&&tud_hid_n_report((uint8_t)i,0,p,(uint16_t)n); }
void mbr_usb_init(void) { mbr_usb_release(&queue);(void)tud_init(0); }
void mbr_usb_task(void) { tud_task();if(tud_mounted()&&!tud_suspended())mbr_usb_drain(&queue,send,NULL); }
bool mbr_usb_submit(const mbr_output_state_t *s) { return mbr_usb_enqueue(&queue,s); }
void mbr_usb_release_all(void) { mbr_usb_release(&queue); }
void tud_mount_cb(void) { mbr_usb_release(&queue); }
void tud_umount_cb(void) { mbr_usb_release(&queue); }
void tud_suspend_cb(bool remote_wakeup_en) { (void)remote_wakeup_en;mbr_usb_release(&queue); }
void tud_resume_cb(void) { mbr_usb_release(&queue); }
const uint8_t *tud_descriptor_device_cb(void) { size_t n;return mbr_usb_device_descriptor(&n); }
const uint8_t *tud_descriptor_configuration_cb(uint8_t index) { size_t n;return index==0?mbr_usb_configuration_descriptor(&n):NULL; }
const uint8_t *tud_hid_descriptor_report_cb(uint8_t i) { size_t n;return mbr_usb_report_descriptor(i,&n); }
const uint16_t *tud_descriptor_string_cb(uint8_t index,uint16_t langid) {
 (void)langid;static uint16_t out[64];if(index==0){out[0]=0x0304;out[1]=0x0409;return out;}
 const char *s=mbr_usb_string(index);if(!s)return NULL;size_t n=strlen(s);if(n>63)n=63;
 out[0]=(uint16_t)(0x0300|((n+1)*2));for(size_t i=0;i<n;++i)out[i+1]=(uint8_t)s[i];return out;
}
uint16_t tud_hid_get_report_cb(uint8_t instance,uint8_t report_id,hid_report_type_t type,uint8_t *buffer,uint16_t length) {
 if(report_id||type!=HID_REPORT_TYPE_INPUT||instance>1)return 0;
 /* Relative deltas are transient; GET_REPORT must never replay motion. */
 uint8_t mouse[5]={queue.last_mouse.bytes[0],0,0,0,0};
 const uint8_t *p=instance?queue.last_escape.bytes:mouse;uint16_t n=instance?8:5;if(n>length)n=length;memcpy(buffer,p,n);return n;
}
void tud_hid_set_report_cb(uint8_t instance,uint8_t id,hid_report_type_t type,const uint8_t *buffer,uint16_t size) { (void)instance;(void)id;(void)type;(void)buffer;(void)size; }
