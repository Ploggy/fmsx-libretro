#ifndef WRAP_NUKEYKT_H
#define WRAP_NUKEYKT_H

#include "opll.h"
#include "libretro.h"

#include <stdint.h>

typedef unsigned char byte;

#define NUKEYKT_REGISTER_PORT 0
#define NUKEYKT_DATA_PORT     1

/*
 * 12/84 cycle waits documented at https://www.msx.org/wiki/MSX-Music_programming
 * imply theoretical max. 28.5 writes per scanline (HPERIOD*2/(12+84) for paired reg+data)
 */
#define MAX_WRITES 32

/*
 * An output sample is generated by NukeYKT every 72 Z80 CPU cycles.
 * The YM2413 actually generates 18 samples in that time; 1 every 4 MSX clock cycles,
 *  but these are spread out over the 9+5(+4 silent cycles) channels.
 * Output sample rate: 3,579,545/72=49,716Hz or 49,7kHz.
 * (Which should be 49716 / 50 = just over 994 samples per 50Hz frame *)
 * Output needs to be resampled for fmsx-libretro to 48kHz.
 *
 * fMSX is not very accurate when it comes to cycles.
 * CPU_HPERIOD ('CPU cycles per HBlank') is set to 228, which translates to 228/(4*18)=~3.2 NukeYKT samples per scanline.
 * To the untrained ear, it appears to be good enough...
 *
 * *) annoyingly, CPU_V313 / Z80_CYCLES_PER_SAMPLE = VPERIOD_PAL/6 / (4*18) = HPERIOD*313/(6*72) = 1368*313/(6*72)
 * = just over 991; 3 samples short. Let's just say fMSX is not cycle-accurate...
 */

// duplicated from MSX.h due to cyclic include
#define HPERIOD      1368
#define VPERIOD_PAL  (HPERIOD*313)
#define CPU_V313     (VPERIOD_PAL/6)

#define CYCLE_COUNT 18
#define Z80_CYCLES_PER_SAMPLE (4*CYCLE_COUNT)
#define NUM_SAMPLES_PER_FRAME (1 + (CPU_V313 / Z80_CYCLES_PER_SAMPLE)) // 992 at 50Hz; 60Hz uses ~830 samples per frame

typedef struct
{
  bool port;
  uint8_t value;
} Write;

typedef struct
{
  opll_t opll;
  int16_t samples[NUM_SAMPLES_PER_FRAME+2]; // 1 'frame' of 20ms (50Hz) at 49,7kHz. +2 for resampler; first and last index.
  unsigned int sample_write_index;
  uint32_t ticksPending;
  Write writes[MAX_WRITES];
  unsigned int port_write_index;
} YM2413_NukeYKT;

void NukeYKT_Reset2413(YM2413_NukeYKT *opll);
void NukeYKT_WritePort2413(YM2413_NukeYKT *opll, bool port, byte value);
void NukeYKT_Sync2413(YM2413_NukeYKT *opll, uint32_t ticks);

#endif