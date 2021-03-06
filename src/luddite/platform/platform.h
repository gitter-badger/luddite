#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdarg.h>
#include <stdint.h>

#include <string>


//---[ Assert ]------------------------------------

#ifdef WIN32
# define TRIGGER_BREAKPOINT { __asm { int 3 }; }
#elif __APPLE__
#  include "TargetConditionals.h"
#  if TARGET_CPU_ARM
//   iOS - ARM
#    define TRIGGER_BREAKPOINT { asm("trap"); }

#  elif TARGET_CPU_ARM64
//   iOS - ARM64
#    define TRIGGER_BREAKPOINT { asm("svc 0"); }

#  elif TARGET_CPU_X86 | TARGET_CPU_X86_64

//    iOS Simulator or OSX
#     define TRIGGER_BREAKPOINT { asm ("int $3"); }

# endif
#elif __linux
# define TRIGGER_BREAKPOINT { asm ("int $3"); }
#endif

// Assert Macro
#define Assert( expr, desc ) \
	{ \
		static bool _skipAlways = false; \
		if (!_skipAlways) { \
            if (pfAssertFunc( !!(expr), (desc), __LINE__, __FILE__, &_skipAlways ) ) { \
				TRIGGER_BREAKPOINT \
			} \
		} \
	}

// Shorthand for checking a pointer is non-null
#define AssertPtr( ptr ) Assert( ((ptr)!=NULL), #ptr " is NULL" )

// Platform specific assert function, used by assert macros. Don't call
// directly, use Assert() instead.
// -- If expr is true, do nothing. If expr is false print out desc and 
// other info (and perhaps a stack trace). 
bool pfAssertFunc( bool expr, const char *desc, int line, const char *file, bool *skip );

//---[ Debug Print ]------------------------------------
// Prints a message at the given severity level
void pfVMessage( int message_level, const char *fmt, va_list args);

//---[ Graphics/texture stuff ]------------------------------------

// Loads a texture and returns a GL handle to it.
// Returns 0 on error
uint32_t pfLoadTexture( const char *filename );

//---[ Filesystem stuff ]------------------------------------
// need to rethink this, maybe when i switch to SDL2 I can
// just use their filesystem stuff.

std::string pfGetResourcePath();

std::string pfPathToResource( const char *resource );

#endif
