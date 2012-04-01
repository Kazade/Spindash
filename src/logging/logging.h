#ifndef LOGGING_H_INCLUDED
#define LOGGING_H_INCLUDED

#include <string>
#include <boost/thread.hpp>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>

namespace logging {

enum LOG_LEVEL {
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4
};

class Logger;

class Handler {
public:
    typedef boost::shared_ptr<Handler> ptr;

    virtual ~Handler() {}
    void write_message(Logger* logger,
                       const boost::posix_time::ptime& time,
                       const std::string& level,
                       const std::string& message) {

        assert(logger);
        do_write_message(logger, time, level, message);
    }

private:
    virtual void do_write_message(Logger* logger,
                       const boost::posix_time::ptime& time,
                       const std::string& level,
                       const std::string& message) = 0;
};

class StdIOHandler : public Handler {
private:
    void do_write_message(Logger* logger,
                       const boost::posix_time::ptime& time,
                       const std::string& level,
                       const std::string& message) {

        if(level == "ERROR") {
            std::cerr << boost::posix_time::to_simple_string(time) << " ERROR " << message << std::endl;
        } else {
            std::cout << boost::posix_time::to_simple_string(time) << " " << level << " " << message << std::endl;
        }
    }
};

class FileHandler : public Handler {
public:
    FileHandler(const std::string& filename, bool move_aside=true):
        filename_(filename) {

        if(move_aside && boost::filesystem::exists(filename_)) {
            if(boost::filesystem::exists(filename_ + ".old")) {
                boost::filesystem::remove(filename_ + ".old");
            }
            boost::filesystem::rename(filename_, filename_ + ".old");
        }

        stream_.open(filename_.c_str());
        assert(stream_.good());
    }

private:
    void do_write_message(Logger* logger,
                       const boost::posix_time::ptime& time,
                       const std::string& level,
                       const std::string& message) {

        assert(stream_.good());
        stream_ << boost::posix_time::to_simple_string(time) << " " << level << " " << message << std::endl;
        stream_.flush();
    }

    std::string filename_;
    std::ofstream stream_;
};

class Logger {
public:
    typedef boost::shared_ptr<Logger> ptr;

    Logger(const std::string& name):
        name_(name),
        level_(LOG_LEVEL_DEBUG) {

    }

    void add_handler(Handler::ptr handler) {
        //FIXME: check it doesn't exist already
        handlers_.push_back(handler);
    }

    void debug(const std::string& text, const std::string& file="None", int32_t line=-1) {
        if(level_ == LOG_LEVEL_NONE) return;

        write_message("DEBUG", text, file, line);
    }

    void info(const std::string& text, const std::string& file="None", int32_t line=-1) {
        if(level_ == LOG_LEVEL_NONE) return;
        if(level_ == LOG_LEVEL_WARN || level_ == LOG_LEVEL_ERROR) return;

        write_message("INFO", text, file, line);
    }

    void warn(const std::string& text, const std::string& file="None", int32_t line=-1) {
        if(level_ == LOG_LEVEL_NONE) return;
        if(level_ == LOG_LEVEL_ERROR) return;

        write_message("WARN", text, file, line);
    }

    void error(const std::string& text, const std::string& file="None", int32_t line=-1) {
        write_message("ERROR", text, file, line);
    }

    void set_level(LOG_LEVEL level) {
        level_ = level;
    }

private:
    void write_message(const std::string& level, const std::string& text,
                       const std::string& file, int32_t line) {

        std::string file_out = file;
        if(file != "None") {
            boost::filesystem::path p(file);
            file_out = p.filename().string();
        }

        std::stringstream s;
        s << boost::this_thread::get_id() << ": ";
        s << text << " (" << file_out << ":" << boost::lexical_cast<std::string>(line) << ")";
        for(uint32_t i = 0; i < handlers_.size(); ++i) {
            handlers_[i]->write_message(this, boost::posix_time::ptime(boost::posix_time::second_clock::local_time()), level, s.str());
        }
    }

    std::string name_;
    std::vector<Handler::ptr> handlers_;

    LOG_LEVEL level_;
};

Logger* get_logger(const std::string& name);

void debug(const std::string& text, const std::string& file="None", int32_t line=-1);
void info(const std::string& text, const std::string& file="None", int32_t line=-1);
void warn(const std::string& text, const std::string& file="None", int32_t line=-1);
void error(const std::string& text, const std::string& file="None", int32_t line=-1);

}

#define L_DEBUG(txt) \
    logging::debug((txt), __FILE__, __LINE__)

#define L_INFO(txt) \
    logging::info((txt), __FILE__, __LINE__)

#define L_WARN(txt) \
    logging::warn((txt), __FILE__, __LINE__)

#define L_ERROR(txt) \
    logging::error((txt), __FILE__, __LINE__)

#define L_DEBUG_N(name, txt) \
    logging::get_logger((name))->debug((txt), __FILE__, __LINE__)

#define L_INFO_N(name, txt) \
    logging::get_logger((name))->info((txt), __FILE__, __LINE__)

#define L_WARN_N(name, txt) \
    logging::get_logger((name))->warn((txt), __FILE__, __LINE__)

#define L_ERROR_N(name, txt) \
    logging::get_logger((name))->error((txt), __FILE__, __LINE__)

#endif // LOGGING_H_INCLUDED
