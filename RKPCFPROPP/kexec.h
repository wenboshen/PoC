#ifndef __KEXEC_H__
#define __KEXEC_H__

//Execute a kernel function with no arguments
#define KEXEC0(function) \
	execute_in_kernel(function, 0, 0, 0, 0, 0, 0, 0)

//Execute a kernel function with a single argument
#define KEXEC1(function, arg0) \
	execute_in_kernel(function, arg0, 0, 0, 0, 0, 0, 0)

//Execute a kernel function with two arguments
#define KEXEC2(function, arg0, arg1) \
	execute_in_kernel(function, arg0, arg1, 0, 0, 0, 0, 0)

//Execute a kernel function with three arguments
#define KEXEC3(function, arg0, arg1, arg2) \
	execute_in_kernel(function, arg0, arg1, arg2, 0, 0, 0, 0)

//Execute a kernel function with four arguments
#define KEXEC4(function, arg0, arg1, arg2, arg3) \
	execute_in_kernel(function, arg0, arg1, arg2, arg3, 0, 0, 0)

//Execute a kernel function with five arguments
#define KEXEC5(function, arg0, arg1, arg2, arg3, arg4) \
	execute_in_kernel(function, arg0, arg1, arg2, arg3, arg4, 0, 0)

//Execute a kernel function with six arguments
#define KEXEC6(function, arg0, arg1, arg2, arg3, arg4, arg5) \
	execute_in_kernel(function, arg0, arg1, arg2, arg3, arg4, arg5, 0)

//Execute a kernel function with seven arguments
#define KEXEC7(function, arg0, arg1, arg2, arg3, arg4, arg5, arg6) \
	execute_in_kernel(function, arg0, arg1, arg2, arg3, arg4, arg5, arg6)

#endif
