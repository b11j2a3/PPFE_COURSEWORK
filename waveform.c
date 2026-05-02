#include "waveform.h"

#include <float.h>
#include <math.h>

static double get_phase_voltage(const WaveformSample *sample, int phase_index) {
    switch (phase_index) {
        case 0:
            return sample->phase_A_voltage;
        case 1:
            return sample->phase_B_voltage;
        case 2:
            return sample->phase_C_voltage;
        default:
            return sample->phase_A_voltage;
    }
}

double compute_rms(const WaveformSample *samples, size_t count, int phase_index) {
    if (count == 0) {
        return 0.0;
    }

    double sum_sq = 0.0;
    const WaveformSample *ptr = samples;

    for (size_t i = 0; i < count; i++, ptr++) {
        double v = get_phase_voltage(ptr, phase_index);
        sum_sq += v * v;
    }

    return sqrt(sum_sq / (double)count);
}

double compute_peak_to_peak(const WaveformSample *samples, size_t count, int phase_index) {
    if (count == 0) {
        return 0.0;
    }

    double min_v = DBL_MAX;
    double max_v = -DBL_MAX;
    const WaveformSample *ptr = samples;

    for (size_t i = 0; i < count; i++, ptr++) {
        double v = get_phase_voltage(ptr, phase_index);
        if (v < min_v) {
            min_v = v;
        }
        if (v > max_v) {
            max_v = v;
        }
    }

    return max_v - min_v;
}

double compute_dc_offset(const WaveformSample *samples, size_t count, int phase_index) {
    if (count == 0) {
        return 0.0;
    }

    double sum = 0.0;
    const WaveformSample *ptr = samples;

    for (size_t i = 0; i < count; i++, ptr++) {
        sum += get_phase_voltage(ptr, phase_index);
    }

    return sum / (double)count;
}

size_t count_clipped(const WaveformSample *samples, size_t count, int phase_index, double limit) {
    size_t clipped = 0;
    const WaveformSample *ptr = samples;

    for (size_t i = 0; i < count; i++, ptr++) {
        double v = get_phase_voltage(ptr, phase_index);
        if (fabs(v) >= limit) {
            clipped++;
        }
    }

    return clipped;
}

int check_compliance(double rms, double nominal, double tolerance_pct) {
    double lower = nominal * (1.0 - tolerance_pct);
    double upper = nominal * (1.0 + tolerance_pct);

    return (rms >= lower && rms <= upper) ? 1 : 0;
}

void analyze_waveforms(const WaveformSample *samples, size_t count, AnalysisResults *out_results) {
    if (!samples || !out_results) {
        return;
    }

    out_results->sample_count = count;

    for (int phase = 0; phase < 3; phase++) {
        out_results->phase[phase].rms = compute_rms(samples, count, phase);
        out_results->phase[phase].peak_to_peak = compute_peak_to_peak(samples, count, phase);
        out_results->phase[phase].dc_offset = compute_dc_offset(samples, count, phase);
        out_results->phase[phase].clipped_count = count_clipped(samples, count, phase, 324.9);
        out_results->phase[phase].compliant = check_compliance(out_results->phase[phase].rms, 230.0, 0.10);
    }

    if (count == 0) {
        out_results->frequency_min = 0.0;
        out_results->frequency_max = 0.0;
        out_results->frequency_mean = 0.0;
        out_results->power_factor_min = 0.0;
        out_results->power_factor_max = 0.0;
        out_results->power_factor_mean = 0.0;
        out_results->thd_min = 0.0;
        out_results->thd_max = 0.0;
        out_results->thd_mean = 0.0;
        return;
    }

    double freq_min = DBL_MAX;
    double freq_max = -DBL_MAX;
    double freq_sum = 0.0;

    double pf_min = DBL_MAX;
    double pf_max = -DBL_MAX;
    double pf_sum = 0.0;

    double thd_min = DBL_MAX;
    double thd_max = -DBL_MAX;
    double thd_sum = 0.0;

    const WaveformSample *ptr = samples;

    for (size_t i = 0; i < count; i++, ptr++) {
        double freq = ptr->frequency;
        if (freq < freq_min) {
            freq_min = freq;
        }
        if (freq > freq_max) {
            freq_max = freq;
        }
        freq_sum += freq;

        double pf = ptr->power_factor;
        if (pf < pf_min) {
            pf_min = pf;
        }
        if (pf > pf_max) {
            pf_max = pf;
        }
        pf_sum += pf;

        double thd = ptr->thd_percent;
        if (thd < thd_min) {
            thd_min = thd;
        }
        if (thd > thd_max) {
            thd_max = thd;
        }
        thd_sum += thd;
    }

    out_results->frequency_min = freq_min;
    out_results->frequency_max = freq_max;
    out_results->frequency_mean = freq_sum / (double)count;

    out_results->power_factor_min = pf_min;
    out_results->power_factor_max = pf_max;
    out_results->power_factor_mean = pf_sum / (double)count;

    out_results->thd_min = thd_min;
    out_results->thd_max = thd_max;
    out_results->thd_mean = thd_sum / (double)count;
}
