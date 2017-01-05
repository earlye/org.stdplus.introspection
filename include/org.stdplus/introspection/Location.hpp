#ifndef h13658C32_5423_4926_B6FA_6FE6A30C5952
#define h13658C32_5423_4926_B6FA_6FE6A30C5952

#include <memory>
#include <string>
#include <thread>

namespace org
{
  namespace stdplus
  {
    namespace introspection
    {
      class Location
      {
      public:
        typedef std::shared_ptr<Location> shared_ptr;
        typedef std::shared_ptr<Location const> shared_ptr_const;

        static shared_ptr_const getLocationFromStack(int position);
        static shared_ptr_const getLocationFromAddress(void* address);

        void* getAddress() const;
        std::string getFilename() const;
        int getLine() const;
        std::string getModule() const;
        std::string getSymbol() const;

        class Builder
        {
        public:
          Builder();
          Location::shared_ptr_const build() const;
          Builder& setAddress(void* value);
          Builder& setFilename(std::string const& value);
          Builder& setLine(int value);
          Builder& setSymbol(std::string const& value);
          Builder& setModule(std::string const& value);
        private:
          Location::shared_ptr m_result;
        };
        static Builder builder();

      private:
        Location();
        void* m_address;
        std::string m_filename;
        int m_line;
        std::string m_module;
        std::string m_symbol;
      };
    } // namespace introspection
  } // namespace logging
} // namespace org

#endif
