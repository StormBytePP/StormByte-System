#pragma once

#include <definitions.h>

#ifdef WINDOWS
	#ifdef StormByteSystem_EXPORTS
		#define STORMBYTE_SYSTEM_PUBLIC	__declspec(dllexport)
  	#else
      	#define STORMBYTE_SYSTEM_PUBLIC	__declspec(dllimport)
  	#endif
  	#define STORMBYTE_SYSTEM_PRIVATE
#else
    #define STORMBYTE_SYSTEM_PUBLIC		__attribute__ ((visibility ("default")))
    #define STORMBYTE_SYSTEM_PRIVATE		__attribute__ ((visibility ("hidden")))
#endif
