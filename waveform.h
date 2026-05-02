#ifndef WAVEFORM_H
#define WAVEFORM_H

#include <stddef.h>

typedef struct {
    double timestamp;
    double phase_A_voltage;
    double phase_B_voltage;
    double phase_C_voltage;
    double line_current;
    double frequency;
    double power_factor;
    double thd_percent;
} WaveformSample;

typedef struct {
    double rms;
    double peak_to_peak;
    double dc_offset;
    size_t clipped_count;
    int compliant;
} PhaseMetrics;

typedef struct {
    PhaseMetrics phase[3];
    double frequency_min;
    double frequency_max;
    double frequency_mean;
    double power_factor_min;
    double power_factor_max;
    double power_factor_mean;
    double thd_min;
    double thd_max;
    double thd_mean;
    size_t sample_count;
} AnalysisResults;

double compute_rms(const WaveformSample *samples, size_t count, int phase_index);
double compute_peak_to_peak(const WaveformSample *samples, size_t count, int phase_index);
double compute_dc_offset(const WaveformSample *samples, size_t count, int phase_index);
size_t count_clipped(const WaveformSample *samples, size_t count, int phase_index, double limit);
int check_compliance(double rms, double nominal, double tolerance_pct);
void analyze_waveforms(const WaveformSample *samples, size_t count, AnalysisResults *out_results);

#endif
