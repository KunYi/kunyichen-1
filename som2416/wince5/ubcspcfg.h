/* If we wish to use CRC's, then change 0 to 1 in the next line */
#define UBCSP_CRC 1

/* Define some basic types - change these for your architecture */
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef signed char int8;
typedef signed short int16;
typedef signed int int32;

/* The defines below require a printf function to be available */

/* Do we want to show packet errors in debug output */
#define SHOW_PACKET_ERRORS	1

/* Do we want to show Link Establishment State transitions in debug output */
#define SHOW_LE_STATES		1

