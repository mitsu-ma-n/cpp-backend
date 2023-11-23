#include "logger.h"

#include <boost/log/utility/setup/common_attributes.hpp>    // add_common_attributes()
#include <boost/log/utility/setup/console.hpp>  // add_console_log()
#include <boost/date_time.hpp>  // Для вывода момента времени

#include <boost/json.hpp>
#include "boost/json/serialize.hpp"
#include "json_fields.h"

#include <string_view>

namespace json = boost::json;

namespace logger {

using namespace std::literals;
namespace logging = boost::log;

// Настройка фильтров вывода сообщений логгера
void InitBoostLogFilter() {
//    logging::core::get()->set_filter(
//        logging::trivial::severity >= logging::trivial::info
//    );
}

// tag_invoke должны быть определны в том же namespace, в котором определны классы,
// которые ими обрабатываются. В наше случае это model
// сериализация сообщения логгера в JSON-значение
namespace boost::log {
void tag_invoke(json::value_from_tag, json::value& jv, logging::record_view const& rec)
{
    std::stringstream ss;
    ss << rec[timestamp];
    std::stringstream ss2;
    ss2 << rec[logging::expressions::smessage];
    jv = {
        {json_field::LOGGER_TIMESTAMP, ss.str()},
        {json_field::LOGGER_DATA, ss2.str()}
    };
}

}

// Функция, задающая форматирование сообщений логгера
void MyFormatter(logging::record_view const& rec, logging::formatting_ostream& strm) {
    json::object jResponseObj;

    // Время
    auto ts = *rec[timestamp];
    jResponseObj[json_field::LOGGER_TIMESTAMP] = to_iso_extended_string(ts);
    // Дополнительная информация в формате json::value.
    jResponseObj[json_field::LOGGER_DATA] = *rec[additional_data];
    // Выводим само сообщение.
    jResponseObj[json_field::LOGGER_MESSAGE] = *rec[logging::expressions::smessage];

    strm << json::value(jResponseObj);
} 

}