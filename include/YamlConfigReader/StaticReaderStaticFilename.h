//
// Created by tin on 3/08/22.
//

#ifndef SIMPLE_YMAL_CONFIG_READER_SIMPLEYAMLCONFIGREADER_H
#define SIMPLE_YMAL_CONFIG_READER_SIMPLEYAMLCONFIGREADER_H

#include <iostream>

#include "helpers.h"
#include "soraxas_toolbox/compile_time_string.h"

namespace sxs
{

    template <typename Tag, typename Tag2, typename... Args>
    class _ConfigReaderRegisterClass
    {
    protected:
        static _ConfigReaderRegisterClass instance;

        template <typename F>
        _ConfigReaderRegisterClass(const F &callback)
        {
            callback.template __register<Args...>();
        }

    public:
        static const _ConfigReaderRegisterClass &doRegister()
        {
            return instance;
        }
    };

    template <typename FileNameWithin64char, typename SecondFileNameWithin64char = void>
    class StaticConfigReaderStaticFilename
    {
    public:
        template <typename Key, typename T>
        static const T &get()
        {
            static const T &thing = []
            {
                _ConfigReaderRegisterClass<FileNameWithin64char, SecondFileNameWithin64char, Key, T>::doRegister();
                const auto &&node = _get_root()[Key::c_str()];
                if (!node.IsDefined())
                {
                    throw std::runtime_error(std::string("The key '") + Key::c_str() +
                                             "' does not exists in the config file.");
                }
                try
                {
                    return node.template as<T>();
                }
                catch (...)
                {
                    std::string info =
                        std::string() + "Trying to cast the key " + Key::c_str() + " into type " + typeid(T).name();
                    //                std::cerr << "!!! The following exception happens for
                    //                the key " << Key::c_str() << " !!!" << std::endl;
                    throw;
                }
            }();
            return thing;
        }

        operator std::string() const
        {
            std::stringstream ss;
            ss << "=== File: " << _get_filename() << " ===" << std::endl;
            ss << _get_root() << std::endl;
            ss << "==========" << std::endl;
            return ss.str();
        }

        friend std::ostream &operator<<(std::ostream &_stream, const StaticConfigReaderStaticFilename &t)
        {
            _stream << std::string(t);
            return _stream;
        }

        template <typename Key, typename T>
        static void __register()
        {
            // eager evaluate in static allocation
            get<Key, T>();
        }

    private:
        StaticConfigReaderStaticFilename() = delete;

        static auto &_get_filename()
        {
            static std::string thing;
            return thing;
        };

        static auto &_get_root()
        {
            static YAML::Node thing = []()
            {
                auto root = YAML::LoadFile(FileNameWithin64char::c_str());
                _merge_node_if_second_file_is_given<SecondFileNameWithin64char>(root);
                return root;
            }();

            return thing;
        };

        template <typename T>
        static void _merge_node_if_second_file_is_given(YAML::Node &node)
        {
            auto overriding_node = YAML::LoadFile(T::c_str());
            node = merge_nodes(node, overriding_node);
        }

        template <>
        static void _merge_node_if_second_file_is_given<void>(YAML::Node &node)
        {
        }
    };

    template <typename File1, typename File2, typename... Args>
    _ConfigReaderRegisterClass<File1, File2, Args...> _ConfigReaderRegisterClass<File1, File2, Args...>::instance{
        StaticConfigReaderStaticFilename<File1, File2>{}};
}  // namespace sxs

#endif  // SIMPLE_YMAL_CONFIG_READER_SIMPLEYAMLCONFIGREADER_H
