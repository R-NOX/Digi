#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "libdigiapix/adc.h"
#include "rnox/queue.h"
#include "rnox/log.h"

#define ARG_ADC_CHIP			0
#define ARG_ADC_CHANNEL			1

#define DEFAULT_ADC_ALIAS		"DEFAULT_ADC"
#define DEFAULT_TIME_INTERVAl		1
#define DEFAULT_NUMBER_OF_SAMPLES	10

#define DATA_LENGTH 100

static adc_t *adc;
static char *datapost;
static int queue_handle = -1;          // queue handle

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
		"Example application using libdigiapix ADC support\n"
		"\n"
		"Usage: %s <adc_chip> <adc_channel> <interval> <number_of_samples> \n\n"
		"<adc_chip>           ADC chip number or alias\n"
		"<adc_channel>        ADC channel number or alias\n"
		"<interval>           Time interval for sampling\n"
		"<number_of_samples>  Number of samples to get\n"
		"\n"
		"Alias for ADC can be configured in the library config file\n"
		"\n", name);

	exit(exitval);
}

/*
 * cleanup() - Frees all the allocated memory before exiting
 */
static void cleanup(void)
{
	/* Free adc */
	ldx_adc_stop_sampling(adc);
	ldx_adc_free(adc);

	if (queue_destroy(queue_handle) == EXIT_FAILURE) {
		log_print(LOG_MSG_INFO, "Failed to close queue");
	}

	free(datapost);
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
 *			corresponding integer value
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
		case ARG_ADC_CHIP:
			return ldx_adc_get_chip(endptr);
		case ARG_ADC_CHANNEL:
			return ldx_adc_get_channel(endptr);
		default:
			return -1;
		}
	}

	return value;
}

/*
 * adc_sampling_cb() - ADC callback for sampling
 *
 * @arg:	ADC sampling data (struct adc_sampling_cb_data).
 * @sample:	The ADC read sample.
 */
static int adc_sampling_cb(int sample, void *arg)
{
	int consumption;

	if (sample < 0) {
		log_print(LOG_MSG_WARNING, "Wrong sample value");
		return EXIT_FAILURE;
	}

	consumption = (2 * pow(10, -7) * pow(sample, 3)) - (0.0007 * pow(sample, 2)) + (1.0915 * sample) - 453.09;

	/* old */
	snprintf(datapost, DATA_LENGTH, "{\"device_id\":\"%s\",\"sensor_id\":\"%s\",\"sensor_type\":\"%s\",\"SFR_gas\":%d}",
									"AirNode_0001", "D6F", "SFR", consumption);

	// snprintf(datapost, DATA_LENGTH, "{\"device_id\":\"%s\",\"sensor_id\":\"%s\",\"sensor_type\":\"%s\":%d}",
	// 								"AirNode_0001", "D6F", "SFR_gas", consumption);

//	log_print(LOG_MSG_INFO, "%d", sample);
//	log_print(LOG_MSG_INFO, "%s", datapost);
	if (queue_put_msg(datapost, NULL) == EXIT_FAILURE) {
		log_print(LOG_MSG_INFO, "Failed to put message in queue [SFR_gas]");
	}

	return EXIT_SUCCESS;
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
	/* Error in ADC initialization occurs if uncomment next line */
//	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	int channel = 0, chip = 0, interval = 0;
	char *name = basename(argv[0]);

	/* Check input parameters */
	if (argc == 1) {
		/* Use default values */
		chip = ldx_adc_get_chip(DEFAULT_ADC_ALIAS);
		channel = ldx_adc_get_channel(DEFAULT_ADC_ALIAS);
		interval = DEFAULT_TIME_INTERVAl;
	} else if (argc == 5) {
		/* Parse command line arguments */
		chip = parse_argument(argv[1], ARG_ADC_CHIP);
		channel = parse_argument(argv[2], ARG_ADC_CHANNEL);
		interval = atoi(argv[3]);
	} else {
		usage_and_exit(name, EXIT_FAILURE);
	}

	if (chip < 0) {
		printf("Invalid chip number\n");
		return EXIT_FAILURE;
	}

	if (channel < 0) {
		printf("Invalid channel number\n");
		return EXIT_FAILURE;
	}

	if (interval <= 0) {
		printf("Time interval must be greater than 0\n");
		return EXIT_FAILURE;
	}

	/* Register signals and exit cleanup function */
	atexit(cleanup);
	register_signals();

	adc = ldx_adc_request(chip, channel);

	if (!adc) {
		log_print(LOG_MSG_INFO, "Failed to initialize ADC\n");
		return EXIT_FAILURE;
	}

    if (queue_create(&queue_handle) != EXIT_SUCCESS) {
    	log_print(LOG_MSG_INFO, "Failed to create messages' queue");
        return EXIT_FAILURE;
    }

    datapost = (char*) malloc(DATA_LENGTH);

	log_print(LOG_MSG_INFO, "daemon-adc-sensor started successfully");

	if (ldx_adc_start_sampling(adc, &adc_sampling_cb, interval, NULL)) {
		log_print(LOG_MSG_INFO, "Failed to initialize the sampling data\n");
		return EXIT_FAILURE;
	}

	while (1) {
		sleep(1);
	}

	return EXIT_SUCCESS;
}
