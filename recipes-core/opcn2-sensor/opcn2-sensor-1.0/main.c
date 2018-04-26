#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "../../librnox/queue.h"
#include "../../librnox/log.h"
#include <libdigiapix/spi.h>
#include "opcn2.h"

#define ARG_SPI_DEVICE		0
#define ARG_SPI_SLAVE		1
#define DATA_LENGTH			200

static char *datapost;

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
 * cleanup() - Frees all the allocated memory before exiting
 */
static void cleanup(void)
{
	/* Release sensor */
	if (sensor_off()) {
		log_print(LOG_MSG_INFO, "OPC-N2 was turned off successfully\n");
	} else {
		log_print(LOG_MSG_INFO, "Failed to turn off OPC-N2");
	}

	free(datapost);

	log_print(LOG_MSG_INFO, "deamon-opcn2-sensor exited successfully");
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
	/* Our process ID and Session ID */
	pid_t pid, sid;

	/* Fork off the parent process */
	pid = fork();
	if (pid < 0) {
			exit(EXIT_FAILURE);
	}
	/* If we got a good PID, then
	   we can exit the parent process. */
	if (pid > 0) {
			exit(EXIT_SUCCESS);
	}

	/* Change the file mode mask */
	umask(0);

	/* Open logs here */
	log_open(LOG_ODELAY | LOG_PID, LOG_DAEMON, false);

	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0) {
		/* Log the failure */
		log_print(LOG_MSG_INFO, "Failed to create SID");
		exit(EXIT_FAILURE);
	}

	/* Change the current working directory */
	if ((chdir("/")) < 0) {
			/* Log the failure */
		log_print(LOG_MSG_INFO, "Failed to change working directory");
		exit(EXIT_FAILURE);
	}

	/* Close out the standard file descriptors */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* Daemon init */
	int spi_device = 0, spi_slave = 0;

	char *name = basename(argv[0]);

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

	/* Register signals and exit cleanup function */
	atexit(cleanup);
	register_signals();

	if (sensor_init(spi_device, spi_slave) == EXIT_SUCCESS) {
		log_print(LOG_MSG_INFO, "OPC-N2 sensor was initialized\n");
	} else {
		log_print(LOG_MSG_INFO, "Failed to initialize OPC-N2 sensor. Exited.\n");
		return EXIT_FAILURE;
	}

//	sleep(1);
//	log_print(LOG_MSG_INFO, print_information_string());

//	sleep(1);
//	ConfigVars2 conf = sensor_read_configuration_variables2();
//	printf("OPC-N2 configuration:\n"
//		   "- SamplingInterval - %d;\n"
//		   "- IntervalCount - %d;\n"
//		   "- FanOnIdle - %d;\n"
//		   "- LaserOnIdle - %d;\n"
//		   "- MaxDataArraysInFile - %d;\n"
//		   "- OnlySavePMData - %d.\n", conf.AMSamplingInterval, conf.AMIntervalCount, conf.AMFanOnIdle,
//		   conf.AMLaserOnIdle, conf.AMMaxDataArraysInFile, conf.AMOnlySavePMData);

//	sleep(1);
//	Status *sensor_status = (Status *)malloc(sizeof(Status));
//	if (sensor_read_status(sensor_status) == EXIT_SUCCESS) {
//		printf("Reading OPC-N2 sensor status:\n"
//			   "- fanON - %d;\n"
//			   "- laserON - %d;\n"
//			   "- fanDAC - %d;\n"
//			   "- laserDAC - %d.\n", sensor_status->fanON, sensor_status->laserON,
//			   sensor_status->fanDAC, sensor_status->laserDAC);
//	}

    int queue_handle = -1;          // queue handle
    if (queue_create(&queue_handle) != EXIT_SUCCESS) {
    	log_print(LOG_MSG_INFO, "Failed to create messages' queue");
        return EXIT_FAILURE;
    }

    datapost = (char*) malloc(DATA_LENGTH);

	log_print(LOG_MSG_INFO, "daemon-opcn2-sensor started successfully");

	while(1) {

//		if (sensor_on()) {
//			log_print(LOG_MSG_INFO, "OPC-N2 was turned on\n");
//		} else { log_print(LOG_MSG_INFO, "Failed to turn on OPC-N2\n"); }

		sleep(1);

		if (sensor_ping()) {
			log_print(LOG_MSG_INFO, "OPC-N2 was pinged successfully\n");
		} else {
			log_print(LOG_MSG_INFO, "Failed to ping OPC-N2 sensor\n");
		}

		sleep(3);

		HistogramData hist = sensor_read_histogram(true);

		//"timestamp": "2013-08-31T01:02:33.555"
//		char timestamp [256];
//		{
//			time_t rawtime;
//			struct tm * timeinfo;
//			time ( &rawtime );
//			timeinfo = localtime ( &rawtime );
//			strftime (timestamp,30,"%Y-%m-%dT%H:%M:%S.000",timeinfo);
//		}

		// build post data
//		asprintf(&datapost, "{\"device_id\":\"%s\",\"sensor_id\":\"%s\",\"sensor_type\":\"%s\",\"PM1\":%f,\"PM2\":%f,\"PM3\":%f,\"SFR\":%f,\"PRESS\":%d}",
//				 "api-test-device-02", "SPI", "OPC-N2", hist.pm1, hist.pm25, hist.pm10, hist.sfr, hist.temp_pressure);
		if (snprintf(datapost, DATA_LENGTH, "{\"device_id\":\"%s\",\"sensor_id\":\"%s\",\"sensor_type\":\"%s\",\"PM1\":%f,\"PM2\":%f,\"PM3\":%f,\"SFR\":%f,\"PRESS\":%d}",
								"api-test-device-02", "SPI", "OPC-N2", hist.pm1, hist.pm25, hist.pm10, hist.sfr, hist.temp_pressure) < 1) {
			log_print(LOG_MSG_INFO, "Failed to create formated data");
		}

		if (queue_put_msg(datapost, NULL) == EXIT_FAILURE) {
			log_print(LOG_MSG_INFO, "Failed to put message in queue");
		}

//		printf("OPC-N2 data: PM1 - %E, PM2.5 - %E, PM10 - %E, TEMP_PRESSURE: %lu, period - %f, sfr - %f.\n",
//				hist.pm1, hist.pm25, hist.pm10, hist.temp_pressure, hist.period, hist.sfr);
	}

	return EXIT_SUCCESS;
}
