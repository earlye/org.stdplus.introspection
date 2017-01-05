#include <org.stdplus/introspection/Location.hpp>

#include <vector>

namespace org
{
  namespace stdplus
  {
    namespace introspection
    {
      Location::Location()
      {
      }

      std::string Location::getFilename() const
      {
        return m_filename;
      }

      int Location::getLine() const
      {
        return m_line;
      }

      void* Location::getAddress() const
      {
        return m_address;
      }

      std::string Location::getModule() const
      {
        return m_module;
      }

      std::string Location::getSymbol() const
      {
        return m_symbol;
      }

      Location::Builder Location::builder()
      {
        return Builder();
      }

      Location::Builder::Builder()
        : m_result( new Location() )
      { }

      Location::shared_ptr_const Location::Builder::build() const
      {
        return m_result;
      }

      Location::Builder& Location::Builder::setAddress(void* value)
      {
        m_result->m_address = value;
        return *this;
      }

      Location::Builder& Location::Builder::setFilename(std::string const& value)
      {
        m_result->m_filename = value;
        return *this;
      }

      Location::Builder& Location::Builder::setLine(int value)
      {
        m_result->m_line = value;
        return *this;
      }

      Location::Builder& Location::Builder::setModule(std::string const& value)
      {
        m_result->m_module = value;
        return *this;
      }

      Location::Builder& Location::Builder::setSymbol(std::string const& value)
      {
        m_result->m_symbol = value;
        return *this;
      }

    } // namespace threads
  } // namespace stdplus
} // namespace org
