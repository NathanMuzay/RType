#pragma once

#include <filesystem>
#include <memory>
#include <string>

namespace engine::scene {

class DataWriter
{
public:
    DataWriter();
    ~DataWriter();
    DataWriter(const DataWriter &) = delete;
    DataWriter &operator=(const DataWriter &) = delete;

    void setString(const std::string &section, const std::string &key, const std::string &value);
    void setNumber(const std::string &section, const std::string &key, float value);
    void setInteger(const std::string &section, const std::string &key, int value);
    void setBool(const std::string &section, const std::string &key, bool value);
    void save(const std::filesystem::path &path) const;

private:
    class Impl;

    std::unique_ptr<Impl> m_impl;
};

} // namespace engine::scene
