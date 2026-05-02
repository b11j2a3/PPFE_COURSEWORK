#ifndef IO_H
#define IO_H

#include <stddef.h>

#include "waveform.h"

int load_waveform_samples(const char *filename, WaveformSample **out_samples, size_t *out_count);
int write_results(const char *filename, const AnalysisResults *results);

#endif
