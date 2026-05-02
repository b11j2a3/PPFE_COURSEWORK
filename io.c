#include "io.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int line_has_data(const char *line) {
    if (!line) {
        return 0;
    }

    for (const char *p = line; *p != '\0'; p++) {
        if (!isspace((unsigned char)*p)) {
            return 1;
        }
    }

    return 0;
}

static int parse_sample_line(const char *line, WaveformSample *out_sample) {
    if (!line || !out_sample) {
        return 0;
    }

    int matched = sscanf(line,
                         "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
                         &out_sample->timestamp,
                         &out_sample->phase_A_voltage,
                         &out_sample->phase_B_voltage,
                         &out_sample->phase_C_voltage,
                         &out_sample->line_current,
                         &out_sample->frequency,
                         &out_sample->power_factor,
                         &out_sample->thd_percent);

    return matched == 8;
}

static int count_data_rows(FILE *file, size_t *out_count) {
    char line[1024];
    size_t count = 0;

    if (!fgets(line, sizeof(line), file)) {
        *out_count = 0;
        return 0;
    }

    while (fgets(line, sizeof(line), file)) {
        if (line_has_data(line)) {
            count++;
        }
    }

    *out_count = count;
    return 1;
}

int load_waveform_samples(const char *filename, WaveformSample **out_samples, size_t *out_count) {
    if (!filename || !out_samples || !out_count) {
        return 1;
    }

    *out_samples = NULL;
    *out_count = 0;

    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open %s: %s\n", filename, strerror(errno));
        return 1;
    }

    size_t row_count = 0;
    if (!count_data_rows(file, &row_count) || row_count == 0) {
        fprintf(stderr, "No data rows found in %s\n", filename);
        fclose(file);
        return 1;
    }

    rewind(file);

    char line[1024];
    if (!fgets(line, sizeof(line), file)) {
        fprintf(stderr, "Failed to read header from %s\n", filename);
        fclose(file);
        return 1;
    }

    WaveformSample *samples = (WaveformSample *)malloc(row_count * sizeof(WaveformSample));
    if (!samples) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return 1;
    }

    size_t index = 0;
    while (fgets(line, sizeof(line), file)) {
        if (!line_has_data(line)) {
            continue;
        }

        if (index >= row_count) {
            break;
        }

        if (!parse_sample_line(line, &samples[index])) {
            fprintf(stderr, "Malformed data at row %zu in %s\n", index + 2, filename);
            free(samples);
            fclose(file);
            return 1;
        }

        index++;
    }

    fclose(file);

    if (index != row_count) {
        fprintf(stderr, "Expected %zu rows but parsed %zu rows\n", row_count, index);
        free(samples);
        return 1;
    }

    *out_samples = samples;
    *out_count = row_count;
    return 0;
}

int write_results(const char *filename, const AnalysisResults *results) {
    if (!filename || !results) {
        return 1;
    }

    FILE *file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Failed to write %s: %s\n", filename, strerror(errno));
        return 1;
    }

    fprintf(file, "Power Quality Analysis Results\n");
    fprintf(file, "Samples: %zu\n\n", results->sample_count);

    for (int phase = 0; phase < 3; phase++) {
        char phase_name = (char)('A' + phase);
        const PhaseMetrics *metrics = &results->phase[phase];

        fprintf(file, "Phase %c\n", phase_name);
        fprintf(file, "  RMS voltage: %.3f V\n", metrics->rms);
        fprintf(file, "  Peak-to-peak: %.3f V\n", metrics->peak_to_peak);
        fprintf(file, "  DC offset: %.6f V\n", metrics->dc_offset);
        fprintf(file, "  Clipped samples: %zu\n", metrics->clipped_count);
        fprintf(file, "  Compliance (230V +/-10%%): %s\n\n", metrics->compliant ? "COMPLIANT" : "OUT OF RANGE");
    }

    fprintf(file, "Frequency (Hz)\n");
    fprintf(file, "  Min: %.3f\n", results->frequency_min);
    fprintf(file, "  Max: %.3f\n", results->frequency_max);
    fprintf(file, "  Mean: %.3f\n\n", results->frequency_mean);

    fprintf(file, "Power Factor\n");
    fprintf(file, "  Min: %.3f\n", results->power_factor_min);
    fprintf(file, "  Max: %.3f\n", results->power_factor_max);
    fprintf(file, "  Mean: %.3f\n\n", results->power_factor_mean);

    fprintf(file, "THD (%%)\n");
    fprintf(file, "  Min: %.3f\n", results->thd_min);
    fprintf(file, "  Max: %.3f\n", results->thd_max);
    fprintf(file, "  Mean: %.3f\n", results->thd_mean);

    fclose(file);
    return 0;
}
