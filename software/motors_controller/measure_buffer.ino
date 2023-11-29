// ================= Measure buffer =================

// Measures are buffered when an "output" command is running to :
// - print last measures a-posteriori for debug and log purpose (over serial)
// - compute moving average quickly for each new measure

const int MAX_SAMPLES_COUNT = 100; // WARNING : high values can make silent fail anywhere due to out-of-memory silent errors
int measure_buffer[MAX_SAMPLES_COUNT];
unsigned long measure_sum;
int measure_index;

void resetMeasureBuffer()
{
    measure_index = 0;
    measure_sum = 0;
    for (int i = 0; i < MAX_SAMPLES_COUNT; i++) {
        measure_buffer[i] = 0;
    }
}

void printMeasureBuffer(int printed_samples_count)
{
    if (printed_samples_count > MAX_SAMPLES_COUNT)
        printed_samples_count = MAX_SAMPLES_COUNT;
    Serial.println("measure_buffer=");
    int start_index = measure_index + MAX_SAMPLES_COUNT - printed_samples_count; // always positive to avoid negative int modulo
    for (int i = 0; i < printed_samples_count; i++) {
        Serial.println(measure_buffer[(start_index + i) % MAX_SAMPLES_COUNT]);
    }
}

int addMeasureAndComputeAverage(int measure)
{
    int oldest_measure_index = (measure_index + MAX_SAMPLES_COUNT - getSamplesCount()) % MAX_SAMPLES_COUNT; // always positive to avoid negative int modulo
    measure_sum -= measure_buffer[oldest_measure_index];
    measure_sum += measure;
    measure_buffer[measure_index] = measure;

    // Cycling over the full buffer allow to cache all possible measures
    // It's only used for log purpose (see 'm' command)
    measure_index = (measure_index + 1) % MAX_SAMPLES_COUNT;

    // Always divide by samples_count, so average value is under-evaluated when measure starts,
    // it's a simple/hacky way to guarantee that at least samples_count measures are taken before comparing with threshold
    return measure_sum / getSamplesCount();
}
