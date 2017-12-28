#ifndef __TYPES_H
#define __TYPES_H

#ifndef bool
typedef unsigned char bool;
#define false (0)
#define true (!false)
#endif

#ifndef ushort
typedef unsigned short ushort;
#endif

#ifndef uint
typedef unsigned int uint;
#endif


#ifndef null
#define null ((void*)0)
#endif

#endif