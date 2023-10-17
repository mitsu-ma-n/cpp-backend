#pragma once

#include <boost/log/trivial.hpp>     // для BOOST_LOG_TRIVIAL

#include "json_fields.h"
#include "request_handler.h"
#include <boost/json.hpp>
#include <boost/json/value.hpp>

#include "server_params.h"
#include "logger.h"

// После добавления декоратора:

namespace http_handler {

template<class SomeRequestHandler>
class LoggingRequestHandler {
     static void LogRequest(const StringRequest& r);
     static void LogResponse(const StringRequest& r);
public:
    LoggingRequestHandler(SomeRequestHandler& hend) : decorated_(hend) {}

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
    // 
          boost::json::object request_jobjext;
          request_jobjext[json_field::REQUEST_IP] = "endpoint.address().to_string()";
          request_jobjext[json_field::REQUEST_URI] = std::string(req.target());
          request_jobjext[json_field::REQUEST_METHOD] = req.method_string();
          // timestamp 1
          //utils::Timer t;
          // .. .. . . .
          BOOST_LOG_TRIVIAL(info)  << boost::log::add_value(additional_data, boost::json::value(request_jobjext)) 
                                   << server_params::REQUEST_RECEIV_MESSAGE;
          boost::json::object response_jobject;
          auto loggingResponse = [&](auto&& response) {
               // timer stop
               // fill LOG JSON
               response_jobject[json_field::RESPONSE_TIME] = "resp time";
               response_jobject[json_field::RESPONSE_CODE] = response.result_int();
               response_jobject[json_field::RESPONSE_CONTENT_TYPE] = response[http::field::content_type];
               send(response);
          };
          // передаём хендлеру наш собственный рычаг для работы 
          decorated_(std::forward<decltype(req)>(req), std::move(loggingResponse));
          BOOST_LOG_TRIVIAL(info)  << boost::log::add_value(additional_data, boost::json::value(response_jobject))
                                   << server_params::RESPONSE_SENT_MESSAGE;
     }

private:
     SomeRequestHandler& decorated_;
};

}  // namespace http_handler
