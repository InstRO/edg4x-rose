// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          github.com:matzke1/sawyer.




#include <sawyer/Sawyer.h>
#include <sawyer/Message.h>
#include <sawyer/Synchronization.h>

namespace Sawyer {

static void
init() {
  Message::initializeLibrary();
}

#if SAWYER_MULTI_THREADED
static boost::once_flag initFlag = BOOST_ONCE_INIT;
#endif

// thread-safe
SAWYER_EXPORT bool
initializeLibrary() {
#if SAWYER_MULTI_THREADED
    boost::call_once(&init, initFlag);
#else
    init();
#endif
    return true;
}

// Presence/absense of strtoll and strtoull depends on the compiler rather than the target environment.   For instance, MinGW
// has strtoll and strtoull and lacks _strtoi64 and _strtoui64 even though both MinGW and MVC are targeting a Windows
// environment.
SAWYER_EXPORT boost::int64_t
strtoll(const char *input, char **rest, int base) {
#ifdef _MSC_VER
    return _strtoi64(input, rest, base);
#else
    return ::strtoll(input, rest, base);
#endif
}

SAWYER_EXPORT boost::uint64_t
strtoull(const char *input, char **rest, int base) {
#ifdef _MSC_VER
    return _strtoui64(input, rest, base);
#else
    return ::strtoull(input, rest, base);
#endif
}

SAWYER_EXPORT std::string
readOneLine(FILE *stream) {
    std::string retval;
    while (1) {
        int c = fgetc(stream);
        if (c < 0)
            break;
        retval += (char)c;
        if ('\n'==c)
            break;
    }
    return retval;
}

SAWYER_EXPORT FILE*
popen(const std::string &cmd, const char *how) {
#ifdef BOOST_WINDOWS
    return ::_popen(cmd.c_str(), how);
#else
    return ::popen(cmd.c_str(), how);
#endif
}

SAWYER_EXPORT int
pclose(FILE *f) {
#ifdef BOOST_WINDOWS
    return ::_pclose(f);
#else
    return ::pclose(f);
#endif
}

SAWYER_EXPORT std::string
generateSequentialName(size_t length) {
    static size_t ncalls = 0;
    size_t sequence = ncalls++;
    std::string letters(length, 'a');
    for (size_t i=0; i<length && sequence; ++i, sequence/=26)
        letters[length-(i+1)] += sequence % 26;
    return letters;
}


} // namespace
