
#include <iostream>
#include <fstream>
#include <string>

#include <sys/sysinfo.h>

#ifndef __APPLE__
#include <sched.h>
#endif

#include <aux/utils.hpp>

namespace aux
{


//===============================================================================
void set_cpu_affinity(size_t cpu_number)
{
#ifndef __APPLE__
	//Assign each thread to particular processor core
	cpu_set_t affinity_mask;
	CPU_ZERO(&affinity_mask);
	CPU_SET(cpu_number, &affinity_mask);
	sched_setaffinity(0, sizeof(affinity_mask), &affinity_mask);
#endif
}

//===============================================================================
double system_memory_usage_ratio()
{
	std::ifstream meminfo_file("/proc/meminfo");

	size_t total_mem = 0;
	size_t avail_mem = 0;

	std::string tag;
	size_t val;
	std::string units;
	bool total_mem_found = false;
	bool avail_mem_found = false;
	while ((!total_mem_found || !avail_mem_found) && meminfo_file >> tag >> val >> units)
	{
		if(tag.find("MemTotal") != std::string::npos)
		{
			total_mem = val;
			total_mem_found = true;
		}
		else if (tag.find("MemAvailable") != std::string::npos)
		{
			avail_mem = val;
			avail_mem_found = true;
		}
	}

	return (double)(total_mem - avail_mem) / (double)total_mem;
}



}

