
#ifndef HTTP_CONSTANTS_HPP_
#define HTTP_CONSTANTS_HPP_

#include <map>
#include <http_parser.h>


namespace
{

//===============================================================================
#define HTTP_METHODS_MAP(XX)             \
    /* base */                           \
    XX(DELETE,         HTTP_DELETE)      \
    XX(GET,            HTTP_GET)         \
    XX(HEAD,           HTTP_HEAD)        \
    XX(POST,           HTTP_POST)        \
    XX(PUT,            HTTP_PUT)         \
                                         \
    /* pathological */                   \
    XX(CONNECT,        HTTP_CONNECT)     \
    XX(OPTIONS,        HTTP_OPTIONS)     \
    XX(TRACE,          HTTP_TRACE)       \
                                         \
    /* webdav */                         \
    XX(COPY,           HTTP_COPY)        \
    XX(LOCK,           HTTP_LOCK)        \
    XX(MKCOL,          HTTP_MKCOL)       \
    XX(MOVE,           HTTP_MOVE)        \
    XX(PROPFIND,       HTTP_PROPFIND)    \
    XX(PROPPATCH,      HTTP_PROPPATCH)   \
    XX(SEARCH,         HTTP_SEARCH)      \
    XX(UNLOCK,         HTTP_UNLOCK)      \
                                         \
    /* subversion */                     \
    XX(REPORT,         HTTP_REPORT)      \
    XX(MKACTIVITY,     HTTP_MKACTIVITY)  \
    XX(CHECKOUT,       HTTP_CHECKOUT)    \
    XX(MERGE,          HTTP_MERGE)       \
                                         \
    /* upnp */                           \
    XX(MSEARCH,        HTTP_MSEARCH)     \
    XX(NOTIFY,         HTTP_NOTIFY)      \
    XX(SUBSCRIBE,      HTTP_SUBSCRIBE)   \
    XX(UNSUBSCRIBE,    HTTP_UNSUBSCRIBE) \
                                         \
    /* RFC-5789 */                       \
    XX(PATCH,          HTTP_PATCH)       \
    XX(PURGE,          HTTP_PURGE)


//===============================================================================
#define HTTP_STATUS_CODE_MAP(XX)                                                              \
    /* 1xx: Informational - Request received, continuing process */                           \
    XX(100, CODE_100_CONTINUE, "100 Continue")                                                          \
    XX(101, CODE_101_SWITCHING_PROTOCOLS, "101 Switching Protocols")                                    \
    XX(102, CODE_102_PROCESSING, "102 Processing")                                                      \
                                                                                              \
    /* 2xx: Success - The action was successfully received, understood, and accepted */       \
    XX(200, CODE_200_OK, "200 OK")                                                                      \
    XX(201, CODE_201_CREATED, "201 Created")                                                            \
    XX(202, CODE_202_ACCEPTED, "202 Accepted")                                                          \
    XX(203, CODE_203_NON_AUTHORITATIVE_INFORMATION, "203 Non-Authoritative Information")                \
    XX(204, CODE_204_NO_CONTENT, "204 No Content")                                                      \
    XX(205, CODE_205_RESET_CONTENT, "205 Reset Content")                                                \
    XX(206, CODE_206_PARTIAL_CONTENT, "206 Partial Content")                                            \
    XX(207, CODE_207_MULTI_STATUS, "207 Multi-Status")                                                  \
    XX(208, CODE_208_ALREADY_REPORTED, "208 Already Reported")                                          \
    XX(226, CODE_226_IM_USED, "226 IM Used")                                                            \
                                                                                              \
    /* 3xx: Redirection - Further action must be taken in order to complete the request */    \
    XX(300, CODE_300_MULTIPLE_CHOICES, "300 Multiple Choices")                                          \
    XX(301, CODE_301_MOVED_PERMANENTLY, "301 Moved Permanently")                                        \
    XX(302, CODE_302_FOUND, "302 Found")                                                                \
    XX(303, CODE_303_SEE_OTHER, "303 See Other")                                                        \
    XX(304, CODE_304_NOT_MODIFIED, "304 Not Modified")                                                  \
    XX(305, CODE_305_USE_PROXY, "305 Use Proxy")                                                        \
    XX(307, CODE_307_TEMPORARY_REDIRECT, "307 Temporary Redirect")                                      \
    XX(308, CODE_308_PERMANENT_REDIRECT, "308 Permanent Redirect")                                      \
                                                                                              \
    /* 4xx: Client Error - The request contains bad syntax or cannot be fulfilled */          \
    XX(400, CODE_400_BAD_REQUEST, "400 Bad Request")                                                    \
    XX(401, CODE_401_UNAUTHORIZED, "401 Unauthorized")                                                  \
    XX(402, CODE_402_PAYMENT_REQUIRED, "402 Payment Required")                                          \
    XX(403, CODE_403_FORBIDDEN, "403 Forbidden")                                                        \
    XX(404, CODE_404_NOT_FOUND, "404 Not Found")                                                        \
    XX(405, CODE_405_METHOD_NOT_ALLOWED, "405 Method Not Allowed")                                      \
    XX(406, CODE_406_NOT_ACCEPTABLE, "406 Not Acceptable")                                              \
    XX(407, CODE_407_PROXY_AUTHENTICATION_REQUIRED, "407 Proxy Authentication Required")                \
    XX(408, CODE_408_REQUEST_TIMEOUT, "408 Request Timeout")                                            \
    XX(409, CODE_409_CONFLICT, "409 Conflict")                                                          \
    XX(410, CODE_410_GONE, "410 Gone")                                                                  \
    XX(411, CODE_411_LENGTH_REQUIRED, "411 Length Required")                                            \
    XX(412, CODE_412_PRECONDITION_FAILED, "412 Precondition Failed")                                    \
    XX(413, CODE_413_PAYLOAD_TOO_LARGE, "413 Payload Too Large")                                        \
    XX(414, CODE_414_URI_TOO_LONG, "414 URI Too Long")                                                  \
    XX(415, CODE_415_UNSUPPORTED_MEDIA_TYPE, "415 Unsupported Media Type")                              \
    XX(416, CODE_416_RANGE_NOT_SATISFIABLE, "416 Range Not Satisfiable")                                \
    XX(417, CODE_417_EXPECTATION_FAILED, "417 Expectation Failed")                                      \
    XX(422, CODE_422_UPROCESSABLE_ENTITY, "422 Unprocessable Entity")                                   \
    XX(423, CODE_423_LOCKED, "423 Locked")                                                              \
    XX(424, CODE_424_FAILED_DEPENDENCY, "424 Failed Dependency")                                        \
    XX(426, CODE_426_UPGRADE_REQUIRED, "426 Upgrade Required")                                          \
    XX(428, CODE_428_PRECONDITION_REQUIRED, "428 Precondition Required")                                \
    XX(429, CODE_429_TOO_MANY_REQUESTS, "429 Too Many Requests")                                        \
    XX(431, CODE_431_REQUEST_HEADER_FIELDS_TOO_LARGE, "431 Request Header Fields Too Large")            \
                                                                                              \
    /* 5xx: Server Error - The server failed to fulfill an apparently valid request */        \
    XX(500, CODE_500_INTERNAL_SERVER_ERROR, "500 Internal Server Error")                                \
    XX(501, CODE_501_NOT_IMPLEMENTED, "501 Not Implemented")                                            \
    XX(502, CODE_502_BAD_GATEWAY, "502 Bad Gateway")                                                    \
    XX(503, CODE_503_SERVICE_UNAVAILABLE, "503 Service Unavailable")                                    \
    XX(504, CODE_504_GATEWAY_TIMEOUT, "504 Gateway Timeout")                                            \
    XX(505, CODE_505_HTTP_VERSION_NOT_SUPPORTED, "505 HTTP Version Not Supported")                      \
    XX(506, CODE_506_VARIANT_ALSO_NEGOTIATES, "506 Variant Also Negotiates")                            \
    XX(507, CODE_507_INSUFFICIENT_STORAGE, "507 Insufficient Storage")                                  \
    XX(508, CODE_508_LOOP_DETECTED, "508 Loop Detected")                                                \
    XX(510, CODE_510_NOT_EXTENDED, "510 Not Extended")                                                  \
    XX(511, CODE_511_NETWORK_AUTHENTICATION_REQUIRED, "511 Network Authentication Required")

}

namespace http
{
//===============================================================================
#define HTTP_METHODS_ENUM_GEN(enum_val, http_method_val) enum_val,
enum class Method
{
    UNKNOWN,
    HTTP_METHODS_MAP(HTTP_METHODS_ENUM_GEN)
};
#undef HTTP_METHODS_ENUM_GEN

//===============================================================================
#define HTTP_METHODS_MAP_GEN(enum_val, http_method_val) {http_method_val, Method::enum_val},
static const std::map<http_method, Method> method_map = {
    HTTP_METHODS_MAP(HTTP_METHODS_MAP_GEN)
};
#undef HTTP_METHODS_MAP_GEN

//===============================================================================
#define HTTP_STATUS_CODE_ENUM_GEN(num, enum_val, text_val) enum_val,
enum class StatusCode
{
    UNKNOWN,
    HTTP_STATUS_CODE_MAP(HTTP_STATUS_CODE_ENUM_GEN)
};
#undef HTTP_STATUS_CODE_ENUM_GEN

//===============================================================================
#define HTTP_STATUS_CODE_TO_STRING_GEN(num, enum_val, text_val) {StatusCode::enum_val, text_val},
static const std::map<StatusCode, std::string> status_code_to_sting = {
    HTTP_STATUS_CODE_MAP(HTTP_STATUS_CODE_TO_STRING_GEN)
};
#undef HTTP_STATUS_CODE_TO_STRING_GEN

//===============================================================================
#define HTTP_NUM_TO_STATUS_CODE_GEN(num, enum_val, text_val) {num, StatusCode::enum_val},
static const std::map<unsigned int, StatusCode> num_to_status_code = {
    HTTP_STATUS_CODE_MAP(HTTP_NUM_TO_STATUS_CODE_GEN)
};
#undef HTTP_NUM_TO_STATUS_CODE_GEN

}

#endif //HTTP_CONSTANTS_HPP_
