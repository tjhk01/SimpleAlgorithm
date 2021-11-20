#include <iostream>
#include <vector>
#include <algorithm>
#include <Windows.h>

using namespace std;
typedef struct
{
	int No;
	long startNum;
	long endNum;
}stCalcNum;

// User can modify following 2 MACROs
#define MAX_NUMBER	(400000000)
#define MAX_THREADS	(200)

CRITICAL_SECTION CriticalSec;
DWORD WINAPI PerfectNumCalcThread(LPVOID lpParam)
{
	vector<long> DivisorVec;
	register long DivisorSum = 0;
	register long cnt = 0;
	long percentage_cur = 0;
	long percentage_last = 0;
	long total_iterations = 0;
	stCalcNum CalcNum = *(stCalcNum *)lpParam;

	cout << "Start " << CalcNum.startNum << "\tEnd " << CalcNum.endNum << endl;
	EnterCriticalSection(&CriticalSec);		//Starting Signal
	LeaveCriticalSection(&CriticalSec);
	for (cnt = CalcNum.startNum; cnt < CalcNum.endNum; cnt += 2)
	{
		//Find Divisor
		for (long div_cnt = 1; div_cnt * div_cnt <= cnt; div_cnt++)
		{
			total_iterations++;
			if ((cnt % div_cnt) == 0)				//Found One of Divisor
			{
				DivisorVec.push_back(div_cnt);
				DivisorSum += div_cnt;
				if (cnt != div_cnt * div_cnt)
				{
					long div_tmp = cnt / div_cnt;	//Store Another Divisor at sametime
					total_iterations++;
					if (div_cnt != 1)				//Ignore Self
					{
						DivisorVec.push_back(div_tmp);
						DivisorSum += div_tmp;
					}
				}
			}
		}

		if ((DivisorVec.size() > 0) && (DivisorSum == cnt))
		{
			vector<long>::iterator it_cnt = DivisorVec.begin();
			cout << *it_cnt;

			for (; it_cnt != DivisorVec.end(); it_cnt++)
			{
				cout << " - " << *it_cnt;
			}
			cout << endl;
			cout << "Perfect Number Found :" << DivisorSum << endl;
		}
		DivisorVec.clear();
		DivisorSum = 0;

		EnterCriticalSection(&CriticalSec);
		percentage_cur = ((cnt - CalcNum.startNum) * 100) / (CalcNum.endNum - CalcNum.startNum);
		if (percentage_cur != percentage_last)
		{
			SYSTEMTIME time;
			GetLocalTime(&time);
			cout << "Thread [" << CalcNum.No << "] : [" << time.wHour << ":" << time.wMinute << ":" << time.wSecond << "]" << percentage_cur << "%" << endl;
			percentage_last = percentage_cur;
		}
		LeaveCriticalSection(&CriticalSec);
	}

	cout << "End of Thread [" << GetCurrentThreadId() << "] with [" << total_iterations << "] times iteration" << endl;

	return 0;
}


int main(void)
{
	HANDLE  hThreadArray[MAX_THREADS];
	DWORD   dwThreadIdArray[MAX_THREADS];
	static stCalcNum Num;
	
	memset(&Num, 0, sizeof(Num));
	InitializeCriticalSection(&CriticalSec);

	EnterCriticalSection(&CriticalSec);
	for (int i = 0; i < MAX_THREADS; i++)
	{
		Num.startNum = 2 + Num.endNum;
		Num.endNum = (i + 1) * (MAX_NUMBER / MAX_THREADS);
		Num.No = i + 1;
		hThreadArray[i] = CreateThread(
			NULL,                   // default security attributes
			0,                      // use default stack size  
			PerfectNumCalcThread,   // thread function name
			&Num,					// argument to thread function 
			0,                      // use default creation flags 
			&dwThreadIdArray[i]);   // returns the thread identifier 
		Sleep(50);
	}
	LeaveCriticalSection(&CriticalSec);
	while (1);
	return 0;
}