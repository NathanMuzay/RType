#include "DataWriter.hpp"

#include <fstream>
#include <system_error>

#include "DataWriterImpl.hpp"
#include "SceneError.hpp"

namespace engine::scene {

nlohmann::json &DataWriter::Impl::slot(const std::string &section, const std::string &key)
{
    if (section.empty())
        return m_root[key];
    return m_root[section][key];
}

const nlohmann::json &DataWriter::Impl::root() const
{
    return m_root;
}

DataWriter::DataWriter() : m_impl(std::make_unique<Impl>())
{
}

DataWriter::~DataWriter() = default;

void DataWriter::setString(const std::string &section, const std::string &key, const std::string &value)
{
    m_impl->slot(section, key) = value;
}

void DataWriter::setNumber(const std::string &section, const std::string &key, float value)
{
    m_impl->slot(section, key) = value;
}

void DataWriter::setInteger(const std::string &section, const std::string &key, int value)
{
    m_impl->slot(section, key) = value;
}

void DataWriter::setBool(const std::string &section, const std::string &key, bool value)
{
    m_impl->slot(section, key) = value;
}

void DataWriter::save(const std::filesystem::path &path) const
{
    std::filesystem::path temporary = path;
    temporary += ".tmp";
    {
        std::ofstream file(temporary, std::ios::trunc);
        if (!file)
            throw SceneError("DataWriter: cannot write " + temporary.string());
        file << m_impl->root().dump(Impl::INDENT) << '\n';
        if (!file)
            throw SceneError("DataWriter: cannot write " + temporary.string());
    }
    std::error_code error;
    std::filesystem::rename(temporary, path, error);
    if (error)
        throw SceneError("DataWriter: cannot replace " + path.string() + ": " + error.message());
}

} // namespace engine::scene
