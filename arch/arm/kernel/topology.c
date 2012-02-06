/*
 * arch/arm/kernel/topology.c
 *
 * Copyright (C) 2011 Linaro Limited.
 * Written by: Vincent Guittot
 *
 * based on arch/sh/kernel/topology.c
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#include <linux/cpu.h>
#include <linux/cpumask.h>
#include <linux/init.h>
#include <linux/percpu.h>
#include <linux/node.h>
#include <linux/nodemask.h>
#include <linux/sched.h>
#include <linux/cpumask.h>
#include <linux/cpuset.h>
#include <linux/notifier.h>

#include <asm/cputype.h>
#include <asm/topology.h>

#define ARM_FAMILY_MASK 0xFF0FFFF0

#define MPIDR_SMP_BITMASK (0x3 << 30)
#define MPIDR_SMP_VALUE (0x2 << 30)

#define MPIDR_MT_BITMASK (0x1 << 24)

/*
 * These masks reflect the current use of the affinity levels.
 * The affinity level can be up to 16 bits according to ARM ARM
 */

#define MPIDR_LEVEL0_MASK 0x3
#define MPIDR_LEVEL0_SHIFT 0

#define MPIDR_LEVEL1_MASK 0xF
#define MPIDR_LEVEL1_SHIFT 8

#define MPIDR_LEVEL2_MASK 0xFF
#define MPIDR_LEVEL2_SHIFT 16

struct cputopo_arm cpu_topology[NR_CPUS];

/*
 * cpu topology mask update management
 */

static unsigned int prev_sched_mc_power_savings = 0;
static unsigned int prev_sched_smt_power_savings = 0;

/*
 * default topology function
 */
const struct cpumask *cpu_coregroup_mask(int cpu)
{
	return &cpu_topology[cpu].core_sibling;
}

/*
 * clear cpu topology masks
 */
static void clear_cpu_topology_mask(void)
{
	unsigned int cpuid;
	for_each_possible_cpu(cpuid) {
		struct cputopo_arm *cpuid_topo = &(cpu_topology[cpuid]);
		cpumask_clear(&cpuid_topo->core_sibling);
		cpumask_clear(&cpuid_topo->thread_sibling);
	}
	smp_wmb();
}

/*
 * default_cpu_topology_mask set the core and thread mask as described in the
 * ARM ARM
 */
static void default_cpu_topology_mask(unsigned int cpuid)
{
	struct cputopo_arm *cpuid_topo = &cpu_topology[cpuid];
	unsigned int cpu;

	for_each_possible_cpu(cpu) {
		struct cputopo_arm *cpu_topo = &cpu_topology[cpu];

		if (cpuid_topo->socket_id == cpu_topo->socket_id) {
			cpumask_set_cpu(cpuid, &cpu_topo->core_sibling);
			if (cpu != cpuid)
				cpumask_set_cpu(cpu,
					&cpuid_topo->core_sibling);

			if (cpuid_topo->core_id == cpu_topo->core_id) {
				cpumask_set_cpu(cpuid,
					&cpu_topo->thread_sibling);
				if (cpu != cpuid)
					cpumask_set_cpu(cpu,
						&cpuid_topo->thread_sibling);
			}
		}
	}
	smp_wmb();
}

static void normal_cpu_topology_mask(void)
{
	unsigned int cpuid;

	for_each_possible_cpu(cpuid) {
		default_cpu_topology_mask(cpuid);
	}
	smp_wmb();
}

/*
 * For Cortex-A9 MPcore, we emulate a multi-package topology in power mode.
 * The goal is to gathers tasks on 1 virtual package
 */
static void power_cpu_topology_mask_CA9(unsigned int cpuid)
{
	struct cputopo_arm *cpuid_topo = &cpu_topology[cpuid];
	unsigned int cpu;

	for_each_possible_cpu(cpu) {
		struct cputopo_arm *cpu_topo = &cpu_topology[cpu];

		if ((cpuid_topo->socket_id == cpu_topo->socket_id)
		&& ((cpuid & 0x1) == (cpu & 0x1))) {
			cpumask_set_cpu(cpuid, &cpu_topo->core_sibling);
			if (cpu != cpuid)
				cpumask_set_cpu(cpu,
					&cpuid_topo->core_sibling);

			if (cpuid_topo->core_id == cpu_topo->core_id) {
				cpumask_set_cpu(cpuid,
					&cpu_topo->thread_sibling);
				if (cpu != cpuid)
					cpumask_set_cpu(cpu,
						&cpuid_topo->thread_sibling);
			}
		}
	}
	smp_wmb();
}

static int need_topology_update(void)
{
	int update;

	update = ((prev_sched_mc_power_savings ^ sched_mc_power_savings)
	       || (prev_sched_smt_power_savings ^ sched_smt_power_savings));

	prev_sched_mc_power_savings = sched_mc_power_savings;
	prev_sched_smt_power_savings = sched_smt_power_savings;

	return update;
}

#define ARM_CORTEX_A9_FAMILY 0x410FC090

/* update_cpu_topology_policy select a cpu topology policy according to the
 * available cores.
 * TODO: The current version assumes that all cores are exactly the same which
 * might not be true. We need to update it to take into account various
 * configuration among which system with different kind of core.
 */
static int update_cpu_topology_mask(void)
{
	unsigned long cpuid;

	if (sched_mc_power_savings == POWERSAVINGS_BALANCE_NONE) {
		normal_cpu_topology_mask();
		return 0;
	}

	for_each_possible_cpu(cpuid) {
		struct cputopo_arm *cpuid_topo = &(cpu_topology[cpuid]);

		switch (cpuid_topo->id) {
		case ARM_CORTEX_A9_FAMILY:
			power_cpu_topology_mask_CA9(cpuid);
		break;
		default:
			default_cpu_topology_mask(cpuid);
		break;
		}
	}

	return 0;
}

/*
 * store_cpu_topology is called at boot when only one cpu is running
 * and with the mutex cpu_hotplug.lock locked, when several cpus have booted,
 * which prevents simultaneous write access to cpu_topology array
 */
void store_cpu_topology(unsigned int cpuid)
{
	struct cputopo_arm *cpuid_topo = &cpu_topology[cpuid];
	unsigned int mpidr;

	/* If the cpu topology has been already set, just return */
	if (cpuid_topo->core_id != -1)
		return;

	mpidr = read_cpuid_mpidr();

	/* create cpu topology mapping */
	if ((mpidr & MPIDR_SMP_BITMASK) == MPIDR_SMP_VALUE) {
		/*
		 * This is a multiprocessor system
		 * multiprocessor format & multiprocessor mode field are set
		 */

		if (mpidr & MPIDR_MT_BITMASK) {
			/* core performance interdependency */
			cpuid_topo->thread_id = (mpidr >> MPIDR_LEVEL0_SHIFT)
				& MPIDR_LEVEL0_MASK;
			cpuid_topo->core_id = (mpidr >> MPIDR_LEVEL1_SHIFT)
				& MPIDR_LEVEL1_MASK;
			cpuid_topo->socket_id = (mpidr >> MPIDR_LEVEL2_SHIFT)
				& MPIDR_LEVEL2_MASK;
		} else {
			/* largely independent cores */
			cpuid_topo->thread_id = -1;
			cpuid_topo->core_id = (mpidr >> MPIDR_LEVEL0_SHIFT)
				& MPIDR_LEVEL0_MASK;
			cpuid_topo->socket_id = (mpidr >> MPIDR_LEVEL1_SHIFT)
				& MPIDR_LEVEL1_MASK;
		}

		cpuid_topo->id = read_cpuid_id() & ARM_FAMILY_MASK;

	} else {
		/*
		 * This is an uniprocessor system
		 * we are in multiprocessor format but uniprocessor system
		 * or in the old uniprocessor format
		 */
		cpuid_topo->thread_id = -1;
		cpuid_topo->core_id = 0;
		cpuid_topo->socket_id = -1;
	}

	/*
	 * The core and thread sibling masks can also be updated during the
	 * call of arch_update_cpu_topology
	 */
	default_cpu_topology_mask(cpuid);


	printk(KERN_INFO "CPU%u: thread %d, cpu %d, socket %d, mpidr %x\n",
		cpuid, cpu_topology[cpuid].thread_id,
		cpu_topology[cpuid].core_id,
		cpu_topology[cpuid].socket_id, mpidr);
}

/*
 * arch_update_cpu_topology is called by the scheduler before building
 * a new sched_domain hierarchy.
 */
int arch_update_cpu_topology(void)
{
	if (!need_topology_update())
		return 0;

	/* clear core threads mask */
	clear_cpu_topology_mask();

	/* set topology mask */
	update_cpu_topology_mask();

	return 1;
}

/*
 * init_cpu_topology is called at boot when only one cpu is running
 * which prevent simultaneous write access to cpu_topology array
 */
void init_cpu_topology(void)
{
	unsigned int cpu;

	/* init core mask */
	for_each_possible_cpu(cpu) {
		struct cputopo_arm *cpu_topo = &(cpu_topology[cpu]);

		cpu_topo->id = -1;
		cpu_topo->thread_id = -1;
		cpu_topo->core_id =  -1;
		cpu_topo->socket_id = -1;
		cpumask_clear(&cpu_topo->core_sibling);
		cpumask_clear(&cpu_topo->thread_sibling);
	}
	smp_wmb();
}
