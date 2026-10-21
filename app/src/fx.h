#ifndef FX_H
#define FX_H
typedef struct {
    float echo;      /* echo level 0..1 */
    int time;        /* 0..5: 1/16, 1/8, dotted 1/8, 1/4, dotted 1/4, 1/2 */
    float repeats;   /* feedback 0..1 */
    float reverb;    /* 0..1 */
    float wow;       /* tape wow and flutter 0..1 */
} FxParams;
extern FxParams fx_params;
extern const char *fx_time_names[6];
void fx_reset(void);
void fx_process(float *stereo, int frames, float bpm);
#endif
