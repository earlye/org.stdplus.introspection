#include <org.stdplus/introspection/Location.hpp>

#include <stdplus/STDPLUS_ASSERT_EQUALS.hpp>
#include <stdplus/demangle.hpp>
#include <stdplus/format.hpp>
#include <org.stdplus/posix/basename.hpp>

#include <iostream>

void find_address(int depth);
void testLocationFromStack()
{
  using org::stdplus::introspection::Location;
  using org::stdplus::posix::basename;

  // DO NOT split the following line - the test depends on it...
  std::string file = __FILE__; int line = __LINE__; Location::shared_ptr_const value = Location::getLocationFromStack(1);

  STDPLUS_ASSERT_EQUALS(basename(file),basename(value->getFilename()));
  STDPLUS_ASSERT_EQUALS(line,value->getLine());
  STDPLUS_ASSERT_EQUALS(".test_harness.exe",value->getModule());
  STDPLUS_ASSERT_EQUALS("testLocationFromStack()",value->getSymbol());
  STDPLUS_ASSERT_EXCEPT(value->getAddress());

  Location::shared_ptr_const value2 = Location::getLocationFromAddress(value->getAddress());

  STDPLUS_ASSERT_EQUALS(basename(file),basename(value2->getFilename()));
  STDPLUS_ASSERT_EQUALS(line,value2->getLine());
  STDPLUS_ASSERT_EQUALS(".test_harness.exe",value2->getModule());
  STDPLUS_ASSERT_EQUALS("testLocationFromStack()",value2->getSymbol());
  STDPLUS_ASSERT_EXCEPT(value2->getAddress());
}

void testLocationCaching()
{
  using org::stdplus::introspection::Location;

  Location::shared_ptr_const value = Location::getLocationFromStack(1);
  Location::shared_ptr_const value2 = Location::getLocationFromAddress(value->getAddress());
  // Caching:
  STDPLUS_ASSERT_EQUALS(value.get(),value2.get());
}


#if 0
#define PACKAGE "autoconf sucks"
#define PACKAGE_VERSION "1.2.3"
#include <bfd.h>
#include <libunwind.h>
#include <sys/types.h>
#include <unistd.h>
#include <mach-o/dyld.h>

typedef char* pointer;


class AutoBfd
{
public:
  typedef std::shared_ptr< AutoBfd > shared_ptr;

  ~AutoBfd( )
  {
    bfd_close(m_handle);
  }

  AutoBfd(AutoBfd const& copy) = delete;

  bfd* get() { return m_handle; }

  static shared_ptr build( std::string const& filename )
  {
    return shared_ptr(new AutoBfd(filename));
  }

private:
  AutoBfd(std::string const& filename)
    : m_handle(open(filename))
  { }

  bfd* open(std::string const& filename)
  {
    bfd* result = bfd_openr(filename.c_str(), NULL);

    if ( result == NULL )
      throw std::runtime_error("Could not open file \"" + filename + "\""); // todo: make bfd-specific exception hierarchy.

    if ( bfd_check_format( result, bfd_archive) )
      throw std::runtime_error("\"" + filename + "\" is not bfd-compliant"); // todo: make bfd-specific exception hierarchy.

    char** matching = NULL;
    if ( !bfd_check_format_matches(result, bfd_object, &matching))
      {
        if (matching)
          {
            free(matching);
          }
        throw std::runtime_error( "\"" + filename + "\" is not bfd_object");
      }

    return result;
  }

  bfd* m_handle;
};

class AutoBfdSymbols
{
public:
  typedef std::shared_ptr< AutoBfdSymbols > shared_ptr;

  ~AutoBfdSymbols()
  { free( m_value ); }

  asymbol** get() { return m_value; }

  static shared_ptr build(AutoBfd::shared_ptr bfd)
  {
    if ( (HAS_SYMS & bfd_get_file_flags(bfd->get())) == 0)
      throw std::runtime_error("File has no symtab");

    asymbol** result = NULL;
    unsigned int size = 0;
    long count = bfd_read_minisymbols(bfd->get(),false,(void**)&result,&size);
    if (count ==0)
      count = bfd_read_minisymbols(bfd->get(),true,(void**)&result,&size);

    if ( count < 0 )
      throw std::runtime_error("Error getting minisymbols");

    if ( count == 0 )
      throw std::runtime_error("No minisymbols found");

    std::cout << "AutoBfdSymbols::build: read " << count << " symbols" << std::endl;
    // for ( long i =0; i != count ; ++i )
    //   {
    //     std::cout << "- " << i << result[i]->name << std::endl;
    //   }

    return shared_ptr(new AutoBfdSymbols(result));
  }

private:
  AutoBfdSymbols( asymbol** value )
    : m_value(value)
  { }

  asymbol** m_value;
};

struct MapOverSectionsContext
{
  pointer m_programCounter = NULL;
  bool m_found = false;
  asymbol** m_symbols = NULL;
  char const* m_filename = NULL;
  char const* m_functionName = NULL;
  unsigned int m_line = 0;
};

void find_nearest_line(bfd* bfd_, asection* aSection, void* context)
{
  MapOverSectionsContext* ctx = (MapOverSectionsContext*)context;

  if (!ctx)
    return;

  if (ctx->m_found) return;

  if ((SEC_ALLOC & bfd_get_section_flags(bfd_,aSection)) == 0)
    return;

  bfd_vma vma = bfd_get_section_vma(bfd_,aSection);
  bfd_size_type size = bfd_section_size(bfd_,aSection);

  if ((pointer)ctx->m_programCounter < (pointer)vma)
    return;

  if ((pointer)ctx->m_programCounter >= (pointer)vma+size)
    return;

  ctx->m_found = bfd_find_nearest_line
    (bfd_,
     aSection,
     ctx->m_symbols,
     (bfd_vma)ctx->m_programCounter,
     &ctx->m_filename,
     &ctx->m_functionName,
     &ctx->m_line);

  std::cout << stdplus::format("m_found:%d aSection:%p vma:%p size:%d vma+size:%p name:%s symbol:%p")
    % ctx->m_found % aSection
    % vma % size % (vma+size)
    % aSection->name % (void*)aSection->symbol
            << std::endl;

  std::cout << stdplus::format(" -- symbol -- \n  .name:%s")
    % aSection->symbol->name
            << std::endl;
}

int popen3(int fd[3],const char **const cmd);
std::string run(const char** const cmd);
std::string run(std::vector<std::string> const& cmd);

#include <stdplus/string_trim.hpp>

void* find_address(int depth)
{
  unw_cursor_t cursor;
  unw_context_t context;

  // Initialize cursor to current frame for local unwinding.
  unw_getcontext(&context);
  unw_init_local(&cursor, &context);

  pointer location = 0;
  do
    {
      unw_word_t offset, pc;
      unw_get_reg(&cursor, UNW_REG_IP, &pc);
      if (pc == 0) {
        break;
      }
      if ( depth-- == 0 ) {
        location = (pointer)pc;
      }
      char sym[256];
      if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
        std::cerr << stdplus::format("%p : %s") % (void*)pc % stdplus::demangle(sym) << std::endl;
        //std::string command  = stdplus::format("atos -p %d %p") % getpid() % (void*)pc;
        //        std::cerr << command << std::endl;
        //system(command.c_str());

        std::cout << stdplus::format("method:\"%s\" module:\"%s\" file_line:\"%s\"")
          % method % module % file_line << std::endl;
      } else {
        printf(" -- error: unable to obtain symbol name for this frame\n");
      }
    }
  while (unw_step(&cursor) > 0);

#if 0
  int num_dynamic_libraries = _dyld_image_count();
  std::cerr << stdplus::format("Num Dynamic Libraries:%d") % num_dynamic_libraries << std::endl;
  for(int i = 0; i != num_dynamic_libraries; ++i )
    {
      mach_header_64 const* header = reinterpret_cast<mach_header_64 const*>(_dyld_get_image_header(i));
      intptr_t offset = _dyld_get_image_vmaddr_slide(i);
      std::string image_name = _dyld_get_image_name(i);
      std::cerr << stdplus::format("Image:%d [%s] header:%p magic:%d offset:%p num_cmds:%d") % i % image_name % header % header->magic % offset % header->ncmds << std::endl;

      // Check the commands to see if one contains the location.

      load_command const* command = reinterpret_cast<load_command const*>(&header[1]);
      for(uint32_t j = 0;j != header->ncmds; ++j)
        {
          switch(command->cmd)
            {
            case LC_SEGMENT_64:
              {
                segment_command_64 const* segment = reinterpret_cast< segment_command_64 const* >( command );
                char* start = (char*)segment->vmaddr + offset;
                char* end = start + segment->vmsize;

                std::cerr << stdplus::format("  cmd[%d]: LC_SEGMENT_64 %15s %p %p")
                  % j
                  % segment->segname
                  % (void*)start
                  % (void*)end
                          << std::endl;

                if ( start <= location && location < end )
                  {
                    std::cerr << "  ^^^^^ THIS ONE! "<< std::endl;

                    pointer nonOffsetLocation = location - offset;
                    AutoBfd::shared_ptr autoBfd(AutoBfd::build(image_name));
                    std::cout << "    autoBfd:" << autoBfd.get() << std::endl;

                    AutoBfdSymbols::shared_ptr autoBfdSymbols(AutoBfdSymbols::build(autoBfd));

                    MapOverSectionsContext ctx = { .m_programCounter = nonOffsetLocation, .m_symbols = autoBfdSymbols->get() };
                    std::cerr << stdplus::format( "    -> bfd_map_over_sections: %p %d %s:%d\n" ) % (void*)ctx.m_programCounter % ctx.m_found % (ctx.m_filename ? ctx.m_filename : "not found") % ctx.m_line;
                    bfd_map_over_sections(autoBfd->get(),find_nearest_line,&ctx);
                    std::cerr << stdplus::format( "    <- bfd_map_over_sections: %p %d %s:%d\n" ) % (void*)ctx.m_programCounter % ctx.m_found % (ctx.m_filename ? ctx.m_filename : "not found") % ctx.m_line;
                    return;
                  }
                break;
              }
            default:
              std::cerr << stdplus::format("  cmd[%d]: 0x%x") % j % command->cmd << std::endl;
            }
          command = reinterpret_cast<load_command const*>( (char*)command + command->cmdsize);
        }

    }
#endif
}

#include <stdio.h>

std::string run(const char** const cmd)
{
  // std::cout << "args:" << std::endl;
  // for( char const** arg = cmd; *arg ; ++arg )
  //   {
  //     std::cout << (char*)*arg << std::endl;
  //   }
  int fd[3];
  int pid = popen3(fd,cmd);
  // int status=-1;
  // waitpid(pid,&status,0);
  // std::cout << "status:" << status << std::endl;

  FILE* stdin = fdopen(fd[STDIN_FILENO],"w");
  FILE* stdout = fdopen(fd[STDOUT_FILENO],"r");
  FILE* stderr = fdopen(fd[STDERR_FILENO],"r");

  // TODO: do this in separate introspection, and build a "subprocess result object"
  char buffer[1024];
  std::string result="";
  do
    {
      buffer[0] = '\0';
      fgets(buffer, sizeof(buffer), stdout);
      // std::cout << "stdout chunk:" << buffer << std::endl;
      result += buffer;
    }
  while(!feof(stdout));

  do
    {
      buffer[0] = '\0';
      fgets(buffer, sizeof(buffer), stderr);
      // std::cout << "stderr chunk:" << buffer << std::endl;
      result += buffer;
    }
  while(!feof(stderr));

  return result;
}

std::string run(std::vector<std::string> const& cmd) {
  std::vector<char const*> command;
  // std::cout << "args:" << std::endl;
  for( std::string const& arg:cmd )
    {
      // std::cout << arg << std::endl;
      command.push_back(arg.c_str());
    }
  command.push_back(NULL);
  return run(command.data());
}
#endif
