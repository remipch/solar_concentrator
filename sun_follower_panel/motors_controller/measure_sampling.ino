// ================= Measure sampling ================= 


int current_period_ms = 10;
int current_samples_count = 30;

void setSampling(int period_ms, int samples_count) {
  current_period_ms = period_ms;
  Serial.print("  period_ms: ");
  Serial.println(current_period_ms);
  
  if(samples_count > MAX_SAMPLES_COUNT)
    current_samples_count = MAX_SAMPLES_COUNT;
  else
    current_samples_count = samples_count;
  Serial.print("  samples_count: ");
  Serial.println(current_samples_count);
}

int getPeriodMs() {
  return current_period_ms;
}

int getSamplesCount() {
  return current_samples_count;
}

