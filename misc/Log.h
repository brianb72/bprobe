#ifndef __LOG_H__
#define __LOG_H__

#define LOGPRI_NONE 0
#define LOGPRI_LO   1
#define LOGPRI_HI   2


class cLog {
public:
	cLog();
	~cLog();
    void Log(char *fmt, ...);
	void LogLo(char *fmt, ...);
	void LogHi(char *fmt, ...);
	int  OpenLog(char *szName);
    void CloseLog();
	void SetPriority(int value);
private:
	FILE *fpLog;
	int priority;
};


#endif