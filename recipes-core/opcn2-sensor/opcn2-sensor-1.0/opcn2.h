#ifndef OPCN2_H_
#define OPCN2_H_

#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>

typedef struct {
    int fanON;
    int laserON;
    int fanDAC;
    int laserDAC;
} Status;

typedef struct {
    int major;
    int minor;
} Firmware;


typedef struct {
  float pm1;
  float pm25;
  float pm10;
} PMData;

typedef struct {
    // Bin Boundaries
    int bb0;
    int bb1;
    int bb2;
    int bb3;
    int bb4;
    int bb5;
    int bb6;
    int bb7;
    int bb8;
    int bb9;
    int bb10;
    int bb11;
    int bb12;
    int bb13;
    int bb14;

    // Bin Particle Volume (floats)
    float bpv0;
    float bpv1;
    float bpv2;
    float bpv3;
    float bpv4;
    float bpv5;
    float bpv6;
    float bpv7;
    float bpv8;
    float bpv9;
    float bpv10;
    float bpv11;
    float bpv12;
    float bpv13;
    float bpv14;
    float bpv15;

    // Bin Particle Densities (floats)
    float bpd0;
    float bpd1;
    float bpd2;
    float bpd3;
    float bpd4;
    float bpd5;
    float bpd6;
    float bpd7;
    float bpd8;
    float bpd9;
    float bpd10;
    float bpd11;
    float bpd12;
    float bpd13;
    float bpd14;
    float bpd15;

    // Bin Sample Volume Weightings (floats)
    float bsvw0;
    float bsvw1;
    float bsvw2;
    float bsvw3;
    float bsvw4;
    float bsvw5;
    float bsvw6;
    float bsvw7;
    float bsvw8;
    float bsvw9;
    float bsvw10;
    float bsvw11;
    float bsvw12;
    float bsvw13;
    float bsvw14;
    float bsvw15;

    // Gain Scaling Coefficient
    float gsc;

    // Sample Flow Rate (ml/s)
    float sfr;

    // LaserDAC 8 bit int
    unsigned int laser_dac;
    unsigned int fan_dac;

    // Time of Flight to Sample Flow Rate Conversion Factor
    unsigned int tof_sfr;
} ConfigVars;

typedef struct {
  int AMSamplingInterval;
  int AMIntervalCount;
  int AMFanOnIdle;
  int AMLaserOnIdle;
  int AMMaxDataArraysInFile;
  int AMOnlySavePMData;
} ConfigVars2;

typedef struct {
    double bin0;
    double bin1;
    double bin2;
    double bin3;
    double bin4;
    double bin5;
    double bin6;
    double bin7;
    double bin8;
    double bin9;
    double bin10;
    double bin11;
    double bin12;
    double bin13;
    double bin14;
    double bin15;

    // Mass Time-of-Flight
    float bin1MToF;
    float bin3MToF;
    float bin5MToF;
    float bin7MToF;

    // Sample Flow Rate
    float sfr;

    // Either the Temperature or Pressure
    unsigned long temp_pressure;

    // Sampling Period
    float period;

    // Checksum
    unsigned int checksum;

    float pm1;
    float pm25;
    float pm10;
} HistogramData;


int sensor_init(int spi_device, int spi_slave);
int sensor_release();
// methods
bool sensor_ping();
bool sensor_on();
bool sensor_off();

bool sensor_save_config_variables();
bool sensor_enter_bootloader();
bool sensor_set_fan_power(unsigned char value);
bool sensor_set_laser_power(unsigned char value);
bool sensor_toggle_fan(bool state);
bool sensor_toggle_laser(bool state);

int sensor_read_status();
int print_information_string();
ConfigVars sensor_read_configuration_variables();
ConfigVars2 sensor_read_configuration_variables2();
PMData sensor_read_pm_data();
HistogramData sensor_read_histogram(bool convert_to_conc);
void print_histogram_data(const HistogramData *hist);

#endif /* OPCN2_H_ */
