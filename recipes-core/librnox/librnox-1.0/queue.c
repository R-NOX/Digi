#include "queue.h"
#include "log.h"

#define __need_error_t
#include <errno.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>

#define QUEUE_FILENAME      "/queue_sensors"
#define QUEUE_INT_VALUE     28832

static key_t key = -1;

key_t queue_get_key(void)
{
    if (key == -1) {
        key = ftok(QUEUE_FILENAME, QUEUE_INT_VALUE);
    }

    return key;
}

int queue_create(int* handle)
{
    int msgflg = IPC_CREAT | 0666;

//    if (((*handle = msgget(queue_get_key(), 0666)) > 0)
//            && (queue_destroy(*handle)) != EXIT_SUCCESS)
//        return EXIT_FAILURE;

    *handle = msgget(queue_get_key(), msgflg);

    if (*handle < 0) {
        log_print(LOG_MSG_ERR, "msgget failed with %s", strerror(errno));
        return EXIT_FAILURE;
    }

    log_print(LOG_MSG_DEBUG, "channel created");
    return EXIT_SUCCESS;
}

int queue_open(int* handle)
{
    int msgflg = IPC_CREAT | 0666;

    *handle = msgget(queue_get_key(), msgflg);

    if (*handle < 0) {
        log_print(LOG_MSG_ERR, "msgget failed with %s", strerror(errno));
        return EXIT_FAILURE;
    }

    log_print(LOG_MSG_DEBUG, "channel is open");
    return EXIT_SUCCESS;
}

int queue_destroy(int handle)
{
    if (msgctl(handle, IPC_RMID, NULL) < 0) {
        log_print(LOG_MSG_ERR, "msgctl failed with %s", strerror(errno));
        return EXIT_FAILURE;
    }

    log_print(LOG_MSG_DEBUG, "channel destroyed");
    return EXIT_SUCCESS;
}

int queue_get_msg(int handle, queue_t *queue_msg)
{
    int size = -1;

//    if (((size = msgrcv(handle, queue_msg, QUEUE_MAX_BUFFER_SIZE, 0, 0)) < 0) && (errno != EINTR)) {
    if (((size = msgrcv(handle, queue_msg, QUEUE_MAX_BUFFER_SIZE, 1, 0)) < 0) && (errno != EINTR)) {
        log_print(LOG_MSG_ERR, "msgrcv failed with %s", strerror(errno));
        return -1;
    }

    if (size > 0) log_print(LOG_MSG_DEBUG, "received message: \'%s\'", queue_msg->mtext);

    return size;
}

int queue_put_msg(/*int handle, */const char *msg, const char * const portname)
{
    int handle;

    queue_t queue_msg;
    queue_msg.mtype = 1;
    strcpy(queue_msg.mtext, msg);

    handle = msgget(queue_get_key(), 0666);
    if (handle == -1) {
        return EXIT_FAILURE;
    }

    if (msgsnd(handle, &queue_msg, strlen(msg), IPC_NOWAIT) < 0) {
        log_print(LOG_MSG_WARNING, "failed with %s", strerror(errno));

        // if (errno == EAGAIN) system("reboot");
        
        return EXIT_FAILURE;
    }

    if (setlogmask(0) & LOG_MASK(LOG_DEBUG)) {
        if (portname != NULL) log_print(LOG_PREPROCESSOR_MACROS, LOG_DEBUG, "port: %s: sended data: '%s'", portname, queue_msg.mtext);
        else log_print(LOG_PREPROCESSOR_MACROS, LOG_DEBUG, "sended data: '%s'", queue_msg.mtext);
    }


    return EXIT_SUCCESS;
}
