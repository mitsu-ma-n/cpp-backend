#pragma once

#include <boost/log/trivial.hpp>     // для BOOST_LOG_TRIVIAL
#include <boost/log/utility/setup/file.hpp>     // add_file_log()
#include <boost/log/utility/manipulators/add_value.hpp> // Вывод в поток манипулятора (logging::add_value)

#include <boost/json.hpp>

#include <string_view>

// Настраиваем собственный форматер
// Упрощение вывода атрибутов в собственном форматере
BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", unsigned int)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
// Мы можем добавить и свои атрибуты:
BOOST_LOG_ATTRIBUTE_KEYWORD(file, "File", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(line, "Line", int)

BOOST_LOG_ATTRIBUTE_KEYWORD(additional_data, "AdditionalData", boost::json::value)

//class Logger {
//public:
//    Logger() {};
namespace logger {


    void InitBoostLogFilter();

    void MyFormatter(boost::log::record_view const& rec, boost::log::formatting_ostream& strm);
};

