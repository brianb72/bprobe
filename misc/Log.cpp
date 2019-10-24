#include <stdio.h>
#include <stdarg.h>
#include "misc/log.h"

/* Log class - There are two log priorites, hi and lo.
   They will only be logged if the priority is set to their level */


/* Constructor - Set the fp to null */
cLog::cLog()
{
	fpLog = NULL;
	priority = LOGPRI_NONE;
}

/* Destructor - Close the file if it exists */
cLog::~cLog()
{
   CloseLog();
}

int cLog::OpenLog(char *szName)
{
	fpLog = fopen(szName, "wt");
	if (fpLog == NULL) return 0;
	return 1;
}

void cLog::CloseLog()
{
	if (fpLog != NULL) {
       fclose(fpLog);
	   fpLog = NULL;
	}
}

void cLog::SetPriority(int value)
{
   priority = value;
}


/* Log a string as long as file is open, regardless of priority */
void cLog::Log(char *fmt, ...)
{
	if (fpLog != NULL) {
		va_list argp;
		va_start(argp, fmt);
		vfprintf(fpLog, fmt, argp);
		va_end(argp);
		fprintf(fpLog, "\n");
	}
}

/* Log a string if priority is Lo or Hi */
void cLog::LogLo(char *fmt, ...)
{
	if ((fpLog != NULL) & (priority >= LOGPRI_LO)) {
		va_list argp;
		va_start(argp, fmt);
		vfprintf(fpLog, fmt, argp);
		va_end(argp);
		fprintf(fpLog, "\n");
	}
}


void cLog::LogHi(char *fmt, ...) 
{
	if ((fpLog != NULL) & (priority >= LOGPRI_HI)) {
		va_list argp;
		va_start(argp, fmt);
		vfprintf(fpLog, fmt, argp);
		va_end(argp);
		fprintf(fpLog, "\n");
	}
}
