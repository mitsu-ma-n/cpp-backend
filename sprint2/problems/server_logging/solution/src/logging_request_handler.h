#pragma once

#include "request_handler.h"

// После добавления декоратора:

namespace http_handler {

template<class SomeRequestHandler>
class LoggingRequestHandler {
     static void LogRequest(const StringRequest& r);
     static void LogResponse(const StringRequest& r);
public:
    LoggingRequestHandler(SomeRequestHandler& hend) : decorated_(hend) {

    }

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
         LogRequest(req);
         decorated_(std::move(req),std::move(send));
         LogResponse(send);
     }

private:
     SomeRequestHandler& decorated_;
};

}  // namespace http_handler
