#include <Synth.h>

static inline int16_t Rect(uint16_t freq, uint32_t pos, uint32_t f_s)
{
	uint32_t period = f_s / freq;

	if ((pos % period) < period/2)
		return INT16_MAX/2;
	else
		return INT16_MIN/2;
}

static inline int16_t Saw(uint16_t freq, uint32_t pos, uint32_t f_s)
{
	uint32_t period = f_s / freq;
	pos %= period;

	return 2 * INT16_MAX * pos / period + INT16_MIN;
}

static inline int16_t Tri(uint16_t freq, uint32_t pos, uint32_t f_s)
{
	uint32_t period = f_s / freq;
	pos %= period;

	if (pos <= period/2)
		return 4 * INT16_MAX * pos / period + INT16_MIN;
	else
		return INT16_MAX - 4 * INT16_MAX * pos / period;
}

int16_t SynthGetSample(SynthSong *song)
{
	SynthChannel *chan = &song->channels[0];
	SynthNote *note = &chan->notes[chan->note];

	int32_t sample = 0;
	switch (chan->instrument) {
	case SynthRect:
		sample = Rect(note->freq, song->pos, song->samplingFrequency);
		break;
	case SynthSaw:
		sample = Saw(note->freq, song->pos, song->samplingFrequency);
		break;
	case SynthTri:
		sample = Tri(note->freq, song->pos, song->samplingFrequency);
		break;
	}

	sample = sample * note->volume / 0xff;

	if (song->pos++ % (song->samplingFrequency*note->duration/1000) == 0)
		chan->note = (chan->note + 1) % chan->length;

	return sample;
}
