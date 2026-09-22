"""Validate RP2350 ARM-S UF2, including Picotool's documented ignore block."""
import sys,struct,hashlib
from pathlib import Path
for arg in sys.argv[1:]:
 p=Path(arg);data=p.read_bytes();assert data and len(data)%512==0,p
 count=len(data)//512;ids=set();addresses=set();sentinels=0;expected=None
 for i in range(count):
  block=data[i*512:(i+1)*512];m0,m1,flags,address,size,number,total,family=struct.unpack_from('<8I',block)
  assert (m0,m1)==(0x0a324655,0x9e5d5157) and struct.unpack_from('<I',block,508)[0]==0x0ab16f30
  assert flags&0x2000 and size==256
  # Pico SDK boot/uf2.h and picotool elf2uf2.cpp: absolute-family ignored block.
  if family==0xe48bff57:
   assert i==0 and flags==0xa000 and address==0x10ffff00 and number==0 and total==2
   assert struct.unpack_from('<I',block,32+256)[0]==0x9957e304
   sentinels+=1
   continue
  assert family==0xe48bff59 and flags==0x2000,hex(family)
  if expected is None: expected=total
  assert total==expected and number not in ids and number<total
  assert 0x10000000<=address<0x10400000 and address%256==0 and address not in addresses
  ids.add(number);addresses.add(address)
 assert len(ids)==expected and count==expected+sentinels and 0x10000000 in addresses
 assert ids==set(range(expected))
 print(p.name,len(data),hashlib.sha256(data).hexdigest(),'RP2350_ARM_S',expected,'payload blocks')
