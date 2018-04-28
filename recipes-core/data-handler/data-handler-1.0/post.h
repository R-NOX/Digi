#ifndef POST_H
#define POST_H

/* open CURL session
 *
 * inputs: void

 * returns:
 *      exit code */
int post_sessionOpen(void);
/* open CURL session
 *
 * inputs: void

 * returns: void */
void post_sessionClose(void);

/* post data to server
 *
 * inputs:
 *      data        i   post data
 *      portname    i   portn name

 * returns:
 *      exit code */
int post(const char * const data/*, const char * const portname*/);

int test_connection_to_server();


#endif // POST_H
