#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "rnox/queue.h"
#include "rnox/log.h"

#include <libdigiapix/i2c.h>

#define I2C_TIMEOUT 1

#define DEFAULT_I2C_ALIAS		"DEFAULT_I2C_BUS"
#define I2C_BUS_NUMBER			1
#define I2C_SENSOR_ADDRESS		0x28//(0x28 << 1) | 1
#define DEFAULT_I2C_ADDRESS_SIZE	2

static i2c_t *i2c_bus;
static unsigned int i2c_address;
static uint8_t *tx_buf;
static uint8_t *rx_buf;

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
		"Example application using libdigiapix I2C support\n"
		"\n"
		"Usage: %s <i2c-bus> <i2c-address> <address-size> <page-size> <page-index>\n\n"
		"<i2c-bus>       I2C bus index to use or alias\n"
		"<i2c-address>   Address of the I2C EEPROM memory\n"
		"<address-size>  Number of EEPROM memory address bytes\n"
		"<page-size>     EEPROM memory page size in bytes\n"
		"<page-index>    EEPROM memory page index to use\n"
		"\n"
		"Aliases for I2C can be configured in the library config file\n"
		"\n", name);

	exit(exitval);
}

/*
 * cleanup() - Frees all the allocated memory before exiting
 */
static void cleanup(void)
{
	/* Free i2c */
	ldx_i2c_free(i2c_bus);

	/* Free buffers */
	free(rx_buf);
	free(tx_buf);

	log_print(LOG_MSG_INFO, "deamon-prs-sensor exited successfully");
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
 *
 * Return: The parsed integer argument, -1 on error.
 */
static int parse_argument(char *argv)
{
	char *endptr;
	long value;

	errno = 0;
	value = strtol(argv, &endptr, 10);

	if ((errno == ERANGE && (value == LONG_MAX || value == LONG_MIN))
			  || (errno != 0 && value == 0))
		return -1;

	if (endptr == argv)
		return ldx_i2c_get_bus(endptr);

	return value;
}

uint16_t _16bit_int(char MSB, char LSB)
{
  // Combine two bytes into a 16-bit unsigned int
  return ((MSB << 8) | LSB);
}

int main(int argc, char **argv)
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

	/* Open any logs here */
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

	/* Deamon initialization */
	char *name = basename(argv[0]);
	uint8_t i2c_bus_number;

	/* Check input parameters */
	if (argc == 1) {
		/* Use default values */
		i2c_bus_number = I2C_BUS_NUMBER;
		i2c_address = I2C_SENSOR_ADDRESS;
	} else if (argc == 6) {
		/* Parse command line arguments */
		i2c_bus_number = parse_argument(argv[1]);
		i2c_address = (unsigned int)strtol(argv[2], NULL, 16);
	} else {
		usage_and_exit(name, EXIT_FAILURE);
	}

	if (i2c_bus_number < 0) {
		printf("I2C bus index must be 0 or greater\n");
		return EXIT_FAILURE;
	}

	/* Register signals and exit cleanup function */
	atexit(cleanup);
	register_signals();

	/* Request I2C */
	i2c_bus = ldx_i2c_request((unsigned int)i2c_bus_number);
	if (!i2c_bus) {
		log_print(LOG_MSG_INFO, "Failed to initialize I2C");
		return EXIT_FAILURE;
	}

	/* Set the timeout for the I2C slave. */
	if (ldx_i2c_set_timeout(i2c_bus, I2C_TIMEOUT) != EXIT_SUCCESS) {
		log_print(LOG_MSG_INFO, "Failed to set I2C timeout");
//		printf("Failed to set I2C timeout\n");
		return EXIT_FAILURE;
	}

    int queue_handle = -1;          // queue handle
    if (queue_create(&queue_handle) != EXIT_SUCCESS) {
    	log_print(LOG_MSG_INFO, "Failed to create messages' queue");
        return EXIT_FAILURE;
    }

//	printf("Read pressure data...\n");

    while (1) {
    	log_print(LOG_MSG_INFO, "daemon-prs-sensor started successfully");

		uint8_t tx_buffer[] = {1};
		uint8_t rx_buffer[4] = {};

		ldx_i2c_transfer(i2c_bus, i2c_address, tx_buffer, 1, rx_buffer, 4);
		uint16_t pressure_counts, temp_counts;
		if (((rx_buffer[0] >> 7) | (rx_buffer[0] >> 6)) != 1) {
			pressure_counts =_16bit_int(rx_buffer[0], rx_buffer[1]);
			temp_counts = (uint16_t)(rx_buffer[2] << 3) | (rx_buffer[3] >> 5);
		}
		else {
			log_print(LOG_MSG_INFO, "Sensor indicated BAD status");
//			printf("Sensor indicated BAD status\n");
		}

		float pressure = ((pressure_counts - 1638.0) * (1 - 0) / (14745.0 - 1638)) + 0;
		float temperature = ((temp_counts / 2047.0) * 200) - 50;
		printf("Sensor status: %d-%d\n"
				"Pressure: %f\n"
				"Temperature: %f\n\n", (rx_buffer[0] >> 7), (rx_buffer[0] >> 6), pressure, temperature);


		//"timestamp": "2013-08-31T01:02:33.555"
		char timestamp [256];
		{
			time_t rawtime;
			struct tm * timeinfo;
			time ( &rawtime );
			timeinfo = localtime ( &rawtime );
			strftime (timestamp,30,"%Y-%m-%dT%H:%M:%S.000",timeinfo);
		}

		char *datapost = NULL;
		// build post data
		asprintf(&datapost, "{"
				 "\"timestamp\":\"%s\","
				 "\"device_id\":\"%s\","
				 "\"sensor_id\":\"%s\","
				 "\"sensor_type\":\"%s\","
				 "\"PRESS\":%f,"
				 "\"TEMP\":%f"
				 "}",
				 timestamp,
				 "api-test-device-01",
				 "I2C",
				 "PRS",
				 pressure,
				 temperature);

		queue_put_msg(datapost, NULL);

		sleep(3);
	}

	return EXIT_SUCCESS;
}
