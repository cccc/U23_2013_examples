#ifndef _SYNTH_H_
#define _SYNTH_H_

#include <stdint.h>

#define Note_c  (Note_c1/2)
#define Note_cs (Note_cs1/2)
#define Note_d  (Note_d1/2)
#define Note_ds (Note_ds1/2)
#define Note_e  (Note_e1/2)
#define Note_f  (Note_f1/2)
#define Note_fs (Note_fs1/2)
#define Note_g  (Note_g1/2)
#define Note_gs (Note_gs1/2)
#define Note_a  (Note_a1/2)
#define Note_as (Note_as1/2)
#define Note_h  (Note_h1/2)

#define Note_c1  264
#define Note_cs1 275
#define Note_d1  297
#define Note_ds1 317
#define Note_e1  330
#define Note_f1  352
#define Note_fs1 367
#define Note_g1  396
#define Note_gs1 422
#define Note_a1  440
#define Note_as1 475
#define Note_h1  495

#define Note_c2  (Note_c1*2)
#define Note_cs2 (Note_cs1*2)
#define Note_d2  (Note_d1*2)
#define Note_ds2 (Note_ds1*2)
#define Note_e2  (Note_e1*2)
#define Note_f2  (Note_f1*2)
#define Note_fs2 (Note_fs1*2)
#define Note_g2  (Note_g1*2)
#define Note_gs2 (Note_gs1*2)
#define Note_a2  (Note_a1*2)
#define Note_as2 (Note_as1*2)
#define Note_h2  (Note_h1*2)

#define Note_Pause 0

typedef enum {
	SynthRect,
	SynthSaw,
	SynthTri
} SynthWave;

typedef struct {
	uint16_t freq;
	uint16_t duration;
	uint8_t volume;
} SynthNote;

typedef struct {
	SynthWave instrument;
	uint32_t length;
	SynthNote *notes;
	uint32_t note;
} SynthChannel;

typedef struct {
	uint32_t samplingFrequency;
	uint32_t channelNum;
	SynthChannel *channels;
	uint32_t pos;
} SynthSong;

int16_t SynthGetSample(SynthSong *song);

#endif
