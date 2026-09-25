#pragma once

#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace utils {
    inline std::string generate_uuid() {
        boost::uuids::random_generator generator;

        return boost::uuids::to_string(generator());
    }
}