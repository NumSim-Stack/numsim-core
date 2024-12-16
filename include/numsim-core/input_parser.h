#ifndef INPUT_PARSER_H
#define INPUT_PARSER_H

#include <iostream>
#include <string>
#include <map>
#include <algorithm>

namespace numsim_core {

static inline auto remove_character(std::string && input, char const& key){
    input.erase(std::remove(input.begin(), input.end(), key), input.end());
    return input;
}

class input_parser{
public:
    input_parser (int &argc, char **argv){
        for (int i=1; i < argc; ++i){
            std::string key{remove_character(std::string(argv[i]), '-')};
            if(i < argc-1 && std::string(argv[i+1]).find("-")){
                ++i;
                m_arguments[key] = std::string(argv[i]);
            }else{
                m_arguments[key] = "";
            }
        }
    }

    const std::string& value(const std::string & key) const{
        const auto pos{m_arguments.find(key)};
        if(pos == m_arguments.end()){
            throw std::runtime_error("input_parser::value() no matching input found");
        }
        return pos->second;
    }

    bool contains(const std::string & key) const{
        return m_arguments.find(key) != m_arguments.cend();
    }

    inline void add_help(std::string && key, std::string && name, std::string && description){
        m_help[std::move(key)] = std::make_pair(std::move(name), std::move(description));
    }

    void print()const{
        for(const auto& [key, value] : m_arguments){
            std::cout<<key<<" "<<value<<std::endl;
        }
    }

    void print_help()const{
        for(const auto& [key, description] : m_help){
            std::cout<<key<<" "<<description.first<<" "<<description.second<<std::endl;
        }
    }


private:
    std::map<std::string, std::string> m_arguments;
    std::map<std::string, std::pair<std::string, std::string>> m_help;
};

}

#endif // INPUT_PARSER_H
