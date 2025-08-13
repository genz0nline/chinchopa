#include "status.h"
#include "../http/resp.h"

int set_reason_phrase(response_t *response) {
    switch (response->status) {
        case 200:
            response->reason_phrase = "Ok";
            return 0;
        case 404:
            response->reason_phrase = "Not Found";
            return 0;
        case 403:
            response->reason_phrase = "Forbidden";
            return 0;
        case 500:
            response->reason_phrase = "Internal error";
            return 0;
        case 505:
            response->reason_phrase = "HTTP Version Not Supported";
            return 0;
        default:
            return 1;
    }
}
