//
// Created by Alexandre Lemos on 22/09/2019.
//

#ifndef PROJECT_HARDWARESTATS_H
#define PROJECT_HARDWARESTATS_H

#ifdef __APPLE__
#include <mach/vm_statistics.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#include <mach/task.h>

#endif

inline void printRAM() {
#ifdef __APPLE__
    vm_size_t page_size;
    mach_port_t mach_port;
    mach_msg_type_number_t count;
    vm_statistics64_data_t vm_stats;

    mach_port = mach_host_self();
    count = sizeof(vm_stats) / sizeof(natural_t);
    if (KERN_SUCCESS == host_page_size(mach_port, &page_size) &&
        KERN_SUCCESS == host_statistics64(mach_port, HOST_VM_INFO,
                                          (host_info64_t) &vm_stats, &count)) {
        long long free_memory = (int64_t) vm_stats.free_count * (int64_t) page_size;

        long long used_memory = ((int64_t) vm_stats.active_count +
                                 (int64_t) vm_stats.inactive_count +
                                 (int64_t) vm_stats.wire_count) * (int64_t) page_size;
        printf("free memory: %lldused memory: %lld", free_memory, used_memory);
    }
    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

    if (KERN_SUCCESS != task_info(mach_task_self(),
                                  TASK_BASIC_INFO, (task_info_t) &t_info,
                                  &t_info_count)) {
        std::exit(-1);
    }
    std::cout << "resident_size: " << t_info.resident_size << std::endl;
    std::cout << "virtual_size: " << t_info.virtual_size << std::endl;
#endif

}

#endif //PROJECT_HARDWARESTATS_H
