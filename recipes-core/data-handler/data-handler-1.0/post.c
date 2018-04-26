#include "post.h"
#include "rnox/log.h"

#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <stdlib.h>

#define _GNU_SOURCE
#include <stdio.h>
int asprintf(char **strp, const char *fmt, ...);

// url to server
#define POST_URL "http://rtb.adx1.com/services/druid/ingestion/realtime?datasource=rnox_test_2"

static CURL *post_curl = NULL;                          // curl handle
static struct curl_slist *post_headers = NULL;          // http headers to send with request



int post_sessionOpen(void) {
    if (post_curl != NULL) return EXIT_SUCCESS;

    // init curl handle
    if ((post_curl = curl_easy_init()) == NULL) {
        // log error
        log_print(LOG_MSG_ERR, "failed to create curl handle");
        return EXIT_FAILURE;
    }

    // set content type
    post_headers = curl_slist_append(post_headers, "Accept: application/json");
    post_headers = curl_slist_append(post_headers, "Content-Type: application/json");

    // set curl options
    curl_easy_setopt(post_curl, CURLOPT_CUSTOMREQUEST, "POST");

    curl_easy_setopt(post_curl, CURLOPT_HTTPHEADER, post_headers);

    // set url to fetch
    curl_easy_setopt(post_curl, CURLOPT_URL, POST_URL);

    // set default user agent
    curl_easy_setopt(post_curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    // set timeout
    curl_easy_setopt(post_curl, CURLOPT_TIMEOUT, 5);

    // enable location redirects
    curl_easy_setopt(post_curl, CURLOPT_FOLLOWLOCATION, 1);

    // set maximum allowed redirects
    curl_easy_setopt(post_curl, CURLOPT_MAXREDIRS, 1);

    return EXIT_SUCCESS;
}
void post_sessionClose(void) {
    // cleanup curl handle
    curl_easy_cleanup(post_curl);
    // free headers
    curl_slist_free_all(post_headers);

    post_curl = NULL;
    post_headers = NULL;
}



int post(const char * const data/*, const char * const portname*/) {
    if (post_curl == NULL) return EXIT_FAILURE;

    CURLcode rcode = CURLE_OK;      // curl result code


    curl_easy_setopt(post_curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(post_curl, CURLOPT_POSTFIELDSIZE, (long)strlen(data));

    // fetch page and capture return code
    // TODO: valgrind: 1 errors 1 contexts
    rcode = curl_easy_perform(post_curl);


    if(rcode != CURLE_OK) {
        log_print(LOG_MSG_ERR, "cannot send data to server: '%s'", data);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


