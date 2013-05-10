#pragma once

#include "antiattack.h"

#define CFG_PLUGIN_VERSION	"1.2.1"
#define CFG_SERVER_IP		"127.0.0.1"
#define CFG_SERVER_PORT		7777
#define CFG_SERVER_OWNER	"Windows test user"



bool aat_logging, aat_clearLog;
int aat_seekOffset;
void **ppPluginData;
extern void	*pAMXFunctions;
logprintf_t logprintf;



struct attstruct
{
	cell attackType;
	std::string data;
};



std::queue<attstruct> amxQueue;
std::list<AMX*> amxList;



#ifdef WIN32
	HANDLE threadHandle;
	HANDLE mutexHandle;
	DWORD __stdcall AttackCheckThread(LPVOID lpParam);
#else
	pthread_mutex_t mutexHandle;
	void * AttackCheckThread(void *lpParam);
#endif



void lockMutex();
void unlockMutex();
void destroyMutex();
void aat_Debug(char *text, ...);





PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() 
{
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK;
}



PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData) 
{
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];

	aat_logging = true;
	aat_clearLog = false;
	aat_seekOffset = -96;

	aat_Debug("\n\t������� �������� ������� Anti-Attack �������...");
	aat_Debug("������ �������: %s", CFG_PLUGIN_VERSION);
	aat_Debug("��������� ������...");

	std::fstream initFile;
	std::string initData;

	size_t initFind;
	short isLoaded = 0;

	aat_Debug("������ ������� ��������! �������� ��������...");

	initFile.open("server.cfg", std::fstream::in);

	while(std::getline(initFile, initData))
	{
		if(initData.find('\r') != std::string::npos)
		{
			initData.resize((initData.length() - 1));
		}

		if(initData.find('\n') != std::string::npos)
		{
			initData.resize((initData.length() - 1));
		}

		initFind = initData.find("bind ");

		if(initFind != std::string::npos)
		{
			if(isLoaded & 1)
			{
				aat_Debug("������ ��� �������� �������� (���� 'bind' � server.cfg ����������)");
				aat_Debug("������ ������� ����������!");
				aat_Debug("\t������� �������� ������� Anti-Attack ����������...");

				logprintf("   Anti-Attack Error: Initialization failed (Duplicating 'bind' key)");

				initFile.close();
				initData.clear();

				return 0;
			}

			initData.erase(0, (initFind + 5));

			if(initData.find(CFG_SERVER_IP))
			{
				aat_Debug("������ ��� �������� �������� (���� 'bind' (%s) �����������)", initData.c_str());
				aat_Debug("������ ������� ����������!");
				aat_Debug("\t������� �������� ������� Anti-Attack ����������...");

				logprintf("   Anti-Attack Error: Initialization failed (Invalid bind IP: %s)", initData.c_str());

				initFile.close();
				initData.clear();

				return 0;
			}

			isLoaded |= 1;
		}

		initFind = initData.find("port ");

		if(initFind != std::string::npos)
		{
			if(isLoaded & 2)
			{
				aat_Debug("������ ��� �������� �������� (���� 'port' � server.cfg ����������)");
				aat_Debug("������ ������� ����������!");
				aat_Debug("\t������� �������� ������� Anti-Attack ����������...");

				logprintf("   Anti-Attack Error: Initialization failed (Duplicating 'port' key)");

				initFile.close();
				initData.clear();

				return 0;
			}

			initData.erase(0, (initFind + 5));

			if(atoi(initData.c_str()) != CFG_SERVER_PORT)
			{
				aat_Debug("������ ��� �������� �������� (���� 'port' (%i) �����������)", atoi(initData.c_str()));
				aat_Debug("������ ������� ����������!");
				aat_Debug("\t������� �������� ������� Anti-Attack ����������...");

				logprintf("   Anti-Attack Error: Initialization failed (Invalid port: %i)", atoi(initData.c_str()));

				initFile.close();
				initData.clear();

				return 0;
			}

			isLoaded |= 2;
		}

		initFind = initData.find("logtimeformat ");

		if(initFind != std::string::npos)
		{
			initData.erase(0, (initFind + 14));

			aat_seekOffset -= (initData.length() + 4);
		}
	}

	if(isLoaded != 3)
	{
		aat_Debug("������ ��� �������� �������� (�� ������ ���� 'bind', ��� 'port')");
		aat_Debug("������ ������� ����������!");
		aat_Debug("\t������� �������� ������� Anti-Attack ����������...");

		logprintf("   Anti-Attack Error: Initialization failed (No 'bind' or 'port' value)");

		initFile.close();
		initData.clear();

		return 0;
	}

	initFile.close();
	initData.clear();

	aat_Debug("�������� ����������� �� IP: %s:%i (�������� ��������: %s)", CFG_SERVER_IP, CFG_SERVER_PORT, CFG_SERVER_OWNER);
	logprintf("  Plugin licensed to: %s:%i (%s)", CFG_SERVER_IP, CFG_SERVER_PORT, CFG_SERVER_OWNER);

	#ifdef WIN32
		DWORD dwThreadId = 0;
		threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AttackCheckThread, NULL, 0, &dwThreadId);
		CloseHandle(threadHandle);
		mutexHandle = CreateMutex(NULL, FALSE, LPCWSTR("aat"));
	#else
		pthread_t threadHandle;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&threadHandle, &attr, &AttackCheckThread, NULL);
		mutexHandle = PTHREAD_MUTEX_INITIALIZER;
	#endif

	aat_Debug("������ ������� ����������!\n");
	logprintf("  Anti-Attack plugin by BJIADOKC loaded.");

	return 1;
}



PLUGIN_EXPORT void PLUGIN_CALL Unload()
{
	destroyMutex();
	amxList.clear();

	#ifdef WIN32
		TerminateThread(threadHandle, (DWORD)0);
	#endif

	aat_Debug("������ ������� ����������!");
	aat_Debug("\t������� �������� ������� Anti-Attack ����������...");

	logprintf("  Anti-Attack plugin by BJIADOKC unloaded.");
}



PLUGIN_EXPORT void PLUGIN_CALL ProcessTick()
{
	if(amxQueue.size() > 0) 
	{
		aat_Debug("� ProcessTick() ���� ������, ���� ���������...");

		for(unsigned int i = 0; i < amxQueue.size(); i++)
		{
			lockMutex();
			attstruct toamx = amxQueue.front();
			amxQueue.pop();
			unlockMutex();

			cell amx_Address, amx_Ret, *phys_addr;
			int amx_Idx;

			for(std::list<AMX*>::iterator amx = amxList.begin(); amx != amxList.end(); amx++) 
			{
				if(toamx.attackType > 4)
				{
					if(amx_FindPublic(*amx, "OnRemoteAttackAttempt", &amx_Idx) == AMX_ERR_NONE) 
					{
						amx_PushString(*amx, &amx_Address, &phys_addr, toamx.data.c_str(), 0, 0);
						amx_Push(*amx, (toamx.attackType - 5));

						amx_Exec(*amx, &amx_Ret, amx_Idx);
						amx_Release(*amx, amx_Address);
					}
				}
				else
				{
					if(amx_FindPublic(*amx, "OnIngameAttackAttempt", &amx_Idx) == AMX_ERR_NONE) 
					{
						amx_Push(*amx, (cell)atoi(toamx.data.c_str()));
						amx_Push(*amx, toamx.attackType);

						amx_Exec(*amx, &amx_Ret, amx_Idx);
					}
				}
			}
		}

		aat_Debug("������ � ProcessTick() ������� ����������");
	}
}



#ifdef WIN32
	DWORD __stdcall AttackCheckThread(LPVOID lpParam)
#else
	void * AttackCheckThread(void *lpParam)
#endif
{
	attstruct found;
	size_t find, sub;
	std::fstream file;

	while(true) 
	{
		file.open("server_log.txt", std::fstream::in | std::fstream::ate);
		file.seekg(aat_seekOffset, std::fstream::cur);

		while(std::getline(file, found.data))
		{
			if(found.data.length() < 28 || found.data.length() > (size_t)abs(aat_seekOffset) || found.data.find("[chat]") != std::string::npos) continue;

			find = found.data.find("Warning: PlayerDialogResponse PlayerId: ");
			sub = found.data.find(" dialog ID doesn't match last sent dialog ID");

			if(find != std::string::npos && sub != std::string::npos && find < sub)
			{
				found.attackType = 0;
				found.data.erase(0, (find + 40));
				found.data.erase((sub - 40));

				lockMutex();
				amxQueue.push(found);
				unlockMutex();

				aat_Debug("������ %i:%s ���������� � ProcessTick()", found.attackType, found.data.c_str());

				SLEEP(500);

				if(aat_clearLog)
				{
					aat_Debug("��� ������� ������� ������");

					file.close();
					file.open("server_log.txt", std::fstream::out | std::fstream::trunc);
				}

				break;
			}

			find = found.data.find("Warning: PlayerDialogResponse crash exploit from PlayerId: ");

			if(find != std::string::npos)
			{
				found.attackType = 1;
				found.data.erase(0, (find + 59));

				lockMutex();
				amxQueue.push(found);
				unlockMutex();

				aat_Debug("������ %i:%s ���������� � ProcessTick()", found.attackType, found.data.c_str());

				SLEEP(500);

				if(aat_clearLog)
				{
					aat_Debug("��� ������� ������� ������");

					file.close();
					file.open("server_log.txt", std::fstream::out | std::fstream::trunc);
				}

				break;
			}
			
			find = found.data.find("Warning: /rcon command exploit from: ");

			if(find != std::string::npos)
			{
				found.attackType = 2;
				found.data.erase(0, (find + 37));
				found.data.erase(found.data.find(":"));

				lockMutex();
				amxQueue.push(found);
				unlockMutex();

				aat_Debug("������ %i:%s ���������� � ProcessTick()", found.attackType, found.data.c_str());

				SLEEP(500);

				if(aat_clearLog)
				{
					aat_Debug("��� ������� ������� ������");

					file.close();
					file.open("server_log.txt", std::fstream::out | std::fstream::trunc);
				}

				break;
			}

			find = found.data.find("Packet was modified, sent by id: ");
			sub = found.data.find(", ip: ");

			if(find != std::string::npos && sub != std::string::npos && find < sub)
			{
				found.attackType = 3;
				found.data.erase(0, (find + 33));
				found.data.erase((sub - 33));

				lockMutex();
				amxQueue.push(found);
				unlockMutex();

				aat_Debug("������ %i:%s ���������� � ProcessTick()", found.attackType, found.data.c_str());

				SLEEP(500);

				if(aat_clearLog)
				{
					aat_Debug("��� ������� ������� ������");

					file.close();
					file.open("server_log.txt", std::fstream::out | std::fstream::trunc);
				}

				break;
			}

			find = found.data.find("Remote Port Refused for Player: ");

			if(find != std::string::npos)
			{
				found.attackType = 4;
				found.data.erase(0, (find + 32));

				lockMutex();
				amxQueue.push(found);
				unlockMutex();

				aat_Debug("������ %i:%s ���������� � ProcessTick()", found.attackType, found.data.c_str());

				SLEEP(500);

				if(aat_clearLog)
				{
					aat_Debug("��� ������� ������� ������");

					file.close();
					file.open("server_log.txt", std::fstream::out | std::fstream::trunc);
				}

				break;
			}

			find = found.data.find("BAD RCON ATTEMPT BY: ");

			if(find != std::string::npos)
			{
				found.attackType = 5;
				found.data.erase(0, (find + 21));

				lockMutex();
				amxQueue.push(found);
				unlockMutex();

				aat_Debug("������ %i:%s ���������� � ProcessTick()", found.attackType, found.data.c_str());

				SLEEP(500);

				if(aat_clearLog)
				{
					aat_Debug("��� ������� ������� ������");

					file.close();
					file.open("server_log.txt", std::fstream::out | std::fstream::trunc);
				}
				
				break;
			}

			find = found.data.find("Invalid client connecting from ");

			if(find != std::string::npos)
			{
				found.attackType = 6;
				found.data.erase(0, (find + 31));

				lockMutex();
				amxQueue.push(found);
				unlockMutex();

				aat_Debug("������ %i:%s ���������� � ProcessTick()", found.attackType, found.data.c_str());

				SLEEP(500);

				if(aat_clearLog)
				{
					aat_Debug("��� ������� ������� ������");

					file.close();
					file.open("server_log.txt", std::fstream::out | std::fstream::trunc);
				}

				break;
			}

			find = found.data.find("Blocking ");
			sub = found.data.find(" due to a 'server full' attack");

			if(find != std::string::npos && sub != std::string::npos && find < sub)
			{
				found.attackType = 7;
				found.data.erase(0, (find + 9));
				found.data.erase((sub - 9));

				lockMutex();
				amxQueue.push(found);
				unlockMutex();

				aat_Debug("������ %i:%s ���������� � ProcessTick()", found.attackType, found.data.c_str());

				SLEEP(500);

				if(aat_clearLog)
				{
					aat_Debug("��� ������� ������� ������");

					file.close();
					file.open("server_log.txt", std::fstream::out | std::fstream::trunc);
				}

				break;
			}
		}

		file.close();
		
		SLEEP(5);
	}

}



cell AMX_NATIVE_CALL aat_n_NetStats(AMX* amx, cell* params)
{
	if(!arguments(1))
	{
		aat_Debug("������ ��� �������� ���������� � native aat_NetStats");
		logprintf("Anti-Attack Warning: Number of arguments in function aat_NetStats does not conform to definition");

		return 0;
	}

    int len = NULL;
    cell *addr  = NULL;

    amx_GetAddr(amx, params[1], &addr); 
    amx_StrLen(addr, &len);

    if(len++)
    {
        char *text = new char[len];

        amx_GetString(text, addr, 0, len);
		std::string netStats = std::string(text);
		aat_Debug(" ");
		aat_Debug(text);
		aat_Debug(" ");
		delete text;

		netStats.erase(0, (netStats.find("Inst. KBits per second: ") + 25));
		netStats.erase(netStats.find('\n'));

		return (cell)atof(netStats.c_str());
    }

	return 0;
}



cell AMX_NATIVE_CALL aat_n_ClearLog(AMX* amx, cell* params)
{
	if(!arguments(1))
	{
		aat_Debug("������ ��� �������� ���������� � native aat_ClearLog");
		logprintf("Anti-Attack Warning: Number of arguments in function aat_ClearLog does not conform to definition");

		return 0;
	}

	if((params[1] && aat_clearLog) || (!params[1] && !aat_clearLog)) return 0;

	aat_clearLog = (params[1]) ? true : false;

	aat_Debug("������� ���� ��� ����������� ����� %s", aat_clearLog ? ("��������") : ("���������"));
	logprintf("Anti-Attack: OnAttack log cleaning %s", aat_clearLog ? ("enabled") : ("disabled"));

	return 1;
}



cell AMX_NATIVE_CALL aat_n_Logging(AMX* amx, cell* params)
{
	if(!arguments(1))
	{
		aat_Debug("������ ��� �������� ���������� � native aat_Logging");
		logprintf("Anti-Attack Warning: Number of arguments in function aat_Logging does not conform to definition");

		return 0;
	}

	if((params[1] && aat_logging) || (!params[1] && !aat_logging)) return 0;

	aat_Debug("\n\t������� �������� ������� Anti-Attack %s...", (params[1]) ? ("�������") : ("����������"));

	aat_logging = (params[1]) ? true : false;

	logprintf("Anti-Attack: logging %s from script", aat_logging ? ("enabled") : ("disabled"));

	return 1;
}



#if defined __cplusplus
	extern "C"
#endif



const AMX_NATIVE_INFO Natives[] = 
{
	{"aat_NetStats", aat_n_NetStats},
	{"aat_Logging", aat_n_Logging},
	{"aat_ClearLog", aat_n_ClearLog},
	{NULL, NULL}
};



PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx)
{
	amxList.push_back(amx);

	return amx_Register(amx, Natives, -1);

}



PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx) 
{
	for(std::list<AMX*>::iterator i = amxList.begin(); i != amxList.end(); i++) 
	{
		if(*i == amx) 
		{
			amxList.erase(i);

			break;
		}
	}

	return AMX_ERR_NONE;
}



void lockMutex()
{
	#ifdef WIN32
		WaitForSingleObject(mutexHandle, INFINITE);
	#else
		pthread_mutex_lock(&mutexHandle);
	#endif
}



void unlockMutex()
{
	#ifdef WIN32
		ReleaseMutex(mutexHandle);
	#else
		pthread_mutex_unlock(&mutexHandle);
	#endif
}



void destroyMutex()
{
	#ifdef WIN32
		CloseHandle(mutexHandle);
	#else
		pthread_mutex_destroy(&mutexHandle);
	#endif
}



void aat_Debug(char *text, ...)
{
	if(aat_logging)
	{
		va_list args;
		std::fstream logfile;

		va_start(args, text);
		std::string buffer = stringvprintf(text, args);
		va_end(args);

		logfile.open("attack_log.txt", std::fstream::out | std::fstream::app);

		logfile << buffer << '\n';
		logfile.flush();
		logfile.close();
	}
}