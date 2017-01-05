#include <org.stdplus/introspection/compiler/target.hpp>

#if STDPLUS_INTROSPECTION_COMPILER_TARGET == STDPLUS_INTROSPECTION_COMPILER_TARGET_WINDOWS
#include <org.stdplus/introspection/Location.hpp>

#include <windows.h>

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
        ULONG framesToSkip = position; // The number of frames to skip from the start of the back trace.

        ULONG framesToCapture = 1; // The number of frames to be captured. You can capture up to MAXUSHORT frames.
        // Windows Server 2003 and Windows XP:  The sum of the FramesToSkip and FramesToCapture parameters must be less than 63.

        PVOID address = NULL;
        PVOID backTrace = &address; // An array of pointers captured from the current stack trace.

        ULONG backTraceHash; // A value that can be used to organize hash tables. If this parameter is NULL, then no hash value is computed.
        // This value is calculated based on the values of the pointers returned in the BackTrace array. Two identical stack traces will generate identical hash values.

        USHORT result = CaptureStackBackTrace(framesToSkip,framesToCapture,&backTrace,&backTraceHash);

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
	    shared_ptr_const result = builder()
	      .setAddress(address)
	      .build();
          }
        else
          {
            return it->second;
          }

      }
    }
  }
}

#endif
