#include "opcn2.h"

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <linux/types.h>
#include <libdigiapix/spi.h>
//#include "rnox/log.h"

/* Avoid "unused parameter" warnings */
#define _UNUSED(x) (void)x;

//static unsigned char _CS = 0;
int _fv;
// attributes
//static Firmware firm_ver;

//#define DEBUG
#ifdef DEBUG
#define DBG(fmt, args...) \
    do \
    { \
    	printf("[spi_sens] %s:%d ",__FUNCTION__,__LINE__); \
        printf(fmt, ## args); \
        printf("\n"); \
    } \
    while(0)
#else
#define DBG(fmt, args...)
#endif

#define CLK_MODE			SPI_CLK_MODE_1
#define CHIP_SELECT			SPI_CS_ACTIVE_LOW
#define BIT_ORDER			SPI_BO_MSB_FIRST
#define MAX_BUS_SPEED		500000 /* 1MHz */
#define BITS_PER_WORD		SPI_BPW_8

static spi_t *spi_dev;


int sensor_init(int spi_device, int spi_slave) {
//    DBG("call");
//	CodeError err = CE_OK;
  // Initiate an instance of the OPCN2 class
  // Ex. OPCN2 alpha(chip_select = A2);
 // _CS = chip_select;

  // Set up SPI
//  SPI.begin(_CS);
//  SPI.setBitOrder(MSBFIRST);
//  SPI.setDataMode(SPI_MODE1);
//  SPI.setClockSpeed(1000000);

  // Set the firmware version
 // _fv = read_information_string().replace(".", "").trim().substring(24, 26).toInt();

	spi_transfer_cfg_t transfer_mode = {0};

	/* Request SPI */
	spi_dev = ldx_spi_request((unsigned int)spi_device, (unsigned int)spi_slave);
	if (!spi_dev) {
		printf("Failed to initialize SPI\n");
		return EXIT_FAILURE;
	}

	/* Configure the transfer mode */
	transfer_mode.clk_mode = CLK_MODE;
	transfer_mode.chip_select = CHIP_SELECT;
	transfer_mode.bit_order = BIT_ORDER;
	if (ldx_spi_set_transfer_mode(spi_dev, &transfer_mode) != EXIT_SUCCESS) {
		printf("Failed to configure SPI transfer mode\n");
		return EXIT_FAILURE;
	}

	/* Configure the bits-per-word */
	if (ldx_spi_set_bits_per_word(spi_dev, BITS_PER_WORD) != EXIT_SUCCESS) {
		printf("Failed to configure SPI bits-per-word\n");
		return EXIT_FAILURE;
	}

	/* Configure the max bus speed */
	if (ldx_spi_set_speed(spi_dev, MAX_BUS_SPEED) != EXIT_SUCCESS) {
		printf("Failed to configure SPI max bus speed\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int sensor_release(){
	DBG("call");
	return 0;
}

uint16_t _16bit_int(char LSB, char MSB)
{
  // Combine two bytes into a 16-bit unsigned int
  return ((MSB << 8) | LSB);
}

bool _compare_arrays(uint8_t array1[], char array2[], int length)
{
  // Compare two arrays and return a boolean

  for (int i = 0; i < length; i++){
    if (array1[i] != array2[i]){
      return false;
    }
  }

  return true;
}

float _calculate_float(char val0, char val1, char val2, char val3)
{
  // Return an IEEE754 float from an array of 4 bytes
  union u_tag {
    char b[4];
    float val;
  } u;

  u.b[0] = val0;
  u.b[1] = val1;
  u.b[2] = val2;
  u.b[3] = val3;

  return u.val;
}

uint32_t _32bit_int(char val0, char val1, char val2, char val3)
{
  // Return a 32-bit unsigned int from 4 bytes
  return ((val3 << 24) | (val2 << 16) | (val1 << 8) | val0);
}

bool sensor_ping()
{
	DBG("call sensor_ping");
	// Isse the check status command
	// ex.
	// $ alpha.ping();
	// $ true
	uint8_t resp[1];
	char expected[] = {0xF3};

	//---digitalWrite(_CS, LOW);       // pull the pin low
	//resp[0] = //---SPI.transfer(0xCF);       // issue the command byte
	//---digitalWrite(_CS, HIGH);      // pull the pin high
	uint8_t cmd = 0xCF;

	if (ldx_spi_transfer(spi_dev, &cmd, resp, 1) == EXIT_FAILURE){
		fprintf(stderr, "Sensor ping error\n");
	}
//    if(verbose) {
//        char buffer[256] = { 0 };
//        log_hex_dump(resp, buffer, 1, SPI_LINE_SIZE, "RX");
//        log_print(LOG_MSG_DEBUG, buffer);
//    }

  return _compare_arrays(resp, expected, 1);
}

bool sensor_on()
{
	DBG("call sensor_on");
	// Turn ON the OPC and return a boolean
	// Ex.
	// $ alpha.on()
	// $ true
	uint8_t vals[2] = {0x0, 0x0};
	char expected[] = {0xF3, 0x03};
	uint8_t cmd = 0x03;

	if (ldx_spi_transfer(spi_dev, &cmd, &vals[0], 1) == EXIT_FAILURE){
		fprintf(stderr, "Sensor_on 1st cmd error\n");
	}


	usleep(100000);
	cmd = 0x00;

	if (ldx_spi_transfer(spi_dev, &cmd, &vals[1], 1) == EXIT_FAILURE){
		fprintf(stderr, "Sensor_on 2nd cmd error\n");
	}
//    if(verbose) {
//        char buffer[256] = { 0 };
//        log_hex_dump(vals, buffer, 2, SPI_LINE_SIZE, "RX Sens ON");
//        log_print(LOG_MSG_DEBUG, buffer);
//    }


  //---digitalWrite(_CS, LOW);
  //vals[0] = //---SPI.transfer(0x03);
  //---digitalWrite(_CS, HIGH);



  //---digitalWrite(_CS, LOW);
  //vals[1] = //---SPI.transfer(0x00);
  //---digitalWrite(_CS, HIGH);

  return _compare_arrays(vals, expected, 2);
}

bool sensor_off()
{
	DBG("call sensor_off");
	// Turn OFF the OPC and return a boolean
	// Ex.
	// $ alpha.off()
	// $ true
	uint8_t vals[2];
	char expected[] = {0xF3, 0x03};

	//---digitalWrite(_CS, LOW);
	//vals[0] = //---SPI.transfer(0x03);
	//---digitalWrite(_CS, HIGH);
	uint8_t cmd = 0x03;

	if (ldx_spi_transfer(spi_dev, &cmd, &vals[0], 1) == EXIT_FAILURE){
		fprintf(stderr, "Sensor_off 1st cmd error\n");
	}


	usleep(100000);
	cmd = 0x01;

  	if (ldx_spi_transfer(spi_dev, &cmd, &vals[1], 1) == EXIT_FAILURE){
  		fprintf(stderr, "Sensor_off 2nd cmd error\n");
  	}
//    if(verbose) {
//        char buffer[256] = { 0 };
//        log_hex_dump(vals, buffer, 2, SPI_LINE_SIZE, "RX Sens OFF");
//        log_print(LOG_MSG_DEBUG, buffer);
//    }

  //---digitalWrite(_CS, LOW);
  //vals[1] = //---SPI.transfer(0x01);
  //---digitalWrite(_CS, HIGH);

  	if (ldx_spi_free(spi_dev) == EXIT_FAILURE) {
  		printf("Failed to release SPI");
  	}

  	return _compare_arrays(vals, expected, 2);
}



int sensor_read_status(Status *data)
{
	DBG("call sensor_read_status");
	// Read key status variables from the OPC-N2
	// Only available with Alphasense OPC-N2 > firmware v18
	// Ex.
	// $ alpha.read_status()
	uint8_t vals[4];
	uint8_t cmd = 0x13;
	int i;

	// Read the status
	//---digitalWrite(_CS, LOW);
	//---SPI.transfer(0x13);
	//---digitalWrite(_CS, HIGH);

	if (ldx_spi_transfer(spi_dev, &cmd, &vals[0], 1) == EXIT_FAILURE) {
		fprintf(stderr, "sensor_read_status 1st cmd error\n");
		return EXIT_FAILURE;
	}

	usleep(10000);

	// Send the read command and build the array of data
	//---digitalWrite(_CS, LOW);
	for (i = 0; i < 4; i++){
	  //vals[i] = //---SPI.transfer(0x13);
	  //delayMicroseconds(4);
		cmd = 0x13;
		if (ldx_spi_transfer(spi_dev, &cmd, &vals[i], 1) == EXIT_FAILURE) {
			fprintf(stderr, "sensor_read_status 2nd cmd error\n");
			return EXIT_FAILURE;
		}
	}

//    if(verbose) {
//        char buffer[256] = { 0 };
//        log_hex_dump(vals, buffer, 4, SPI_LINE_SIZE, "RX STATUS");
//        log_print(LOG_MSG_DEBUG, buffer);
//    }
	//---digitalWrite(_CS, HIGH);

	// Calculate the values!
	data->fanON    = (unsigned int)vals[0];
	data->laserON  = (unsigned int)vals[1];
	data->fanDAC   = (unsigned int)vals[2];
	data->laserDAC = (unsigned int)vals[3];

	return EXIT_SUCCESS;
}

bool sensor_save_config_variables()
{
	DBG("call");
  // Save the configuration variables
  // Ex.
  // $ alpha.save_config_variables()
  uint8_t resp[6];
  char commands[] = {0x43, 0x3F, 0x3C, 0x3F, 0x3C, 0x43};
  char expected[] = {0xF3, 0x43, 0x3f, 0x3c, 0x3f, 0x3c};
  int i;
  //---digitalWrite(_CS, LOW);
  //resp[0] = //---SPI.transfer(commands[0]);
  //---digitalWrite(_CS, HIGH);

  usleep(10000);

  //---digitalWrite(_CS, LOW);
  for (i = 1; i < (int)sizeof(commands); i++){
    //resp[i] = //---SPI.transfer(commands[i]);
    ////delayMicroseconds(4);
  }

  //---digitalWrite(_CS, HIGH);      // Pull the pin high

  return _compare_arrays(resp, expected, 6);
}

bool sensor_enter_bootloader()
{
	DBG("call");
  // Enter into bootloader mode
  uint8_t resp[1];
  char expected[] = {0xF3};

  //---digitalWrite(_CS, LOW);
  //resp[0] = //---SPI.transfer(0x41);
  //---digitalWrite(_CS, HIGH);

  return _compare_arrays(resp, expected, 1);
}

bool sensor_set_fan_power(uint8_t value)
{
    _UNUSED(value);

	DBG("call");
  // Set the fan power
  // Value must be between 0-255
  // Ex.
  // $ alpha.set_fan_power(255);
  uint8_t resp[3];
  char expected[] = {0xF3, 0x42, 0x00};

  //---digitalWrite(_CS, LOW);
  //resp[0] = //---SPI.transfer(0x42);
  //---digitalWrite(_CS, HIGH);

  usleep(10000);

  //---digitalWrite(_CS, LOW);
  //resp[1] = //---SPI.transfer(0x00);

  //delayMicroseconds(4);

  //resp[2] = //---SPI.transfer(value);
  //---digitalWrite(_CS, HIGH);

  return _compare_arrays(resp, expected, 3);
}

bool sensor_set_laser_power(uint8_t value)
{
    _UNUSED(value);

	DBG("call");
  // Set the laser power
  // Value must be between 0-255
  // Ex.
  // $ alpha.set_laser_power(255);
  uint8_t resp[3];
  char expected[] = {0xF3, 0x42, 0x01};

  //---digitalWrite(_CS, LOW);
  //resp[0] = //---SPI.transfer(0x42);
  //---digitalWrite(_CS, HIGH);

  usleep(10000);

  //---digitalWrite(_CS, LOW);
  //resp[1] = //---SPI.transfer(0x01);

  //delayMicroseconds(4);

  //resp[2] = //---SPI.transfer(value);
  //---digitalWrite(_CS, HIGH);

  return _compare_arrays(resp, expected, 3);
}

bool sensor_toggle_fan(bool state)
{
	DBG("call");
	// Toggle the state of the fan
	// Ex.
	// $ alpha.toggle_fan(true); // turns fan on
	uint8_t resp[2];
	char expected[] = {0xF3, 0x03};
	uint8_t cmd = 0x03;

	//---digitalWrite(_CS, LOW);
	//resp[0] = //---SPI.transfer(0x03);
	//---digitalWrite(_CS, HIGH);
	if (ldx_spi_transfer(spi_dev, &cmd, &resp[0], 1) == EXIT_FAILURE) {
		fprintf(stderr,"Toggle fan 1st cmd error \n");
	}

	usleep(10000);

	// turn either on or off
	//---digitalWrite(_CS, LOW);
	if (state == true){
	//resp[1] = //---SPI.transfer(0x04);
		cmd = 0x04;
		if (ldx_spi_transfer(spi_dev, &cmd, &resp[1], 1) == EXIT_FAILURE) {
			fprintf(stderr,"Toggle on fan error \n");
		}
	}
	else {
	//resp[1] = //---SPI.transfer(0x05);
		cmd = 0x05;
		if (ldx_spi_transfer(spi_dev, &cmd, &resp[1], 1) == EXIT_FAILURE) {
			fprintf(stderr,"Toggle off fan error \n");
		}
	}

	//---digitalWrite(_CS, HIGH);

	return _compare_arrays(resp, expected, 2);
}

bool sensor_toggle_laser(bool state)
{
	DBG("call");
  // Toggle the state of the laser
  // Ex.
  // $ alpha.toggle_laser(true);
  uint8_t resp[2];
  char expected[] = {0xF3, 0x03};

  //---digitalWrite(_CS, LOW);
  //resp[0] = //---SPI.transfer(0x03);
  //---digitalWrite(_CS, HIGH);

  usleep(10000);

  //---digitalWrite(_CS, LOW);
  if (state == true){
    //resp[1] = //---SPI.transfer(0x02);
  }
  else {
    //resp[1] = //---SPI.transfer(0x03);
  }

  //---digitalWrite(_CS, HIGH);

  return _compare_arrays(resp, expected, 2);
}

ConfigVars sensor_read_configuration_variables()
{
	DBG("call");
  // Read the configuration variables and return the structure
  // Ex.
  // $ alpha.read_configuration_variables();
  ConfigVars results;       // empty structure for the data
  char vals[256];
  int i;

  // Read the config variables
  //---digitalWrite(_CS, LOW);
  //---SPI.transfer(0x3c);
  //---digitalWrite(_CS, HIGH);

  usleep(10000);

  //---digitalWrite(_CS, LOW);
  for (i = 0; i < 256; i++){
   // vals[i] = //---SPI.transfer(0x00);
    ////delayMicroseconds(4);
  }

  //---digitalWrite(_CS, HIGH);

  // Fill in the results
  results.bb0 = _16bit_int(vals[0], vals[1]);
  results.bb1 = _16bit_int(vals[2], vals[3]);
  results.bb2 = _16bit_int(vals[4], vals[5]);
  results.bb3 = _16bit_int(vals[6], vals[7]);
  results.bb4 = _16bit_int(vals[8], vals[9]);
  results.bb5 = _16bit_int(vals[10], vals[11]);
  results.bb6 = _16bit_int(vals[12], vals[13]);
  results.bb7 = _16bit_int(vals[14], vals[15]);
  results.bb8 = _16bit_int(vals[16], vals[17]);
  results.bb9 = _16bit_int(vals[18], vals[19]);
  results.bb10 = _16bit_int(vals[20], vals[21]);
  results.bb11 = _16bit_int(vals[22], vals[23]);
  results.bb12 = _16bit_int(vals[24], vals[25]);
  results.bb13 = _16bit_int(vals[26], vals[27]);
  results.bb14 = _16bit_int(vals[28], vals[29]);

  // Bin Particle Volumes
  results.bpv0 = _calculate_float(vals[32], vals[33], vals[34], vals[35]);
  results.bpv1 = _calculate_float(vals[36], vals[37], vals[38], vals[39]);
  results.bpv2 = _calculate_float(vals[40], vals[41], vals[42], vals[43]);
  results.bpv3 = _calculate_float(vals[44], vals[45], vals[46], vals[47]);
  results.bpv4 = _calculate_float(vals[48], vals[49], vals[50], vals[51]);
  results.bpv5 = _calculate_float(vals[52], vals[53], vals[54], vals[55]);
  results.bpv6 = _calculate_float(vals[56], vals[57], vals[58], vals[59]);
  results.bpv7 = _calculate_float(vals[60], vals[61], vals[62], vals[63]);
  results.bpv8 = _calculate_float(vals[64], vals[65], vals[66], vals[67]);
  results.bpv9 = _calculate_float(vals[68], vals[69], vals[70], vals[71]);
  results.bpv10 = _calculate_float(vals[72], vals[73], vals[74], vals[75]);
  results.bpv11 = _calculate_float(vals[76], vals[77], vals[78], vals[79]);
  results.bpv12 = _calculate_float(vals[80], vals[81], vals[82], vals[83]);
  results.bpv13 = _calculate_float(vals[84], vals[85], vals[86], vals[87]);
  results.bpv14 = _calculate_float(vals[88], vals[89], vals[90], vals[91]);
  results.bpv15 = _calculate_float(vals[92], vals[93], vals[94], vals[95]);

  // Bin Particle Densities
  results.bpd0 = _calculate_float(vals[96], vals[97], vals[98], vals[99]);
  results.bpd1 = _calculate_float(vals[100], vals[101], vals[102], vals[103]);
  results.bpd2 = _calculate_float(vals[104], vals[105], vals[106], vals[107]);
  results.bpd3 = _calculate_float(vals[108], vals[109], vals[110], vals[111]);
  results.bpd4 = _calculate_float(vals[112], vals[113], vals[114], vals[115]);
  results.bpd5 = _calculate_float(vals[116], vals[117], vals[118], vals[119]);
  results.bpd6 = _calculate_float(vals[120], vals[121], vals[122], vals[123]);
  results.bpd7 = _calculate_float(vals[124], vals[125], vals[126], vals[127]);
  results.bpd8 = _calculate_float(vals[128], vals[129], vals[130], vals[131]);
  results.bpd9 = _calculate_float(vals[132], vals[133], vals[134], vals[135]);
  results.bpd10 = _calculate_float(vals[136], vals[137], vals[138], vals[139]);
  results.bpd11 = _calculate_float(vals[140], vals[141], vals[142], vals[143]);
  results.bpd12 = _calculate_float(vals[144], vals[145], vals[146], vals[147]);
  results.bpd13 = _calculate_float(vals[148], vals[149], vals[150], vals[151]);
  results.bpd14 = _calculate_float(vals[152], vals[153], vals[154], vals[155]);
  results.bpd15 = _calculate_float(vals[156], vals[157], vals[158], vals[159]);

  // Bin Particle Sample Volumes
  results.bsvw0 = _calculate_float(vals[160], vals[161], vals[162], vals[163]);
  results.bsvw1 = _calculate_float(vals[164], vals[165], vals[166], vals[167]);
  results.bsvw2 = _calculate_float(vals[168], vals[169], vals[170], vals[171]);
  results.bsvw3 = _calculate_float(vals[172], vals[173], vals[174], vals[175]);
  results.bsvw4 = _calculate_float(vals[176], vals[177], vals[178], vals[179]);
  results.bsvw5 = _calculate_float(vals[180], vals[181], vals[182], vals[183]);
  results.bsvw6 = _calculate_float(vals[184], vals[185], vals[186], vals[187]);
  results.bsvw7 = _calculate_float(vals[188], vals[189], vals[190], vals[191]);
  results.bsvw8 = _calculate_float(vals[192], vals[193], vals[194], vals[195]);
  results.bsvw9 = _calculate_float(vals[196], vals[197], vals[198], vals[199]);
  results.bsvw10 = _calculate_float(vals[200], vals[201], vals[202], vals[203]);
  results.bsvw11 = _calculate_float(vals[204], vals[205], vals[206], vals[207]);
  results.bsvw12 = _calculate_float(vals[208], vals[209], vals[210], vals[211]);
  results.bsvw13 = _calculate_float(vals[212], vals[213], vals[214], vals[215]);
  results.bsvw14 = _calculate_float(vals[216], vals[217], vals[218], vals[219]);
  results.bsvw15 = _calculate_float(vals[220], vals[221], vals[222], vals[223]);

  // Gain Scaling Coefficient
  results.gsc = _calculate_float(vals[224], vals[225], vals[226], vals[227]);

  // Sample Flow Rate (ml/s)
  results.sfr = _calculate_float(vals[228], vals[229], vals[230], vals[231]);

  // LaserDAC
  results.laser_dac = (unsigned int)vals[232];
  results.fan_dac = (unsigned int)vals[233];

  // Time-of-Flight to Sample Flow Rate ratio
  results.tof_sfr = (unsigned int)vals[234];

  return results;
}

ConfigVars2 sensor_read_configuration_variables2()
{
	// Read the additional configuration variables
	// Only available on OPCN2's with firmware >v18
	// Ex.
	// $ alpha.read_configuration_variables2();
	ConfigVars2 results;       // empty structure for the data
	char vals[9];
	uint8_t cmd = 0x3D;

	// Read the config variables
	//---digitalWrite(_CS, LOW);
	//---SPI.transfer(0x3D);
	//---digitalWrite(_CS, HIGH);

	if (ldx_spi_write(spi_dev, &cmd, 1) == EXIT_FAILURE){
		fprintf(stderr,"Read_configuration: request command error\n");
	}

	usleep(10000);

	cmd = 0x00;
	for (int i = 0; i < 9; i++){
		if (ldx_spi_transfer(spi_dev, &cmd, &vals[i], 1) == EXIT_FAILURE) {
			fprintf(stderr,"Read_configuration: read data error\n");
		}
		usleep(4);
	}

	//---digitalWrite(_CS, HIGH);

	// Fill in the results
	results.AMSamplingInterval    = _16bit_int(vals[0], vals[1]);
	results.AMIntervalCount       = _16bit_int(vals[2], vals[3]);
	results.AMFanOnIdle           = (unsigned int)vals[4];
	results.AMLaserOnIdle         = (unsigned int)vals[5];
	results.AMMaxDataArraysInFile = _16bit_int(vals[6], vals[7]);
	results.AMOnlySavePMData      = (unsigned int)vals[8];

	return results;
}

int print_information_string()
{
	uint8_t vals[60];
	uint8_t cmd = 0x3F;
	if (ldx_spi_write(spi_dev, &cmd, 1) == EXIT_FAILURE){
		fprintf(stderr,"read_information_string 1 cmd error\n");
	}

	usleep(3000);

	cmd = 0x00;
	for (int i = 0; i < 60; i++) {
		if (ldx_spi_transfer(spi_dev, &cmd, &vals[i], 1) == EXIT_FAILURE) {
			fprintf(stderr,"read_information_string 1 cmd error\n");
		}
		usleep(4);
	}

	printf("OPC-N2 information: %s\n", vals);

	return 1;
}

PMData sensor_read_pm_data()
{
	DBG("call");
	// Read the PM Data and reset the histogram, return the struct
	// Only available on OPCN2's with firmware >v18
	// Ex.
	// $ alpha.read_pm_data();
	PMData data;
	uint8_t vals[12];
	int i;

      // Read the data and clear the local memory
      //---digitalWrite(_CS, LOW);       // Pull the CS Low
      //---SPI.transfer(0x32);                 // Transfer the command byte
      //---digitalWrite(_CS, HIGH);

    uint8_t cmd=0x32;
	if (ldx_spi_transfer(spi_dev, &cmd, &vals[0], 1) == EXIT_FAILURE) {
		fprintf(stderr,"Read pm data 1st cmd error \n");
	}


	usleep(12000);
	//delay(12);                          // Delay for 12 ms

	// Send commands and build array of data
	//---digitalWrite(_CS, LOW);

	for (i = 0; i < 12; i++){
	  //vals[i] = //---SPI.transfer(0x00);
	  ////delayMicroseconds(4);
		uint8_t cmd=0x00;
		if (ldx_spi_transfer(spi_dev, &cmd, &vals[i], 1) == EXIT_FAILURE){
			fprintf(stderr,"Read pm data 2nd cmd error\n");
		}

	}
//      if(verbose) {
//          char buffer[256] = { 0 };
//          log_hex_dump(vals, buffer, 12, SPI_LINE_SIZE, "RX PM");
//          log_print(LOG_MSG_DEBUG, buffer);
//      }
      //---digitalWrite(_CS, HIGH);      // Pull the CS High

	data.pm1  = _calculate_float(vals[0], vals[1], vals[2], vals[3]);
	data.pm25 = _calculate_float(vals[4], vals[5], vals[6], vals[7]);
	data.pm10 = _calculate_float(vals[8], vals[9], vals[10], vals[11]);


	return data;
}

HistogramData sensor_read_histogram(bool convert_to_conc)
{
	// Read the Histogram Data and reset the histogram, return the struct
	// convert_to_conc can be set to true if you would like the result
	// returned as concentrations (rather than raw counts) with units of
	// particles per cubic centimeter [#/cc]
	// Ex.
	// $ alpha.read_histogram(true)
	HistogramData data;
	uint8_t vals[62], cmd = 0x30;

	// Read the data and clear the local memory
	if (ldx_spi_transfer(spi_dev, &cmd, &vals[0], 1) == EXIT_FAILURE) {
		fprintf(stderr,"Read histogram: command request error \n");
	}

	usleep(12000);

	cmd = 0x00;
	for (int i = 0; i < 62; i++) {
		if (ldx_spi_transfer(spi_dev, &cmd, &vals[i], 1) == EXIT_FAILURE) {
			fprintf(stderr,"Read histogram: read data error\n");
		}
		usleep(4);
	}

	data.period = _calculate_float(vals[44], vals[45], vals[46], vals[47]);
	data.sfr    = _calculate_float(vals[36], vals[37], vals[38], vals[39]);

	// If convert_to_conc = True, convert from raw data to concentration
	double conv;

	if ( convert_to_conc != true ) {
	  conv = 1.0;
	}
	else {
	  conv = data.sfr * data.period;
	}

	// Calculate all of the values!
	data.bin0   = (double)_16bit_int(vals[0], vals[1]) / conv;
	data.bin1   = (double)_16bit_int(vals[2], vals[3]) / conv;
	data.bin2   = (double)_16bit_int(vals[4], vals[5]) / conv;
	data.bin3   = (double)_16bit_int(vals[6], vals[7]) / conv;
	data.bin4   = (double)_16bit_int(vals[8], vals[9]) / conv;
	data.bin5   = (double)_16bit_int(vals[10], vals[11]) / conv;
	data.bin6   = (double)_16bit_int(vals[12], vals[13]) / conv;
	data.bin7   = (double)_16bit_int(vals[14], vals[15]) / conv;
	data.bin8   = (double)_16bit_int(vals[16], vals[17]) / conv;
	data.bin9   = (double)_16bit_int(vals[18], vals[19]) / conv;
	data.bin10  = (double)_16bit_int(vals[20], vals[21]) / conv;
	data.bin11  = (double)_16bit_int(vals[22], vals[23]) / conv;
	data.bin12  = (double)_16bit_int(vals[24], vals[25]) / conv;
	data.bin13  = (double)_16bit_int(vals[26], vals[27]) / conv;
	data.bin14  = (double)_16bit_int(vals[28], vals[29]) / conv;
	data.bin15  = (double)_16bit_int(vals[30], vals[31]) / conv;

	data.bin1MToF = vals[32] / 3.0;
	data.bin3MToF = vals[33] / 3.0;
	data.bin5MToF = vals[34] / 3.0;
	data.bin7MToF = vals[35] / 3.0;

	// This holds either temperature or pressure
	// If temp, this is temp in C x 10
	// If pressure, this is pressure in Pa
	data.temp_pressure = _32bit_int(vals[40], vals[41], vals[42], vals[43]);

	data.checksum = _16bit_int(vals[48], vals[49]);

	data.pm1 = _calculate_float(vals[50], vals[51], vals[52], vals[53]);
	data.pm25 = _calculate_float(vals[54], vals[55], vals[56], vals[57]);
	data.pm10 = _calculate_float(vals[58], vals[59], vals[60], vals[61]);

	return data;
}


