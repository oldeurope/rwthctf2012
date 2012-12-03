#ifndef __GLOBALS_H_INCLUDED__
#define __GLOBALS_H_INCLUDED__

#ifndef MAIN_PORT
#define MAIN_PORT 30300
#endif

#ifndef LOG_PORT
#define LOG_PORT 30301
#endif

// Logger send timeout (in usec)
#ifndef LOG_SEND_TIMEOUT
#define LOG_SEND_TIMEOUT 1500000
#endif

#ifndef FUCK_YOU_MESSAGE
#define FUCK_YOU_MESSAGE ((const unsigned char*)"go fuck yourself")
#endif
#define FUCK_YOU_LENGTH (strlen((const char*)FUCK_YOU_MESSAGE))

#ifndef PLANT_EVIDENCE_MESSAGE
#define PLANT_EVIDENCE_MESSAGE ((const unsigned char*)"plant evidence")
#endif

#define PLANT_EVIDENCE_LENGTH (strlen((const char*)PLANT_EVIDENCE_MESSAGE))

#ifndef EXTRACT_EVIDENCE_MESSAGE
#define EXTRACT_EVIDENCE_MESSAGE ((const unsigned char*)"extract evidence")
#endif

#define EXTRACT_EVIDENCE_LENGTH (strlen((const char*)PLANT_EVIDENCE_MESSAGE))

#endif
