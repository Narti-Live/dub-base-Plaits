#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/fw.h"
#include "../src/ui.h"
int app_storage_load(void *p, int np, void *s, int ns) { return 0; }
void app_storage_save(const void *p, int np, const void *s, int ns) {}
static float ab[48];
static void run_ms(int ms){ for(int i=0;i<ms*4;i++) fw_render(ab,12,48000.f); }
static uint32_t *cv; static int H=1600;
static void frame(void){ run_ms(33); ui_render(cv, CW, H); }
static void save(const char *n){ FILE*f=fopen(n,"wb"); fwrite(cv,4,CW*H,f); fclose(f); }
int main(int argc,char**argv){
  cv=malloc(CW*H*4); ui_defaults(); ui_apply(); fw_init(3);
  for(int i=0;i<60;i++) frame();
  /* touch FREQUENCY knob and drag */
  ui_touch(UI_DOWN,0,142,290); for(int i=0;i<10;i++){ ui_touch(UI_MOVE,0,142,290-i*6); frame(); }
  save("build/shot_panel.raw"); ui_touch(UI_UP,0,142,230);
  /* hold left (NEW) */
  ui_touch(UI_DOWN,1,308,128); for(int i=0;i<20;i++) frame(); save("build/shot_hold.raw");
  for(int i=0;i<15;i++) frame(); ui_touch(UI_UP,1,308,128); for(int i=0;i<8;i++) frame();
  ui_touch(UI_DOWN,3,308,128); ui_touch(UI_DOWN,4,416,128); for(int i=0;i<75;i++) frame(); ui_touch(UI_UP,3,308,128); ui_touch(UI_UP,4,416,128);
  for(int i=0;i<10;i++) frame(); save("build/shot_tune.raw");
  ui_touch(UI_DOWN,3,308,128); frame(); frame(); ui_touch(UI_UP,3,308,128); for(int i=0;i<30;i++) frame();
  ui_scroll_to(300); frame(); save("build/shot_strip.raw");
  float pos[4]={1180,1500,2700,3300};
  for(int k=0;k<4;k++){ ui_scroll_to(pos[k]); for(int i=0;i<12;i++) frame(); char n[64]; sprintf(n,"build/shot_viz%d.raw",k); save(n);}
  return 0; }
