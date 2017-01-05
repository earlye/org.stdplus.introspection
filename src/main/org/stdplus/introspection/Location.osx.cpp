#include <org.stdplus/introspection/compiler/target.hpp>

#if STDPLUS_INTROSPECTION_COMPILER_TARGET == STDPLUS_INTROSPECTION_COMPILER_TARGET_OSX
#include <org.stdplus/introspection/Location.hpp>

#include <org.stdplus/posix/Run.hpp>

#include <execinfo.h>
#include <stdlib.h>
#include <unistd.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace
{
  // Normally, we would use <org.stdplus/strings/trim>, but
  // I'd like org.stdplus.introspection not to depend on org.stdplus.strings
  std::string string_trim( std::string const& source )
  {
    std::string::size_type beg = source.find_first_not_of(" \r\n\t");
    std::string::size_type end = source.find_last_not_of(" \r\n\t");
    return source.substr(beg,end-beg+1);
  }

}

namespace org
{
  namespace stdplus
  {
    namespace introspection
    {
      typedef std::unordered_map<void*,Location::shared_ptr_const> AddressOutputCache;
      AddressOutputCache addressOutputCache;

      Location::shared_ptr_const Location::getLocationFromStack(int position)
      {
        std::vector<void*> trace(position+1);
        size_t size = backtrace(trace.data(),trace.size());
        if ( position > size )
          throw std::runtime_error("stack position out of bounds");
        return getLocationFromAddress(trace[position]);
      }



      Location::shared_ptr_const Location::getLocationFromAddress(void* address)
      {
        AddressOutputCache::const_iterator it = addressOutputCache.find(address);
        std::string output;
        if (it == addressOutputCache.end())
          {
            std::stringstream pid; pid << getpid();
            std::stringstream address_str; address_str << std::hex << address;

            std::vector<std::string> command;
            command.push_back("/usr/bin/atos");
            command.push_back("-p");
            command.push_back(pid.str());
            command.push_back(address_str.str());

            org::stdplus::posix::Run atos(command);
            if (0 == atos.wait())
              {
                atos.readStdout();

                std::string outputRaw = atos.getStdout();
                std::string output = string_trim(outputRaw);

                std::string::size_type file_line_paren = output.find_last_of("(");

                std::string file_line = output.substr(file_line_paren+1);
                file_line.erase(file_line.length()-1,1);

                std::string::size_type colon = file_line.find_last_of(":");
                std::string filename = file_line.substr(0,colon);
                std::string line_str = file_line.substr(colon+1);

                long line = atol(line_str.c_str());

                std::string::size_type module_paren = output.find_last_of("(",file_line_paren-1);
                std::string module = output.substr(module_paren,file_line_paren - module_paren -1);
                module.erase(0,4);
                module.erase(module.length()-1,1);
                std::string method = output.substr(0,module_paren-1);

                shared_ptr_const result = builder()
                  .setFilename(filename)
                  .setLine(line)
                  .setSymbol(method)
                  .setAddress(address)
                  .setModule(module)
                  .build();
                addressOutputCache.insert(std::make_pair(address,result));
                return result;
              }
            else
              {
                shared_ptr_const result = builder()
                  .setAddress(address)
                  .build();
		
                addressOutputCache.insert(std::make_pair(address,result));
                return result;
              }

          }
        else
          {
            return it->second;
          }
      }
    }
  }
}
#else
#error Need to implement getLocationFromAddress for this platform
#endif
