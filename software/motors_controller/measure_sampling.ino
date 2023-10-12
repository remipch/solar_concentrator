// ================= Measure sampling ================= 


int period_ms = 10;
int samples_count = 30;

void setSampling(int new_period_ms, int new_samples_count) {
  period_ms = new_period_ms;
  
  if(new_samples_count > MAX_SAMPLES_COUNT)
    samples_count = MAX_SAMPLES_COUNT;
  else
    samples_count = new_samples_count;
    
  printSampling();
}

void printSampling() {
  Serial.print("  period_ms: ");
  Serial.println(period_ms);
  Serial.print("  samples_count: ");
  Serial.println(samples_count);
}

int getPeriodMs() {
  return period_ms;
}

int getSamplesCount() {
  return samples_count;
}

