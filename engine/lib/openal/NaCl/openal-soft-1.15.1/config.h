/* API declaration export attribute */
//#define AL_API  __declspec(dllexport)
//#define ALC_API __declspec(dllexport)
#define AL_API
#define ALC_API

/* Define to the library version */
#define ALSOFT_VERSION "1.15.1"

/* Define any available alignment declaration */
#define ALIGN(x) __attribute__((aligned(x)))
#ifdef __MINGW32__
#define align(x) aligned(x)
#endif

#define RESTRICT
#define HAVE_PPAPI
#define SIZEOF_LONG 4
#define SIZEOF_LONG_LONG 8
#define HAVE_STDINT_H

#define AL_LIBTYPE_STATIC

#define HAVE_GCC_DESTRUCTOR

#if 0
/* Define to the appropriate 'restrict' keyword */
#define RESTRICT ${RESTRICT_DECL}

/* Define if we have the C11 aligned_alloc function */
#cmakedefine HAVE_ALIGNED_ALLOC

/* Define if we have the posix_memalign function */
#cmakedefine HAVE_POSIX_MEMALIGN

/* Define if we have the _aligned_malloc function */
#cmakedefine HAVE__ALIGNED_MALLOC

/* Define if we have SSE CPU extensions */
#cmakedefine HAVE_SSE

/* Define if we have ARM Neon CPU extensions */
#cmakedefine HAVE_NEON

/* Define if we have the ALSA backend */
#cmakedefine HAVE_ALSA

/* Define if we have the OSS backend */
#cmakedefine HAVE_OSS

/* Define if we have the Solaris backend */
#cmakedefine HAVE_SOLARIS

/* Define if we have the SndIO backend */
#cmakedefine HAVE_SNDIO

/* Define if we have the MMDevApi backend */
#cmakedefine HAVE_MMDEVAPI

/* Define if we have the DSound backend */
#cmakedefine HAVE_DSOUND

/* Define if we have the Windows Multimedia backend */
#cmakedefine HAVE_WINMM

/* Define if we have the PortAudio backend */
#cmakedefine HAVE_PORTAUDIO

/* Define if we have the PulseAudio backend */
#cmakedefine HAVE_PULSEAUDIO

/* Define if we have the PPAPI backend */
#cmakedefine HAVE_PPAPI

/* Define if we have the CoreAudio backend */
#cmakedefine HAVE_COREAUDIO

/* Define if we have the OpenSL backend */
#cmakedefine HAVE_OPENSL

/* Define if we have the Wave Writer backend */
#cmakedefine HAVE_WAVE

/* Define if we have the stat function */
#cmakedefine HAVE_STAT

/* Define if we have the lrintf function */
#cmakedefine HAVE_LRINTF

/* Define if we have the strtof function */
#cmakedefine HAVE_STRTOF

/* Define if we have the __int64 type */
#cmakedefine HAVE___INT64

/* Define to the size of a long int type */
#cmakedefine SIZEOF_LONG ${SIZEOF_LONG}

/* Define to the size of a long long int type */
#cmakedefine SIZEOF_LONG_LONG ${SIZEOF_LONG_LONG}

/* Define if we have GCC's destructor attribute */
#cmakedefine HAVE_GCC_DESTRUCTOR

/* Define if we have GCC's format attribute */
#cmakedefine HAVE_GCC_FORMAT

/* Define if we have stdint.h */
#cmakedefine HAVE_STDINT_H

/* Define if we have windows.h */
#cmakedefine HAVE_WINDOWS_H

/* Define if we have dlfcn.h */
#cmakedefine HAVE_DLFCN_H

/* Define if we have pthread_np.h */
#cmakedefine HAVE_PTHREAD_NP_H

/* Define if we have xmmintrin.h */
#cmakedefine HAVE_XMMINTRIN_H

/* Define if we have arm_neon.h */
#cmakedefine HAVE_ARM_NEON_H

/* Define if we have malloc.h */
#cmakedefine HAVE_MALLOC_H

/* Define if we have cpuid.h */
#cmakedefine HAVE_CPUID_H

/* Define if we have guiddef.h */
#cmakedefine HAVE_GUIDDEF_H

/* Define if we have initguid.h */
#cmakedefine HAVE_INITGUID_H

/* Define if we have ieeefp.h */
#cmakedefine HAVE_IEEEFP_H

/* Define if we have float.h */
#cmakedefine HAVE_FLOAT_H

/* Define if we have fenv.h */
#cmakedefine HAVE_FENV_H

/* Define if we have fesetround() */
#cmakedefine HAVE_FESETROUND

/* Define if we have _controlfp() */
#cmakedefine HAVE__CONTROLFP

/* Define if we have __control87_2() */
#cmakedefine HAVE___CONTROL87_2

/* Define if we have pthread_setschedparam() */
#cmakedefine HAVE_PTHREAD_SETSCHEDPARAM
#endif
