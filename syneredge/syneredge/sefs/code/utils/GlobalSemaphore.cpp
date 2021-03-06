#include "GlobalSemaphore.hpp"
#include "StringConversion.hpp"
#include "TraceThread.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/format.hpp"

#include <semaphore.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

namespace SynerEdge
{

boost::mutex GlobalSemaphore::mtx;
std::map<void *, int> GlobalSemaphore::globalMap;

GlobalSemaphore::GlobalSemaphore(
	unsigned initial, 
	const std::wstring &name, 
	enum Cleanup cleanup) 
: openExclusive(false), givenName(name)
{
	std::wcout << "at top of global semaphore constructror" << std::endl;
	boost::filesystem::path pth("/");
	pth /= StringConversion::toUTF8(name);

	std::wcout << "after pth: " << name << std::endl;
	TRACE_SETUP(L"utils");
	
	TRACE(TraceData::info, StringConversion::toUTF16(pth.native_file_string()));

	sem_t *handle = 0;

	switch(cleanup)
	{
	case GlobalSemaphore::always:
		openExclusive = true;
		handle = sem_open(pth.native_file_string().c_str(), 
				  O_CREAT | O_EXCL, 0666, initial);
		if (handle == SEM_FAILED)
		{
			sem_unlink(pth.native_file_string().c_str());
			handle = sem_open(pth.native_file_string().c_str(), 
				  O_CREAT | O_EXCL, 0666, initial);
		}
		break;
	case GlobalSemaphore::never:
		openExclusive = false;
		handle = sem_open(pth.native_file_string().c_str(), 
				  0, 0666, initial);
		break;
	case GlobalSemaphore::iffirst:
		handle = sem_open(pth.native_file_string().c_str(), 
				  O_CREAT | O_EXCL, 0666, initial);
		if (handle == SEM_FAILED)
		{
			openExclusive = false;
			handle = sem_open(pth.native_file_string().c_str(), 
				  O_CREAT, 0666, initial);
		}
		else
		{ 
			openExclusive = true;
		}
		break;
	}

	if (handle != SEM_FAILED)
	{
		TRACE(TraceData::info, L"Semaphore create success");

		void *lookup = static_cast<void *>(handle);
		{
			boost::mutex::scoped_lock lk(mtx, true);

			std::map<void *, int>::iterator itor;
			itor = globalMap.find(lookup);
			if (itor != globalMap.end())
			{
				TRACE(TraceData::info, L"Semaphore updated map");
				(*itor).second++;
			}
			else
			{
				TRACE(TraceData::info, L"Semaphore added to map");
				globalMap[lookup] = 1;
			}
		}

		initialCount = initial;		
		semHandle = lookup;
		globalName = StringConversion::toUTF16(pth.native_file_string());
	}
	else
	{
		TRACE(TraceData::info, StringConversion::syserr());
		throw GlobalSemaphoreException(StringConversion::syserr());	
	}
}

GlobalSemaphore::~GlobalSemaphore()
{
	TRACE_SETUP(L"utils");

	TRACE(TraceData::info, L"waiting on lock in destructor");
	boost::mutex::scoped_lock lk(mtx, true);

	void *lookup = static_cast<void *>(semHandle);
	std::map<void *, int>::iterator itor;
	itor = globalMap.find(lookup);
	if (itor != globalMap.end())
	{
		--((*itor).second);

		boost::wformat fmt(L"Found in map - %d");
		fmt % (*itor).second;
		TRACE(TraceData::info, fmt.str());

		if ((*itor).second == 0)
		{
			TRACE(TraceData::info, L"about to close semaphore");
			globalMap.erase(itor);
			if (sem_close(static_cast<sem_t *>(semHandle)) == -1)
			{
				TRACE(TraceData::exception, 
				StringConversion::syserr());
			}

		}
	}
	else
	{
		TRACE(TraceData::info, L"about to close semaphore");
		globalMap.erase(itor);
		if (sem_close(static_cast<sem_t *>(semHandle)) == -1)
		{
			TRACE(TraceData::exception, 
			StringConversion::syserr());
		}
	}

	if (openExclusive)
	{
		TRACE(TraceData::info, L"about to unlink semaphore");
		if (sem_unlink(StringConversion::toUTF8(globalName).c_str()) == -1)
		{
			TRACE(TraceData::exception, StringConversion::syserr());
		}
	}
}

void GlobalSemaphore::post(unsigned count)
{
	for (unsigned i = 0; i < count; i++)
	{
		if (sem_post(static_cast<sem_t *>(semHandle)) != 0)
		{
			throw GlobalSemaphoreException(
				StringConversion::syserr());
		}
	}
}

void GlobalSemaphore::wait()
{
	while(1)
	{
		int result = sem_wait(static_cast<sem_t *>(semHandle));
		if (result == -1)
		{
			if (errno == EINTR)
			{
				continue;
			}
			else if (errno == EAGAIN)
			{
				throw GlobalSemaphoreException(
					StringConversion::syserr());
			}
			else
			{
				throw GlobalSemaphoreException(
					StringConversion::syserr());
			}
		}
		else
		{
			break;
		}
	}
}

bool GlobalSemaphore::tryWait()
{
	bool result = false;
	while(1)
	{
		int result = sem_trywait(static_cast<sem_t *>(semHandle));
		if (result == -1)
		{
			if (errno == EINTR)
			{
				continue;
			}
			else if (errno == EAGAIN)
			{
				throw GlobalSemaphoreException(
					StringConversion::syserr());
			}
			else
			{
				throw GlobalSemaphoreException(
					StringConversion::syserr());
			}
		}
		else
		{
			result = true;
			break;
		}
	}

	return result;
}

std::wstring GlobalSemaphore::getRequestedName() const
{
	return givenName;
}

std::wstring GlobalSemaphore::getAssignedGlobalName() const
{
	return globalName;
}

bool GlobalSemaphore::willCleanup() const
{
	return openExclusive;
}

}


