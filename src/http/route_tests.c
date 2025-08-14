#include "req.h"
#include "route.h"
#include <assert.h>
#include <stdlib.h>

void validate_uri_test1(void *p) {

    request_t *request = (request_t *)malloc(sizeof(request_t));
    
    request->bytes = "/";
    request->uri_pos = 0;

    assert(validate_uri(request) == 0);

    free(request);
}
