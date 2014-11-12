#include "Metric.hpp"

#include <limits.h>
#include <math.h>

using srp::metric::Metric;

Metric::Metric(unsigned int NumSamples) {
  _min = LONG_MAX;
  _max = 0;
  _current_sample = 0;
  _num_samples = NumSamples;
  _running = false;
  _average = 0;
  _stddev = 0;
  _samples = new long[_num_samples];
  for (auto i = 0; i < _num_samples; ++i) {
    _samples[i] = 0;
  }
}

Metric::~Metric() {
  delete[] _samples;
}

void Metric::Enter() {
  if (_running) {
    std::cerr << "Tried to start an already running metric!" << std::endl;
    BUG();
  }
  clock_gettime(CLOCK_REALTIME, &_start);
  _running = true;
}

void Metric::Leave() {
  struct timespec end;
  if (!_running) {
    std::cerr << "Tried to leave a metric that isn't running!" << std::endl;
    BUG();
  }
  clock_gettime(CLOCK_REALTIME, &end);

  long elapsed = (1000000000 * (end.tv_sec - _start.tv_sec)) + (end.tv_nsec - _start.tv_nsec);
  _current_sample = (_current_sample + 1) % _num_samples;
  _samples[_current_sample] = elapsed;

  bool should_update = true;
  if (_current_sample != _max_sample && _current_sample != _min_sample) {
    if (elapsed < _min) {
      _min = elapsed;
      _min_sample = _current_sample;
    }
    if (elapsed > _max) {
      _max = elapsed;
      _max_sample = _current_sample;
    }
    should_update = false;
  } else {
    _min = LONG_MAX;
    _max = 0;
  }

  _average = 0;
  for (auto i = 0; i < _num_samples; ++i) {
    long samp = _samples[i];
    _average += samp;
    if (should_update) {
      if (samp < _min) {
        _min = samp;
      } else if (samp > _max) {
        _max = samp;
      }
    }
  }
  _average /= float(_num_samples);

  _stddev = 0;
  for (auto i = 0; i < _num_samples; ++i) {
    float tmp = (_samples[i] - _average);
    _stddev += tmp * tmp;
  }
  _stddev /= float(_num_samples) - 1;
  _stddev = sqrtf(_stddev);

  _running = false;
}

float Metric::GetAverage() const {
  return _average;
}

float Metric::GetStandardDeviation() const {
  return _stddev;
}
