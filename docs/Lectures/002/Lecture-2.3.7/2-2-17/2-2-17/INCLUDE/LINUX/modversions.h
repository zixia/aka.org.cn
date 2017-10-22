/*
 * Try to be a little smarter about which kernel are we currently running
 */

#ifndef __rh_kernel_modversions_h__
#define __rh_kernel_modversions_h__

/*
 * First, get the version string for the running kernel from
 * /boot/kernel.h - initscripts should create it for us
 */

#include "/boot/kernel.h"

/*
 * Define some nasty macros o we can construct the file names
 * we want to include
 */

#if defined(__rh_modversion_included_file__)
#undef __rh_modversion_included_file__
#endif /* __rh_modversion_included_file__ */

#if defined(__BOOT_KERNEL_UP) && (__BOOT_KERNEL_UP == 1)
#include <linux/modversions-up.h>
#define __rh_modversion_included_file__ 1
#endif /* __BOOT_KERNEL_UP */

#if defined(__BOOT_KERNEL_SMP) && (__BOOT_KERNEL_SMP == 1)
#include <linux/modversions-smp.h>
#define __rh_modversion_included_file__ 1
#endif /* __BOOT_KERNEL_SMP */

#if defined(__BOOT_KERNEL_BOOT) && (__BOOT_KERNEL_BOOT == 1)
#include <linux/modversions-BOOT.h>
#define __rh_modversion_included_file__ 1
#endif /* __BOOT_KERNEL_BOOT */

#if defined(__BOOT_KERNEL_BOOTSMP) && (__BOOT_KERNEL_BOOTSMP == 1)
#include <linux/modversions-BOOTsmp.h>
#define __rh_modversion_included_file__ 1
#endif /* __BOOT_KERNEL_BOOTSMP */

#if !defined(__rh_modversion_included_file__)
#include <linux/modversions-up.h>
#else
#undef __rh_modversion_included_file__
#endif /* __rh_modversion_included_file__ */

#endif /* __rh_kernel_autoconf_h__ */
