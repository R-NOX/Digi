/*
 * main.c
 *
 *  Created on: Apr 23, 2018
 *      Author: Herman <Herman.Yulau@promwad.com>
 */

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
#include "post.h"
#include "db.h"

#define DB_FILENAME "/home/root/rnox.db"

static int queue_handle = -1;          // queue handle
static sqlite3 *database;

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
	if (queue_destroy(queue_handle) == EXIT_FAILURE) {
		log_print(LOG_MSG_INFO, "Failed to close queue");
	}

	post_sessionClose();

	sqlite3_close(database);

	log_print(LOG_MSG_INFO, "daemon-data-handler exited successfully");
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

//	if (endptr == argv)
//		return ldx_i2c_get_bus(endptr);

	return value;
}

static int send_and_delete_item_fromDB(void *NotUsed, int argc, char **argv, char **azColName) {

    char *key = NULL;
    char *json = NULL;

    for (int i = 0; i < argc; i++) {
        if (!strcmp(azColName[i], DB_KEY_FIELD)) key = argv[i];
        if (!strcmp(azColName[i], DB_JSON_FIELD)) json = argv[i];
    }

    if ((key != NULL) && (json != NULL)) {
        if (post(json) == EXIT_SUCCESS) {
        	if (db_delete_item(&database, key) != SQLITE_OK) {
        		log_print(LOG_MSG_INFO, "Failed to delete item from database with KEY: %s", key);
        	}
        }
    }

    return SQLITE_OK;
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
		log_print(LOG_MSG_ERR, "Failed to create SID");
		exit(EXIT_FAILURE);
	}

	/* Change the current working directory */
	if ((chdir("/")) < 0) {
			/* Log the failure */
		log_print(LOG_MSG_ERR, "Failed to change working directory");
		exit(EXIT_FAILURE);
	}

	/* Close out the standard file descriptors */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* Deamon init */
	char *name = basename(argv[0]);

	/* Check input parameters */
	if (argc == 1) {
		/* Use default values */
	} else if (argc == 6) {
		/* Parse command line arguments */
	} else {
		usage_and_exit(name, EXIT_FAILURE);
	}

	/* Register signals and exit cleanup function */
	atexit(cleanup);
	register_signals();

    if (queue_create(&queue_handle) != EXIT_SUCCESS) {
    	log_print(LOG_MSG_ERR, "Failed to create messages' queue. Exit.");
        return EXIT_FAILURE;
    }

    if (db_open(DB_FILENAME, &database) != SQLITE_OK) {
    	log_print(LOG_MSG_ERR, "Failed to open database.");
    }

    if (post_sessionOpen() == EXIT_FAILURE) {
    	log_print(LOG_MSG_ERR, "Failed to open post session. Exit.");
    	return EXIT_FAILURE;
    }

    log_print(LOG_MSG_INFO, "daemon-data-handler started successfully");

    bool isConnection = true;

    /* Send data from database if it exists */
	if(test_connection_to_server() == EXIT_SUCCESS) {
		db_get_items(&database, send_and_delete_item_fromDB);
		isConnection = true;
	}

	while(1) {

		/* Check connection and send data */
		if (isConnection == false) {
			if(test_connection_to_server() == EXIT_SUCCESS) {
				db_get_items(&database, send_and_delete_item_fromDB);
				isConnection = true;
			}
			else {
				log_print(LOG_MSG_ERR, "No connection to server");
			}
		}

		/* Send data or save to database if there is no connection */
	    queue_t msg = { -1, { 0 } };
	    if (queue_get_msg(queue_handle, &msg)) {
//	    	log_print(LOG_MSG_INFO, "Retrieved data from queue: %s", msg.mtext);
	    	if (isConnection) {
	    		if (post(msg.mtext) == EXIT_FAILURE) {
					log_print(LOG_MSG_ERR, "Failed to send data to server");
					isConnection = false;
				}
//	    		else { log_print(LOG_MSG_INFO, "Data was sent to server");}
	    	}

	    	if (isConnection == false) {
	    		if (!db_add_item(&database, msg.mtext, NULL)) {
//	    			log_print(LOG_MSG_INFO, "Saved to database");
	    		}
	    		else {log_print(LOG_MSG_ERR, "Failed to save to database");}
	    		continue;
	    	}
	    }

		sleep(1);
	}
}
