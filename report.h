#ifndef WUWA_REPORT_H
#define WUWA_REPORT_H
#include <windows.h>
#include <xinput.h>
/* DualSense USB/full Bluetooth and Bluetooth's initial compact report. */
static int parse_report(const unsigned char *p, unsigned n, XINPUT_GAMEPAD *out) {
 unsigned base, btn; XINPUT_GAMEPAD g = {0};
 if(n >= 78 && p[0] == 0x31) {base=2; btn=9;}
 else if(n >= 64 && p[0] == 1) {base=1; btn=8;}
 else if(n >= 10 && p[0] == 1) {base=1; btn=5;}
 else return 0;
 unsigned char b=p[btn], c=p[btn+1], d=p[btn+2];
 static const WORD hats[9]={1,1|8,8,2|8,2,2|4,4,1|4,0};
 if((b&15)<9) g.wButtons= hats[b&15];
 if(b&0x10)g.wButtons|=XINPUT_GAMEPAD_X;
 if(b&0x20)g.wButtons|=XINPUT_GAMEPAD_A;
 if(b&0x40)g.wButtons|=XINPUT_GAMEPAD_B;
 if(b&0x80)g.wButtons|=XINPUT_GAMEPAD_Y;
 if(c&1)g.wButtons|=XINPUT_GAMEPAD_LEFT_SHOULDER;
 if(c&2)g.wButtons|=XINPUT_GAMEPAD_RIGHT_SHOULDER;
 if(c&0x10)g.wButtons|=XINPUT_GAMEPAD_BACK;
 if(c&0x20)g.wButtons|=XINPUT_GAMEPAD_START;
 if(c&0x40)g.wButtons|=XINPUT_GAMEPAD_LEFT_THUMB;
 if(c&0x80)g.wButtons|=XINPUT_GAMEPAD_RIGHT_THUMB;
 (void)d;
 if(btn==5){g.bLeftTrigger=p[8];g.bRightTrigger=p[9];}
 else {g.bLeftTrigger=p[base+4];g.bRightTrigger=p[base+5];}
 int ax[4]; for(int i=0;i<4;i++){int v=p[base+i];ax[i]=v>=128?(v-128)*32767/127:(v-128)*256;}
 g.sThumbLX=(SHORT)ax[0];g.sThumbLY=(SHORT)(ax[1]==-32768?32767:-ax[1]);
 g.sThumbRX=(SHORT)ax[2];g.sThumbRY=(SHORT)(ax[3]==-32768?32767:-ax[3]);
 *out=g; return 1;
}
#endif
