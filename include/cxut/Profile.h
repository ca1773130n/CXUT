#pragma once

#include "Pattern.h"
#include <glog/logging.h>
#include <iostream>
#include <map>

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#include <sys/time.h>
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

#define PROFILE(name, code) { AutoTimer _UniqueTimer(name); code }
#define PROFILE_START(name) StopWatch::getInstance()->startTimer(name);
#define PROFILE_END(name) StopWatch::getInstance()->stopTimer(name);
#define PROFILE_PAUSE(name) StopWatch::getInstance()->pauseTimer(name);
#define PROFILE_STAT() StopWatch::getInstance()->printResults();

namespace cxut {
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
	struct timezone {
			int tz_minuteswest;
			int tz_dsttime;
		};

		struct timeval {
			long tv_sec;
			long tv_usec;
		};

		int gettimeofday(struct timeval *tv, struct timezone *tz) {
			FILETIME ft;
			unsigned __int64 tmpres = 0;
			static int tzflag;

			if (NULL != tv) {
				GetSystemTimeAsFileTime(&ft);

				tmpres |= ft.dwHighDateTime;
				tmpres <<= 32;
				tmpres |= ft.dwLowDateTime;

				tmpres -= DELTA_EPOCH_IN_MICROSECS;
				tmpres /= 10;

				tv->tv_sec = (tmpres / 1000000UL);
				tv->tv_usec = (tmpres % 1000000UL);
			}

			if (NULL != tz) {
				if (!tzflag) {
					_tzset();
					tzflag++;
				}
				tz->tz_minuteswest = _timezone / 60;
				tz->tz_dsttime = _daylight;
			}
			return 0;
		}
#endif

	class Timer {
	public:
		Timer(std::string name) {
			mName.assign(name);
			reset();
		}

		void reset(void) {
			gettimeofday(&mStartTime, NULL);
		}

		float getElapsed(bool print = false) {
			long mSec = 0;
			gettimeofday(&mCurTime, NULL);
			mSec = (mCurTime.tv_sec - mStartTime.tv_sec) * 1000000 + (mCurTime.tv_usec - mStartTime.tv_usec);
			if (print)
				LOG(INFO) << "Time elapsed for [" << mName << "] : " << mSec / 1000.f << "msec";
			return mSec / 1000.f;
		}

	protected:
		struct timeval mStartTime;
		struct timeval mCurTime;
		size_t mElapsedMsec;
		std::string mName;
	};

	class StopWatch;
	class StopWatch : public ISingleton<StopWatch> {
	public:
		StopWatch() {}
		~StopWatch() {
			for (auto t : mTimers) {
				delete t.second;
			}
		}

		void startTimer(const char *name) {
			std::string strName(name);
			if (mTimers[strName] != NULL) {
				mTimers[strName]->reset();
			}
			else {
				Timer *newTimer = new Timer(strName);
				mTimers[strName] = newTimer;
			}
		}

		void pauseTimer(const char *name) {
			std::string strName(name);
			mResults[strName] += mTimers[strName]->getElapsed();
		}

		void stopTimer(const char *name) {
			std::string strName(name);
			mResults[strName] = mTimers[strName]->getElapsed();
		}

		void printResults(bool flush = true) {
			for (auto t : mResults)
				LOG(INFO) << t.first.c_str() << " : [" << t.second << "] ms";
			std::cout << std::flush;
		}

	protected:
		std::map<std::string, Timer *> mTimers;
		std::map<std::string, float> mResults;
	};

	class AutoTimer : public Timer {
	public:
		AutoTimer(char *name) : Timer(name) {
		}

		~AutoTimer() {
			getElapsed();
		}

	protected:
	};
}
