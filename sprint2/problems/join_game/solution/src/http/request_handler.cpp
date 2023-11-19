#include "request_handler.h"

#include "boost/beast/http/status.hpp"
#include <boost/beast/http/file_body.hpp>
#include <boost/json.hpp>
#include "boost/json/serialize.hpp"
#include <boost/url.hpp>
#include <iostream>
#include <string_view>

#include "boost/json/value_from.hpp"
#include "json_fields.h"

namespace json = boost::json;

namespace http_handler {

StringResponse RequestHandler::ReportServerError(unsigned version, bool keep_alive) const {
    return StringResponse();
}

}  // namespace http_handler
