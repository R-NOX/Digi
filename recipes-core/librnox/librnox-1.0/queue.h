#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>

#define QUEUE_MAX_BUFFER_SIZE   4079

typedef struct msgbuf {
    long    mtype;
    char    mtext[QUEUE_MAX_BUFFER_SIZE];
} queue_t;

/* create new channel
 *
 * inputs:
 *      handle      o       queue file handle
 *
 * returns:
 *      exit code */
int queue_create(int* handle);
/* open channel
 *
 * inputs:
 *      handle      o       queue file handle
 *
 * returns:
 *      exit code */
int queue_open(int* handle);


/* destroy channel
 *
 * inputs:
 *      handle      i       queue file handle
 *
 * returns:
 *      exit code */
int queue_destroy(int handle);


/* get message
 *
 * inputs:
 *      handle      i       queue file handle
 *      buffer      o       buffer for msg
 *
 * returns:
 *      size of the received message */
int queue_get_msg(int handle, queue_t *queue_msg);

/* put message
 *
 * inputs:
 *      handle      i       queue file handle
 *      msg         i       message
 *
 * returns:
 *      exit code */
int queue_put_msg(/*int handle, */const char *msg, const char * const portname);


#endif // QUEUE_H
