#ifndef KEYMAP_READER_HPP
#define KEYMAP_READER_HPP

#include <map>
#include <string>
#include <fstream>
#include "picojson.h"

struct Coord
{
    int row;
    int col;
};

class KeyMapReader
{
public:
    /// @brief converts map.json into keymap
    /// @param filename map.json
    /// @return map of keys
    static std::map<int, Coord> loadFromJson(const std::string &filename)
    {
        std::map<int, Coord> keymap;
        std::ifstream file(filename);

        if (!file.is_open())
            return keymap;

        picojson::value v;
        std::string err = picojson::parse(v, file);

        if (!err.empty() || !v.is<picojson::object>())
        {
            return keymap;
        }

        const picojson::object &obj = v.get<picojson::object>();

        for (auto it = obj.begin(); it != obj.end(); ++it)
        {
            int id = std::stoi(it->first);
            if (it->second.is<picojson::object>())
            {
                const picojson::object &coords = it->second.get<picojson::object>();

                Coord c;
                c.row = static_cast<int>(coords.at("row").get<double>());
                c.col = static_cast<int>(coords.at("col").get<double>());

                keymap[id] = c;
            }
        }

        return keymap;
    }
};

#endif