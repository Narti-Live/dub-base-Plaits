/* C interface to the firmware core (fw.cc). */
#ifndef DUB_FW_H
#define DUB_FW_H
#include <stdint.h>

#define FW_SCOPE 1024

#ifdef __cplusplus
extern "C" {
#endif

extern volatile float hw_pots[7];   /* FREQ HARM TIMBRE MORPH, T-att FM-att M-att */
extern volatile int hw_buttons[2];

typedef struct {
    int step, num_notes, degree, accent, voicing, root;
    uint32_t block;
    int8_t notes[5];
} FwEvent;

typedef struct {
    int8_t degree[16];
    uint8_t rank[16];
    uint8_t accent[16];
    int8_t chord[16][5];
    uint8_t voicing[16];
    int step, density, length, speed, octave, model, transpose, clocked;
    float envelope, resonance, attack_s, decay_s, env_oct, q;
    float lfo[4];
    float harmonics, timbre, morph;
    float led[8][2];
    float meter;
    float cv_volts, cv_scale;   /* AUX pitch CV */
    int cv_tune;               /* CV TUNE: 0 off, 1 = 0 V, 2 = +2 V */
    uint32_t step_counter, blocks;
    FwEvent last;
} FwSnapshot;

void fw_init(uint32_t seed);
void fw_variant(int *key_offset, int *minor_only, int *voicings);
void fw_render(float *stereo, int frames, float sample_rate);
void fw_set_clock(int mode, float bpm);
void fw_set_mix(float out, float aux);
int fw_snapshot(FwSnapshot *out);
void fw_scope(float *out, int n);

/* implemented by the app */
int app_storage_load(void *persistent, int persistent_size, void *state, int state_size);
void app_storage_save(const void *persistent, int persistent_size, const void *state, int state_size);

#ifdef __cplusplus
}
#endif
#endif
