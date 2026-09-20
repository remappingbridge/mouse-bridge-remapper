from pathlib import Path
import json,re
root=Path(__file__).resolve().parents[1]
modules=json.loads((root/'ci/modules.json').read_text())
for module,allowed in modules.items():
    assert (root/'src'/module).is_dir(),module
    for p in (root/'src'/module).glob('*.c'):
        s=p.read_text()
        assert not re.search(r'#\s*include\s*[<"][^">]+\.c[">]',s),p
        for dependency in re.findall(r'#include "mbr/([^/]+)/',s):
            assert dependency==module or dependency in allowed,(p,dependency)
        for needle,owners in [(r'\btud_\w+|#include "tusb.h"',{'usb_hid'}),(r'#include "hardware/(gpio|spi)',{'hat','renderer'}),(r'#include "btstack|\bcyw43_arch_init',{'bt_runtime','ble_hogp','logitech_hidpp'}),(r'\bflash_range_',{'product_storage'})]:
            assert module in owners or not re.search(needle,s),(p,needle)
        assert not re.search(r'\btud_(connect|disconnect)\s*\(',s),p
        assert not re.search(r'\b(multicore_launch_core1|stdio_init_all)\s*\(',s),p
for forbidden in ['classic_hid','keyboard_transport','hid_aggregator','composite']:
    assert forbidden not in modules
print('architecture boundaries PASS')
# Source ownership includes public headers, so raw SDK types cannot leak through facades.
for module,allowed in modules.items():
    for p in (root/'include/mbr'/module).glob('*.h'):
        s=p.read_text()
        for dependency in re.findall(r'#include "mbr/([^/]+)/',s):
            assert dependency==module or dependency in allowed,(p,dependency)
        assert not re.search(r'#include [<"](?:tusb|btstack|hardware/|pico/)',s),p
config=(root/'include/tusb_config.h').read_text()
for cls in ['CDC','MSC','MIDI','VENDOR']:
    assert f'#define CFG_TUD_{cls} 0' in config
main=(root/'src/application/main.c').read_text()
assert '#ifdef MBR_QUALIFICATION' in main
assert 'mbr_qualification' not in (root/'src/application/application.c').read_text()
