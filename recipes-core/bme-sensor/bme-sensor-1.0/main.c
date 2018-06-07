#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "rnox/queue.h"
#include "rnox/log.h"
#include <libdigiapix/i2c.h>

#include "bme280.h"
#include "bme280_defs.h"

#define I2C_TIMEOUT					10
#define I2C_BUS_NUMBER				1
#define I2C_SENSOR_ADDRESS			0xEE // SDO to GND; 7bit - address, 1bit - R/W mode

#define DATA_LENGTH			150

static i2c_t *i2c_bus = NULL;
static unsigned int i2c_address;
static char *datapost = NULL;
static int queue_handle = -1;          // queue handle

struct bme280_dev dev;

/*
 * cleanup() - Frees all the allocated memory before exiting
 */
static void cleanup(void)
{
	bme280_set_sensor_mode(0, &dev);

	/* Free i2c */
	ldx_i2c_free(i2c_bus);

	if (queue_destroy(queue_handle) == EXIT_FAILURE) {
		log_print(LOG_MSG_INFO, "Failed to close queue");
	}

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

void user_delay_ms(uint32_t period)
{
    /*
     * Return control or wait,
     * for a period amount of milliseconds
     */
	usleep(period * 1000);
}

int8_t user_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
    int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */

    /*
     * The parameter dev_id can be used as a variable to store the I2C address of the device
     */

    /*
     * Data on the bus should be like
     * |------------+---------------------|
     * | I2C action | Data                |
     * |------------+---------------------|
     * | Start      | -                   |
     * | Write      | (reg_addr)          |
     * | Stop       | -                   |
     * | Start      | -                   |
     * | Read       | (reg_data[0])       |
     * | Read       | (....)              |
     * | Read       | (reg_data[len - 1]) |
     * | Stop       | -                   |
     * |------------+---------------------|
     */

	// printf("dev_id: 0x%X \n", dev_id);
	// printf("reg_addr: 0x%X \n", reg_addr);
	// printf("len: %d \n", len);

	rslt = ldx_i2c_transfer(i2c_bus, dev_id, &reg_addr, 1, reg_data, len);

	// printf("rslt: %d \n", rslt);

    return rslt;
}

int8_t user_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
    int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */

	uint8_t temp_buff[33];

    /*
     * The parameter dev_id can be used as a variable to store the I2C address of the device
     */

    /*
     * Data on the bus should be like
     * |------------+---------------------|
     * | I2C action | Data                |
     * |------------+---------------------|
     * | Start      | -                   |
     * | Write      | (reg_addr)          |
     * | Write      | (reg_data[0])       |
     * | Write      | (....)              |
     * | Write      | (reg_data[len - 1]) |
     * | Stop       | -                   |
     * |------------+---------------------|
     */

	temp_buff[0] = reg_addr;

	memcpy(&temp_buff[1], reg_data, len);

	rslt = ldx_i2c_write(i2c_bus, dev_id, temp_buff, len + 1);

    return rslt;
}

void print_sensor_data(struct bme280_data *comp_data)
{
#ifdef BME280_FLOAT_ENABLE
        printf("%0.2f, %0.2f, %0.2f\r\n",comp_data->temperature, comp_data->pressure, comp_data->humidity);
#else
        printf("%ld, %ld, %ld\r\n",comp_data->temperature, comp_data->pressure, comp_data->humidity);
#endif
}

int main(int argc, char *argv[])
{
	int8_t rslt = BME280_OK;
	uint8_t settings_sel;
	struct bme280_data comp_data;

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
	char *name = basename(argv[0]);
	uint8_t i2c_bus_number;

	/* Check input parameters */
	if (argc == 1) {
		/* Use default values */
		i2c_bus_number = I2C_BUS_NUMBER;
		i2c_address = I2C_SENSOR_ADDRESS;
	}
	// else if (argc == 6) {
	// 	/* Parse command line arguments */
	// 	i2c_bus_number = parse_argument(argv[1]);
	// 	i2c_address = (unsigned int)strtol(argv[2], NULL, 16);
	// } 
	else {
		printf("exit: argc check\n");
		exit(EXIT_FAILURE);
	}

	if (i2c_bus_number < 0) {
		printf("I2C bus index must be 0 or greater\n");
		return EXIT_FAILURE;
	}

	/* Register signals and exit cleanup function */
	atexit(cleanup);
	register_signals();

// printf("Start initialize I2C\n");

	/* Request I2C */
	i2c_bus = ldx_i2c_request((unsigned int)i2c_bus_number);
	if (!i2c_bus) {
		log_print(LOG_MSG_INFO, "Failed to initialize I2C");
		printf("Failed to initialize I2C\n");
		return EXIT_FAILURE;
	}

// printf("Start to set I2C timeout\n");

	/* Set the timeout for the I2C slave. */
	if (ldx_i2c_set_timeout(i2c_bus, I2C_TIMEOUT) != EXIT_SUCCESS) {
		log_print(LOG_MSG_INFO, "Failed to set I2C timeout");
		printf("Failed to set I2C timeout\n");
		return EXIT_FAILURE;
	}

	/* Init BME sensor */
	dev.dev_id = BME280_I2C_ADDR_SEC;
	dev.intf = BME280_I2C_INTF;
	dev.read = user_i2c_read;
	dev.write = user_i2c_write;
	dev.delay_ms = user_delay_ms;

// printf("Start initialize BME\n");

	if (bme280_init(&dev) == EXIT_SUCCESS) {
		log_print(LOG_MSG_INFO, "BME sensor was initialized\n");
		printf("Init BME280 ok\n");
	} else {
		log_print(LOG_MSG_INFO, "Failed to initialize BME sensor. Exited.\n");
		printf("Failed to init BME280\n");
		return EXIT_FAILURE;
	}

    if (queue_create(&queue_handle) != EXIT_SUCCESS) {
    	log_print(LOG_MSG_INFO, "Failed to create messages' queue");
        return EXIT_FAILURE;
    }

    datapost = (char*) malloc(DATA_LENGTH);

	log_print(LOG_MSG_INFO, "daemon-bme-sensor started successfully");

	/* Recommended mode of operation: Indoor navigation */
	dev.settings.osr_h = BME280_OVERSAMPLING_1X;
	dev.settings.osr_p = BME280_OVERSAMPLING_16X;
	dev.settings.osr_t = BME280_OVERSAMPLING_2X;
	dev.settings.filter = BME280_FILTER_COEFF_16;
	dev.settings.standby_time = BME280_STANDBY_TIME_62_5_MS;

	settings_sel = BME280_OSR_PRESS_SEL;
	settings_sel |= BME280_OSR_TEMP_SEL;
	settings_sel |= BME280_OSR_HUM_SEL;
	settings_sel |= BME280_STANDBY_SEL;
	settings_sel |= BME280_FILTER_SEL;

	rslt = bme280_set_sensor_settings(settings_sel, &dev);

	rslt = bme280_set_sensor_mode(BME280_NORMAL_MODE, &dev);

	// printf("Temperature, Pressure, Humidity\r\n");

	while (1) {
		/* Delay while the sensor completes a measurement */
		dev.delay_ms(1000);

		rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);

		printf("bme280_get_sensor_data: %d\n", rslt);

		/* old */
		if (snprintf(datapost, DATA_LENGTH, "{\"device_id\":\"%s\",\"sensor_id\":\"%s\",\"sensor_type\":\"%s\",\"TEMP_gas\":%.2f,\"RH_gas\":%.2f,\"PRESS_gas\":%.d}",
								"AirNode_0001", "BME280", "BME", comp_data.temperature, comp_data.humidity, (uint32_t)comp_data.pressure) < 1) {
			log_print(LOG_MSG_INFO, "Failed to create formated data");
		}

		if (queue_put_msg(datapost, NULL) == EXIT_FAILURE) {
			log_print(LOG_MSG_INFO, "Failed to put message in queue");
		}

		// /* TEMP_gas */
		// if (snprintf(datapost, DATA_LENGTH, "{\"device_id\":\"%s\",\"sensor_id\":\"%s\",\"sensor_type\":\"%s\",\"TEMP_gas\":%.2f}",
		// 						"AirNode_0001", "BME280", "TEMP_gas", comp_data.temperature) < 1) {
		// 	log_print(LOG_MSG_INFO, "Failed to create formated data [TEMP_gas]");
		// }

		// if (queue_put_msg(datapost, NULL) == EXIT_FAILURE) {
		// 	log_print(LOG_MSG_INFO, "Failed to put message in queue [TEMP_gas]");
		// }

		// /* RH_gas */
		// if (snprintf(datapost, DATA_LENGTH, "{\"device_id\":\"%s\",\"sensor_id\":\"%s\",\"sensor_type\":\"%s\",\"RH_gas\":%.2f}",
		// 						"AirNode_0001", "BME280", "RH_gas", comp_data.humidity) < 1) {
		// 	log_print(LOG_MSG_INFO, "Failed to create formated data [RH_gas]");
		// }

		// if (queue_put_msg(datapost, NULL) == EXIT_FAILURE) {
		// 	log_print(LOG_MSG_INFO, "Failed to put message in queue [RH_gas]");
		// }

		// /* PRESS_gas */
		// if (snprintf(datapost, DATA_LENGTH, "{\"device_id\":\"%s\",\"sensor_id\":\"%s\",\"sensor_type\":\"%s\",\"PRESS_gas\":%.d}",
		// 						"AirNode_0001", "BME280", "PRESS_gas", (uint32_t)comp_data.pressure) < 1) {
		// 	log_print(LOG_MSG_INFO, "Failed to create formated data [PRESS_gas]");
		// }

		// if (queue_put_msg(datapost, NULL) == EXIT_FAILURE) {
		// 	log_print(LOG_MSG_INFO, "Failed to put message in queue [PRESS_gas]");
		// }

		// print_sensor_data(&comp_data);
	}

	return EXIT_SUCCESS;
}
