#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <libdigiapix/spi.h>
#include "opcn2.h"

#define ARG_SPI_DEVICE		0
#define ARG_SPI_SLAVE		1

/*
 * usage_and_exit() - Show usage information and exit with 'exitval' return
 *					  value
 *
 * @name:	Application name.
 * @exitval:	The exit code.
 */
static void usage_and_exit(char *name, int exitval)
{
	fprintf(stdout,
		"Example application using libdigiapix SPI support\n"
		"\n"
		"Usage: %s <spi-dev> <spi-ss> <address-size> <page-size> <page-index>\n\n"
		"<spi-dev>       SPI device index to use or alias\n"
		"<spi-ss>        SPI slave index to use or alias\n"
		"<address-size>  Number of EEPROM memory address bytes\n"
		"<page-size>     EEPROM memory page size in bytes\n"
		"<page-index>    EEPROM memory page index to use\n"
		"\n"
		"Aliases for SPI can be configured in the library config file\n"
		"\n", name);

	exit(exitval);
}

/*
 * sigaction_handler() - Handler to execute after receiving a signal
 *
 * @signum:	Received signal.
 */
static void sigaction_handler(int signum)
{
	/* 'atexit' executes the cleanup function */
	exit(EXIT_FAILURE);
}

/*
 * register_signals() - Registers program signals
 */
static void register_signals(void)
{
	struct sigaction action;

	action.sa_handler = sigaction_handler;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);

	sigaction(SIGHUP, &action, NULL);
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGTERM, &action, NULL);
}

/*
 * parse_argument() - Parses the given string argument and returns the
 *					  corresponding integer value
 *
 * @argv:	Argument to parse in string format.
 * @arg_type:	Type of the argument to parse.
 *
 * Return: The parsed integer argument, -1 on error.
 */
static int parse_argument(char *argv, int arg_type)
{
	char *endptr;
	long value;

	errno = 0;
	value = strtol(argv, &endptr, 10);

	if ((errno == ERANGE && (value == LONG_MAX || value == LONG_MIN))
			  || (errno != 0 && value == 0))
		return -1;

	if (endptr == argv) {
		switch (arg_type) {
		case ARG_SPI_DEVICE:
			return ldx_spi_get_device(endptr);
		case ARG_SPI_SLAVE:
			return ldx_spi_get_slave(endptr);
		default:
			return -1;
		}
	}

	return value;
}

int main(int argc, char *argv[])
{
	int spi_device = 0, spi_slave = 0;

	char *name = basename(argv[0]);

	uint8_t *devices = NULL, *slaves = NULL;
	int devices_number = 0, slaves_number = 0;


	/* Retrieve the list of available SPI interfaces */
	devices_number = ldx_spi_list_available_devices(&devices);

	if (devices_number > 0) {
		printf("The target has %d available SPI devices:\n", devices_number);
		for (int i = 0; i < devices_number; i++) {
			printf(" - SPI-%d\n", devices[i]);
			}
	} else {
		printf("The target does not have any SPI interface available\n");
	}

	/* Retrieve the list of available SPI slaves for SPI Device 0*/
	slaves_number = ldx_spi_list_available_slaves(devices[0], &slaves);

	if (slaves_number > 0) {
		printf("SPI interface %d has %d available slaves:\n", devices[0], slaves_number);
		for (int i = 0; i < slaves_number; i++) {
			printf(" - SPI-%d.%d\n", devices[0], slaves[i]);
		}
	} else {
		printf("SPI interface %d does not have available slaves\n", devices[0]);
	}

	free(devices);
	free(slaves);

	/* Check input parameters */
	if (argc == 1) {
		/* Use default values */
		spi_device = 2; //ldx_spi_get_device(DEFAULT_SPI_ALIAS);
		spi_slave = 0; //ldx_spi_get_slave(DEFAULT_SPI_ALIAS);
	} else if (argc == 2) {
		/* Parse command line arguments */
		spi_device = parse_argument(argv[1], ARG_SPI_DEVICE);
		spi_slave = parse_argument(argv[2], ARG_SPI_SLAVE);
	} else {
		usage_and_exit(name, EXIT_FAILURE);
	}

	if (spi_device < 0 || spi_slave < 0) {
		printf("Unable to parse SPI device/slave arguments\n");
		return EXIT_FAILURE;
	}

	/* Register signals */
	register_signals();

	if (sensor_init(spi_device, spi_slave) == EXIT_SUCCESS) {
		printf("OPC-N2 sensor was initialized\n");
	} else {
		printf("Failed to initialize OPC-N2 sensor\n");
	}

	if (sensor_on()) {
		printf("OPC-N2 was turned on\n");
	} else { printf("Failed to turn on OPC-N2\n"); }

	sleep(1);

	if (sensor_ping()) {
		printf("OPC-N2 was pinged successfully\n");
	} else {
		printf("Failed to ping OPC-N2 sensor\n");
	}

	sleep(1);
	print_information_string();

	sleep(1);
	ConfigVars2 conf = sensor_read_configuration_variables2();
	printf("OPC-N2 configuration:\n"
		   "- SamplingInterval - %d;\n"
		   "- IntervalCount - %d;\n"
		   "- FanOnIdle - %d;\n"
		   "- LaserOnIdle - %d;\n"
		   "- MaxDataArraysInFile - %d;\n"
		   "- OnlySavePMData - %d.\n", conf.AMSamplingInterval, conf.AMIntervalCount, conf.AMFanOnIdle,
		   conf.AMLaserOnIdle, conf.AMMaxDataArraysInFile, conf.AMOnlySavePMData);

	sleep(1);
	Status *sensor_status = (Status *)malloc(sizeof(Status));
	if (sensor_read_status(sensor_status) == EXIT_SUCCESS) {
		printf("Reading OPC-N2 sensor status:\n"
			   "- fanON - %d;\n"
			   "- laserON - %d;\n"
			   "- fanDAC - %d;\n"
			   "- laserDAC - %d.\n", sensor_status->fanON, sensor_status->laserON,
			   sensor_status->fanDAC, sensor_status->laserDAC);
	}

	sleep(2);
	for (int i = 0; i < 5; i++) {
		HistogramData hist = sensor_read_histogram(true);

		printf("OPC-N2 data: PM1 - %E, PM2.5 - %E, PM10 - %E, TEMP_PRESSURE: %lu, period - %f, sfr - %f.\n",
				hist.pm1, hist.pm25, hist.pm10, hist.temp_pressure, hist.period, hist.sfr);
		sleep(3);
	}

	if (sensor_off()) {
		printf("OPC-N2 was turned off successfully\n");
	} else {
		printf("Failed to turn off OPC-N2");
	}

	return EXIT_SUCCESS;
}
