/* stb.h - v2.25 - Sean's Tool Box -- public domain -- http://nothings.org/stb.h
          no warranty is offered or implied; use this code at your own risk

   This is a single header file with a bunch of useful utilities
   for getting stuff done in C/C++.

   Documentation: http://nothings.org/stb/stb_h.html
   Unit tests:    http://nothings.org/stb/stb.c


 ============================================================================
   You MUST                                                                  
                                                                             
      #define STB_DEFINE                                                     
                                                                             
   in EXACTLY _one_ C or C++ file that includes this header, BEFORE the
   include, like this:                                                                
                                                                             
      #define STB_DEFINE                                                     
      #include "stb.h"
      
   All other files should just #include "stb.h" without the #define.
 ============================================================================


Version History

   2.25   various warning & bugfixes
   2.24   various warning & bugfixes
   2.23   fix 2.22
   2.22   64-bit fixes from '!='; fix stb_sdict_copy() to have preferred name
   2.21   utf-8 decoder rejects "overlong" encodings; attempted 64-bit improvements
   2.20   fix to hash "copy" function--reported by someone with handle "!="
   2.19   ???
   2.18   stb_readdir_subdirs_mask
   2.17   stb_cfg_dir
   2.16   fix stb_bgio_, add stb_bgio_stat(); begin a streaming wrapper
   2.15   upgraded hash table template to allow:
            - aggregate keys (explicit comparison func for EMPTY and DEL keys)
            - "static" implementations (so they can be culled if unused)
   2.14   stb_mprintf
   2.13   reduce identifiable strings in STB_NO_STB_STRINGS
   2.12   fix STB_ONLY -- lots of uint32s, TRUE/FALSE things had crept in
   2.11   fix bug in stb_dirtree_get() which caused "c://path" sorts of stuff
   2.10   STB_F(), STB_I() inline constants (also KI,KU,KF,KD)
   2.09   stb_box_face_vertex_axis_side
   2.08   bugfix stb_trimwhite()
   2.07   colored printing in windows (why are we in 1985?)
   2.06   comparison functions are now functions-that-return-functions and
          accept a struct-offset as a parameter (not thread-safe)
   2.05   compile and pass tests under Linux (but no threads); thread cleanup
   2.04   stb_cubic_bezier_1d, smoothstep, avoid dependency on registry
   2.03   ?
   2.02   remove integrated documentation
   2.01   integrate various fixes; stb_force_uniprocessor
   2.00   revised stb_dupe to use multiple hashes
   1.99   stb_charcmp
   1.98   stb_arr_deleten, stb_arr_insertn
   1.97   fix stb_newell_normal()
   1.96   stb_hash_number()
   1.95   hack stb__rec_max; clean up recursion code to use new functions
   1.94   stb_dirtree; rename stb_extra to stb_ptrmap
   1.93   stb_sem_new() API cleanup (no blockflag-starts blocked; use 'extra')
   1.92   stb_threadqueue--multi reader/writer queue, fixed size or resizeable
   1.91   stb_bgio_* for reading disk asynchronously
   1.90   stb_mutex uses CRITICAL_REGION; new stb_sync primitive for thread
          joining; workqueue supports stb_sync instead of stb_semaphore
   1.89   support ';' in constant-string wildcards; stb_mutex wrapper (can
          implement with EnterCriticalRegion eventually)
   1.88   portable threading API (only for win32 so far); worker thread queue
   1.87   fix wildcard handling in stb_readdir_recursive
   1.86   support ';' in wildcards
   1.85   make stb_regex work with non-constant strings;
               beginnings of stb_introspect()
   1.84   (forgot to make notes)
   1.83   whoops, stb_keep_if_different wasn't deleting the temp file
   1.82   bring back stb_compress from stb_file.h for cmirror
   1.81   various bugfixes, STB_FASTMALLOC_INIT inits FASTMALLOC in release
   1.80   stb_readdir returns utf8; write own utf8-utf16 because lib was wrong
   1.79   stb_write
   1.78   calloc() support for malloc wrapper, STB_FASTMALLOC
   1.77   STB_FASTMALLOC
   1.76   STB_STUA - Lua-like language; (stb_image, stb_csample, stb_bilinear)
   1.75   alloc/free array of blocks; stb_hheap bug; a few stb_ps_ funcs;
          hash*getkey, hash*copy; stb_bitset; stb_strnicmp; bugfix stb_bst
   1.74   stb_replaceinplace; use stdlib C function to convert utf8 to UTF-16
   1.73   fix performance bug & leak in stb_ischar (C++ port lost a 'static')
   1.72   remove stb_block, stb_block_manager, stb_decompress (to stb_file.h)
   1.71   stb_trimwhite, stb_tokens_nested, etc.
   1.70   back out 1.69 because it might problemize mixed builds; stb_filec()
   1.69   (stb_file returns 'char *' in C++)
   1.68   add a special 'tree root' data type for stb_bst; stb_arr_end
   1.67   full C++ port. (stb_block_manager)
   1.66   stb_newell_normal
   1.65   stb_lex_item_wild -- allow wildcard items which MUST match entirely
   1.64   stb_data
   1.63   stb_log_name
   1.62   stb_define_sort; C++ cleanup
   1.61   stb_hash_fast -- Paul Hsieh's hash function (beats Bob Jenkins'?)
   1.60   stb_delete_directory_recursive
   1.59   stb_readdir_recursive
   1.58   stb_bst variant with parent pointer for O(1) iteration, not O(log N)
   1.57   replace LCG random with Mersenne Twister (found a public domain one)
   1.56   stb_perfect_hash, stb_ischar, stb_regex
   1.55   new stb_bst API allows multiple BSTs per node (e.g. secondary keys)
   1.54   bugfix: stb_define_hash, stb_wildmatch, regexp
   1.53   stb_define_hash; recoded stb_extra, stb_sdict use it
   1.52   stb_rand_define, stb_bst, stb_reverse
   1.51   fix 'stb_arr_setlen(NULL, 0)'
   1.50   stb_wordwrap
   1.49   minor improvements to enable the scripting language
   1.48   better approach for stb_arr using stb_malloc; more invasive, clearer
   1.47   stb_lex (lexes stb.h at 1.5ML/s on 3Ghz P4; 60/70% of optimal/flex)
   1.46   stb_wrapper_*, STB_MALLOC_WRAPPER
   1.45   lightly tested DFA acceleration of regexp searching
   1.44   wildcard matching & searching; regexp matching & searching
   1.43   stb_temp
   1.42   allow stb_arr to use stb_malloc/realloc; note this is global
   1.41   make it compile in C++; (disable stb_arr in C++)
   1.40   stb_dupe tweak; stb_swap; stb_substr
   1.39   stb_dupe; improve stb_file_max to be less stupid
   1.38   stb_sha1_file: generate sha1 for file, even > 4GB
   1.37   stb_file_max; partial support for utf8 filenames in Windows
   1.36   remove STB__NO_PREFIX - poor interaction with IDE, not worth it
          streamline stb_arr to make it separately publishable
   1.35   bugfixes for stb_sdict, stb_malloc(0), stristr
   1.34   (streaming interfaces for stb_compress)
   1.33   stb_alloc; bug in stb_getopt; remove stb_overflow
   1.32   (stb_compress returns, smaller&faster; encode window & 64-bit len)
   1.31   stb_prefix_count
   1.30   (STB__NO_PREFIX - remove stb_ prefixes for personal projects)
   1.29   stb_fput_varlen64, etc.
   1.28   stb_sha1
   1.27   ?
   1.26   stb_extra
   1.25   ?
   1.24   stb_copyfile
   1.23   stb_readdir
   1.22   ?
   1.21   ?
   1.20   ?
   1.19   ?
   1.18   ?
   1.17   ?
   1.16   ?
   1.15   stb_fixpath, stb_splitpath, stb_strchr2
   1.14   stb_arr
   1.13   ?stb, stb_log, stb_fatal
   1.12   ?stb_hash2
   1.11   miniML
   1.10   stb_crc32, stb_adler32
   1.09   stb_sdict
   1.08   stb_bitreverse, stb_ispow2, stb_big32
          stb_fopen, stb_fput_varlen, stb_fput_ranged
          stb_fcmp, stb_feq
   1.07   (stb_encompress)
   1.06   stb_compress
   1.05   stb_tokens, (stb_hheap)
   1.04   stb_rand
   1.03   ?(s-strings)
   1.02   ?stb_filelen, stb_tokens
   1.01   stb_tolower
   1.00   stb_hash, stb_intcmp
          stb_file, stb_stringfile, stb_fgets
          stb_prefix, stb_strlower, stb_strtok
          stb_image
          (stb_array), (stb_arena)

Parenthesized items have since been removed.

LICENSE

This software is in the public domain. Where that dedication is not
recognized, you are granted a perpetual, irrevocable license to copy,
distribute, and modify this file as you see fit.

CREDITS

 Written by Sean Barrett.

 Fixes:
  Philipp Wiesemann
  Robert Nix
  r-lyeh
  blackpawn
  Mojofreem@github
  Ryan Whitworth
  Vincent Isambart
*/

#ifndef STB__INCLUDE_STB_H
#define STB__INCLUDE_STB_H

#define STB_VERSION  1

#ifdef STB_INTROSPECT
   #define STB_DEFINE
#endif

#ifdef STB_DEFINE_THREADS
   #ifndef STB_DEFINE
   #define STB_DEFINE
   #endif
   #ifndef STB_THREADS
   #define STB_THREADS
   #endif
#endif

#ifdef _WIN32
   #define _CRT_SECURE_NO_WARNINGS
   #define _CRT_NONSTDC_NO_DEPRECATE
   #define _CRT_NON_CONFORMING_SWPRINTFS
   #if !defined(_MSC_VER) || _MSC_VER > 1700
   #include <intrin.h> // _BitScanReverse
   #endif
#endif

#include <stdlib.h>     // stdlib could have min/max
#include <stdio.h>      // need FILE
#include <string.h>     // stb_define_hash needs memcpy/memset
#include <time.h>       // stb_dirtree
#ifdef __MINGW32__
   #include <fcntl.h>   // O_RDWR
#endif

#ifdef STB_PERSONAL
   typedef int Bool;
   #define False 0
   #define True 1
#endif

#ifdef STB_MALLOC_WRAPPER_PAGED
   #define STB_MALLOC_WRAPPER_DEBUG
#endif
#ifdef STB_MALLOC_WRAPPER_DEBUG
   #define STB_MALLOC_WRAPPER
#endif
#ifdef STB_MALLOC_WRAPPER_FASTMALLOC
   #define STB_FASTMALLOC
   #define STB_MALLOC_WRAPPER
#endif

#ifdef STB_FASTMALLOC
   #ifndef _WIN32
      #undef STB_FASTMALLOC
   #endif
#endif

#ifdef STB_DEFINE
   #include <assert.h>
   #include <stdarg.h>
   #include <stddef.h>
   #include <ctype.h>
   #include <math.h>
   #ifndef _WIN32
   #include <unistd.h>
   #else
   #include <io.h>      // _mktemp
   #include <direct.h>  // _rmdir
   #endif
   #include <sys/types.h> // stat()/_stat()
   #include <sys/stat.h>  // stat()/_stat()
#endif

#define stb_min(a,b)   ((a) < (b) ? (a) : (b))
#define stb_max(a,b)   ((a) > (b) ? (a) : (b))

#ifndef STB_ONLY
   #if !defined(__cplusplus) && !defined(min) && !defined(max)
     #define min(x,y) stb_min(x,y)
     #define max(x,y) stb_max(x,y)
   #endif

   #ifndef M_PI
     #define M_PI  3.14159265358979323846f
   #endif

   #ifndef TRUE
     #define TRUE  1
     #define FALSE 0
   #endif

   #ifndef deg2rad
   #define deg2rad(a)  ((a)*(M_PI/180))
   #endif
   #ifndef rad2deg
   #define rad2deg(a)  ((a)*(180/M_PI))
   #endif
   
   #ifndef swap
   #ifndef __cplusplus
   #define swap(TYPE,a,b)  \
               do { TYPE stb__t; stb__t = (a); (a) = (b); (b) = stb__t; } while (0)
   #endif              
   #endif
    
   typedef unsigned char  uint8 ;
   typedef   signed char   int8 ;
   typedef unsigned short uint16;
   typedef   signed short  int16;
  #if defined(STB_USE_LONG_FOR_32_BIT_INT) || defined(STB_LONG32)
   typedef unsigned long  uint32;
   typedef   signed long   int32;
  #else
   typedef unsigned int   uint32;
   typedef   signed int    int32;
  #endif

   typedef unsigned char  uchar ;
   typedef unsigned short ushort;
   typedef unsigned int   uint  ;
   typedef unsigned long  ulong ;

   // produce compile errors if the sizes aren't right
   typedef char stb__testsize16[sizeof(int16)==2];
   typedef char stb__testsize32[sizeof(int32)==4];
#endif

#ifndef STB_TRUE
  #define STB_TRUE 1
  #define STB_FALSE 0
#endif

// if we're STB_ONLY, can't rely on uint32 or even uint, so all the
// variables we'll use herein need typenames prefixed with 'stb':
typedef unsigned char stb_uchar;
typedef unsigned char stb_uint8;
typedef unsigned int  stb_uint;
typedef unsigned short stb_uint16;
typedef          short stb_int16;
typedef   signed char  stb_int8;
#if defined(STB_USE_LONG_FOR_32_BIT_INT) || defined(STB_LONG32)
  typedef unsigned long  stb_uint32;
  typedef          long  stb_int32;
#else
  typedef unsigned int   stb_uint32;
  typedef          int   stb_int32;
#endif
typedef char stb__testsize2_16[sizeof(stb_uint16)==2 ? 1 : -1];
typedef char stb__testsize2_32[sizeof(stb_uint32)==4 ? 1 : -1];

#ifdef _MSC_VER
  typedef unsigned __int64 stb_uint64;
  typedef          __int64 stb_int64;
  #define STB_IMM_UINT64(literalui64) (literalui64##ui64)
  #define STB_IMM_INT64(literali64) (literali64##i64)
#else
  // ??
  typedef unsigned long long stb_uint64;
  typedef          long long stb_int64;
  #define STB_IMM_UINT64(literalui64) (literalui64##ULL)
  #define STB_IMM_INT64(literali64) (literali64##LL)
#endif
typedef char stb__testsize2_64[sizeof(stb_uint64)==8 ? 1 : -1];

// add platform-specific ways of checking for sizeof(char*) == 8,
// and make those define STB_PTR64
#if defined(_WIN64) || defined(__x86_64__) || defined(__ia64__) || defined(__LP64__)
  #define STB_PTR64
#endif

#ifdef STB_PTR64
typedef char stb__testsize2_ptr[sizeof(char *) == 8];
typedef stb_uint64 stb_uinta;
typedef stb_int64  stb_inta;
#else
typedef char stb__testsize2_ptr[sizeof(char *) == 4];
typedef stb_uint32 stb_uinta;
typedef stb_int32  stb_inta;
#endif
typedef char stb__testsize2_uinta[sizeof(stb_uinta)==sizeof(char*) ? 1 : -1];

// if so, we should define an int type that is the pointer size. until then,
// we'll have to make do with this (which is not the same at all!)

typedef union
{
   unsigned int i;
   void * p;
} stb_uintptr;


#ifdef __cplusplus
   #define STB_EXTERN   extern "C"
#else
   #define STB_EXTERN   extern
#endif

// check for well-known debug defines
#if defined(DEBUG) || defined(_DEBUG) || defined(DBG)
   #ifndef NDEBUG
      #define STB_DEBUG
   #endif
#endif

#ifdef STB_DEBUG
   #include <assert.h>
#endif


STB_EXTERN void stb_wrapper_malloc(void *newp, int sz, char *file, int line);
STB_EXTERN void stb_wrapper_free(void *oldp, char *file, int line);
STB_EXTERN void stb_wrapper_realloc(void *oldp, void *newp, int sz, char *file, int line);
STB_EXTERN void stb_wrapper_calloc(size_t num, size_t sz, char *file, int line);
STB_EXTERN void stb_wrapper_listall(void (*func)(void *ptr, int sz, char *file, int line));
STB_EXTERN void stb_wrapper_dump(char *filename);
STB_EXTERN int stb_wrapper_allocsize(void *oldp);
STB_EXTERN void stb_wrapper_check(void *oldp);

#ifdef STB_DEFINE
// this is a special function used inside malloc wrapper
// to do allocations that aren't tracked (to avoid
// reentrancy). Of course if someone _else_ wraps realloc,
// this breaks, but if they're doing that AND the malloc
// wrapper they need to explicitly check for reentrancy.
//
// only define realloc_raw() and we do realloc(NULL,sz)
// for malloc() and realloc(p,0) for free().
static void * stb__realloc_raw(void *p, int sz)
{
   if (p == NULL) return malloc(sz);
   if (sz == 0)   { free(p); return NULL; }
   return realloc(p,sz);
}
#endif

#ifdef _WIN32
STB_EXTERN void * stb_smalloc(size_t sz);
STB_EXTERN void   stb_sfree(void *p);
STB_EXTERN void * stb_srealloc(void *p, size_t sz);
STB_EXTERN void * stb_scalloc(size_t n, size_t sz);
STB_EXTERN char * stb_sstrdup(char *s);
#endif

#ifdef STB_FASTMALLOC
#define malloc  stb_smalloc
#define free    stb_sfree
#define realloc stb_srealloc
#define strdup  stb_sstrdup
#define calloc  stb_scalloc
#endif

#ifndef STB_MALLOC_ALLCHECK
   #define stb__check(p)  1
#else
   #ifndef STB_MALLOC_WRAPPER
      #error STB_MALLOC_ALLCHECK requires STB_MALLOC_WRAPPER
   #else
      #define stb__check(p) stb_mcheck(p)
   #endif
#endif

#ifdef STB_MALLOC_WRAPPER
   STB_EXTERN void * stb__malloc(int, char *, int);
   STB_EXTERN void * stb__realloc(void *, int, char *, int);
   STB_EXTERN void * stb__calloc(size_t n, size_t s, char *, int);
   STB_EXTERN void   stb__free(void *, char *file, int);
   STB_EXTERN char * stb__strdup(char *s, char *file, int);
   STB_EXTERN void   stb_malloc_checkall(void);
   STB_EXTERN void   stb_malloc_check_counter(int init_delay, int rep_delay);
   #ifndef STB_MALLOC_WRAPPER_DEBUG
      #define stb_mcheck(p) 1
   #else
      STB_EXTERN int   stb_mcheck(void *);
   #endif


   #ifdef STB_DEFINE

   #ifdef STB_MALLOC_WRAPPER_DEBUG
      #define STB__PAD   32
      #define STB__BIAS  16
      #define STB__SIG   0x51b01234
      #define STB__FIXSIZE(sz)  (((sz+3) & ~3) + STB__PAD)
      #define STB__ptr(x,y)   ((char *) (x) + (y))
   #else
      #define STB__ptr(x,y)   (x)
      #define STB__FIXSIZE(sz)  (sz)
   #endif

   #ifdef STB_MALLOC_WRAPPER_DEBUG
   int stb_mcheck(void *p)
   {
      unsigned int sz;
      if (p == NULL) return 1;
      p = ((char *) p) - STB__BIAS;
      sz = * (unsigned int *) p;
      assert(* (unsigned int *) STB__ptr(p,4) == STB__SIG);
      assert(* (unsigned int *) STB__ptr(p,8) == STB__SIG);
      assert(* (unsigned int *) STB__ptr(p,12) == STB__SIG);
      assert(* (unsigned int *) STB__ptr(p,sz-4) == STB__SIG+1);
      assert(* (unsigned int *) STB__ptr(p,sz-8) == STB__SIG+1);
      assert(* (unsigned int *) STB__ptr(p,sz-12) == STB__SIG+1);
      assert(* (unsigned int *) STB__ptr(p,sz-16) == STB__SIG+1);
      stb_wrapper_check(STB__ptr(p, STB__BIAS));
      return 1;
   }

   static void stb__check2(void *p, int sz, char *file, int line)
   {
      stb_mcheck(p);
   }

   void stb_malloc_checkall(void)
   {
      stb_wrapper_listall(stb__check2);
   }
   #else
   void stb_malloc_checkall(void) { }
   #endif

   static int stb__malloc_wait=(1 << 30), stb__malloc_next_wait = (1 << 30), stb__malloc_iter;
   void stb_malloc_check_counter(int init_delay, int rep_delay)
   {
      stb__malloc_wait = init_delay;
      stb__malloc_next_wait = rep_delay;
   }

   void stb_mcheck_all(void)
   {
      #ifdef STB_MALLOC_WRAPPER_DEBUG
      ++stb__malloc_iter;
      if (--stb__malloc_wait <= 0) {
         stb_malloc_checkall();
         stb__malloc_wait = stb__malloc_next_wait;
      }
      #endif
   }

   #ifdef STB_MALLOC_WRAPPER_PAGED
   #define STB__WINDOWS_PAGE (1 << 12)
   #ifndef _WINDOWS_
   STB_EXTERN __declspec(dllimport) void * __stdcall VirtualAlloc(void *p, unsigned long size, unsigned long type, unsigned long protect);
   STB_EXTERN __declspec(dllimport) int   __stdcall VirtualFree(void *p, unsigned long size, unsigned long freetype);
   #endif
   #endif

   static void *stb__malloc_final(int sz)
   {
      #ifdef STB_MALLOC_WRAPPER_PAGED
      int aligned = (sz + STB__WINDOWS_PAGE - 1) & ~(STB__WINDOWS_PAGE-1);
      char *p = VirtualAlloc(NULL, aligned + STB__WINDOWS_PAGE, 0x2000, 0x04); // RESERVE, READWRITE
      if (p == NULL) return p;
      VirtualAlloc(p, aligned,   0x1000, 0x04); // COMMIT, READWRITE
      return p;
      #else
      return malloc(sz);
      #endif
   }

   static void stb__free_final(void *p)
   {
      #ifdef STB_MALLOC_WRAPPER_PAGED
      VirtualFree(p, 0, 0x8000); // RELEASE
      #else
      free(p);
      #endif
   }

   int stb__malloc_failure;
   static void *stb__realloc_final(void *p, int sz, int old_sz)
   {
      #ifdef STB_MALLOC_WRAPPER_PAGED
      void *q = stb__malloc_final(sz);
      if (q == NULL)
          return ++stb__malloc_failure, q;
      // @TODO: deal with p being smaller!
      memcpy(q, p, sz < old_sz ? sz : old_sz);
      stb__free_final(p);
      return q;
      #else
      return realloc(p,sz);
      #endif
   }

   void stb__free(void *p, char *file, int line)
   {
      stb_mcheck_all();
      if (!p) return;
      #ifdef STB_MALLOC_WRAPPER_DEBUG
      stb_mcheck(p);
      #endif
      stb_wrapper_free(p,file,line);
      #ifdef STB_MALLOC_WRAPPER_DEBUG
         p = STB__ptr(p,-STB__BIAS);
         * (unsigned int *) STB__ptr(p,0) = 0xdeadbeef;
         * (unsigned int *) STB__ptr(p,4) = 0xdeadbeef;
         * (unsigned int *) STB__ptr(p,8) = 0xdeadbeef;
         * (unsigned int *) STB__ptr(p,12) = 0xdeadbeef;
      #endif
      stb__free_final(p);
   }

   void * stb__malloc(int sz, char *file, int line)
   {
      void *p;
      stb_mcheck_all();
      if (sz == 0) return NULL;
      p = stb__malloc_final(STB__FIXSIZE(sz));
      if (p == NULL) p = stb__malloc_final(STB__FIXSIZE(sz));
      if (p == NULL) p = stb__malloc_final(STB__FIXSIZE(sz));
      if (p == NULL) {
         ++stb__malloc_failure;
         #ifdef STB_MALLOC_WRAPPER_DEBUG
         stb_malloc_checkall();
         #endif
         return p;
      }
      #ifdef STB_MALLOC_WRAPPER_DEBUG
      * (int *) STB__ptr(p,0) = STB__FIXSIZE(sz);
      * (unsigned int *) STB__ptr(p,4) = STB__SIG;
      * (unsigned int *) STB__ptr(p,8) = STB__SIG;
      * (unsigned int *) STB__ptr(p,12) = STB__SIG;
      * (unsigned int *) STB__ptr(p,STB__FIXSIZE(sz)-4) = STB__SIG+1;
      * (unsigned int *) STB__ptr(p,STB__FIXSIZE(sz)-8) = STB__SIG+1;
      * (unsigned int *) STB__ptr(p,STB__FIXSIZE(sz)-12) = STB__SIG+1;
      * (unsigned int *) STB__ptr(p,STB__FIXSIZE(sz)-16) = STB__SIG+1;
      p = STB__ptr(p, STB__BIAS);
      #endif
      stb_wrapper_malloc(p,sz,file,line);
      return p;
   }

   void * stb__realloc(void *p, int sz, char *file, int line)
   {
      void *q;

      stb_mcheck_all();
      if (p == NULL) return stb__malloc(sz,file,line);
      if (sz == 0  ) { stb__free(p,file,line); return NULL; }

      #ifdef STB_MALLOC_WRAPPER_DEBUG
         stb_mcheck(p);
         p = STB__ptr(p,-STB__BIAS);
      #endif
      #ifdef STB_MALLOC_WRAPPER_PAGED
      {
         int n = stb_wrapper_allocsize(STB__ptr(p,STB__BIAS));
         if (!n)
            stb_wrapper_check(STB__ptr(p,STB__BIAS));
         q = stb__realloc_final(p, STB__FIXSIZE(sz), STB__FIXSIZE(n));
      }
      #else
      q = realloc(p, STB__FIXSIZE(sz));
      #endif
      if (q == NULL)
         return ++stb__malloc_failure, q;
      #ifdef STB_MALLOC_WRAPPER_DEBUG
      * (int *) STB__ptr(q,0) = STB__FIXSIZE(sz);
      * (unsigned int *) STB__ptr(q,4) = STB__SIG;
      * (unsigned int *) STB__ptr(q,8) = STB__SIG;
      * (unsigned int *) STB__ptr(q,12) = STB__SIG;
      * (unsigned int *) STB__ptr(q,STB__FIXSIZE(sz)-4) = STB__SIG+1;
      * (unsigned int *) STB__ptr(q,STB__FIXSIZE(sz)-8) = STB__SIG+1;
      * (unsigned int *) STB__ptr(q,STB__FIXSIZE(sz)-12) = STB__SIG+1;
      * (unsigned int *) STB__ptr(q,STB__FIXSIZE(sz)-16) = STB__SIG+1;

      q = STB__ptr(q, STB__BIAS);
      p = STB__ptr(p, STB__BIAS);
      #endif
      stb_wrapper_realloc(p,q,sz,file,line);
      return q;
   }

   STB_EXTERN int stb_log2_ceil(unsigned int);
   static void *stb__calloc(size_t n, size_t sz, char *file, int line)
   {
      void *q;
      stb_mcheck_all();
      if (n == 0 || sz == 0) return NULL;
      if (stb_log2_ceil(n) + stb_log2_ceil(sz) >= 32) return NULL;
      q = stb__malloc(n*sz, file, line);
      if (q) memset(q, 0, n*sz);
      return q;
   }

   char * stb__strdup(char *s, char *file, int line)
   {
      char *p;
      stb_mcheck_all();
      p = stb__malloc(strlen(s)+1, file, line);
      if (!p) return p;
      strcpy(p, s);
      return p;
   }
   #endif // STB_DEFINE

   #ifdef STB_FASTMALLOC
   #undef malloc
   #undef realloc
   #undef free
   #undef strdup
   #undef calloc
   #endif

   // include everything that might define these, BEFORE making macros
   #include <stdlib.h>
   #include <string.h>
   #include <malloc.h>

   #define malloc(s)      stb__malloc (  s, __FILE__, __LINE__)
   #define realloc(p,s)   stb__realloc(p,s, __FILE__, __LINE__)
   #define calloc(n,s)    stb__calloc (n,s, __FILE__, __LINE__)
   #define free(p)        stb__free   (p,   __FILE__, __LINE__)
   #define strdup(p)      stb__strdup (p,   __FILE__, __LINE__)

#endif

//////////////////////////////////////////////////////////////////////////////
//
//                         Windows UTF8 filename handling
//
// Windows stupidly treats 8-bit filenames as some dopey code page,
// rather than utf-8. If we want to use utf8 filenames, we have to
// convert them to WCHAR explicitly and call WCHAR versions of the
// file functions. So, ok, we do.


#ifdef _WIN32
   #define stb__fopen(x,y)    _wfopen((const wchar_t *)stb__from_utf8(x), (const wchar_t *)stb__from_utf8_alt(y))
   #define stb__windows(x,y)  x
#else
   #define stb__fopen(x,y)    fopen(x,y)
   #define stb__windows(x,y)  y
#endif


typedef unsigned short stb__wchar;

STB_EXTERN stb__wchar * stb_from_utf8(stb__wchar *buffer, char *str, int n);
STB_EXTERN char       * stb_to_utf8  (char *buffer, stb__wchar *str, int n);

STB_EXTERN stb__wchar *stb__from_utf8(char *str);
STB_EXTERN stb__wchar *stb__from_utf8_alt(char *str);
STB_EXTERN char *stb__to_utf8(stb__wchar *str);


#ifdef STB_DEFINE
stb__wchar * stb_from_utf8(stb__wchar *buffer, char *ostr, int n)
{
   unsigned char *str = (unsigned char *) ostr;
   stb_uint32 c;
   int i=0;
   --n;
   while (*str) {
      if (i >= n)
         return NULL;
      if (!(*str & 0x80))
         buffer[i++] = *str++;
      else if ((*str & 0xe0) == 0xc0) {
         if (*str < 0xc2) return NULL;
         c = (*str++ & 0x1f) << 6;
         if ((*str & 0xc0) != 0x80) return NULL;
         buffer[i++] = c + (*str++ & 0x3f);
      } else if ((*str & 0xf0) == 0xe0) {
         if (*str == 0xe0 && (str[1] < 0xa0 || str[1] > 0xbf)) return NULL;
         if (*str == 0xed && str[1] > 0x9f) return NULL; // str[1] < 0x80 is checked below
         c = (*str++ & 0x0f) << 12;
         if ((*str & 0xc0) != 0x80) return NULL;
         c += (*str++ & 0x3f) << 6;
         if ((*str & 0xc0) != 0x80) return NULL;
         buffer[i++] = c + (*str++ & 0x3f);
      } else if ((*str & 0xf8) == 0xf0) {
         if (*str > 0xf4) return NULL;
         if (*str == 0xf0 && (str[1] < 0x90 || str[1] > 0xbf)) return NULL;
         if (*str == 0xf4 && str[1] > 0x8f) return NULL; // str[1] < 0x80 is checked below
         c = (*str++ & 0x07) << 18;
         if ((*str & 0xc0) != 0x80) return NULL;
         c += (*str++ & 0x3f) << 12;
         if ((*str & 0xc0) != 0x80) return NULL;
         c += (*str++ & 0x3f) << 6;
         if ((*str & 0xc0) != 0x80) return NULL;
         c += (*str++ & 0x3f);
         // utf-8 encodings of values used in surrogate pairs are invalid
         if ((c & 0xFFFFF800) == 0xD800) return NULL;
         if (c >= 0x10000) {
            c -= 0x10000;
            if (i + 2 > n) return NULL;
            buffer[i++] = 0xD800 | (0x3ff & (c >> 10));
            buffer[i++] = 0xDC00 | (0x3ff & (c      ));
         }
      } else
         return NULL;
   }
   buffer[i] = 0;
   return buffer;
}

char * stb_to_utf8(char *buffer, stb__wchar *str, int n)
{
   int i=0;
   --n;
   while (*str) {
      if (*str < 0x80) {
         if (i+1 > n) return NULL;
         buffer[i++] = (char) *str++;
      } else if (*str < 0x800) {
         if (i+2 > n) return NULL;
         buffer[i++] = 0xc0 + (*str >> 6);
         buffer[i++] = 0x80 + (*str & 0x3f);
         str += 1;
      } else if (*str >= 0xd800 && *str < 0xdc00) {
         stb_uint32 c;
         if (i+4 > n) return NULL;
         c = ((str[0] - 0xd800) << 10) + ((str[1]) - 0xdc00) + 0x10000;
         buffer[i++] = 0xf0 + (c >> 18);
         buffer[i++] = 0x80 + ((c >> 12) & 0x3f);
         buffer[i++] = 0x80 + ((c >>  6) & 0x3f);
         buffer[i++] = 0x80 + ((c      ) & 0x3f);
         str += 2;
      } else if (*str >= 0xdc00 && *str < 0xe000) {
         return NULL;
      } else {
         if (i+3 > n) return NULL;
         buffer[i++] = 0xe0 + (*str >> 12);
         buffer[i++] = 0x80 + ((*str >> 6) & 0x3f);
         buffer[i++] = 0x80 + ((*str     ) & 0x3f);
         str += 1;
      }
   }
   buffer[i] = 0;
   return buffer;
}

stb__wchar *stb__from_utf8(char *str)
{
   static stb__wchar buffer[4096];
   return stb_from_utf8(buffer, str, 4096);
}

stb__wchar *stb__from_utf8_alt(char *str)
{
   static stb__wchar buffer[64];
   return stb_from_utf8(buffer, str, 64);
}

char *stb__to_utf8(stb__wchar *str)
{
   static char buffer[4096];
   return stb_to_utf8(buffer, str, 4096);
}

#endif

//////////////////////////////////////////////////////////////////////////////
//
//                         Miscellany
//

STB_EXTERN void stb_fatal(char *fmt, ...);
STB_EXTERN void stb_(char *fmt, ...);
STB_EXTERN void stb_append_to_file(char *file, char *fmt, ...);
STB_EXTERN void stb_log(int active);
STB_EXTERN void stb_log_fileline(int active);
STB_EXTERN void stb_log_name(char *filename);

STB_EXTERN void stb_swap(void *p, void *q, size_t sz);
STB_EXTERN void *stb_copy(void *p, size_t sz);
STB_EXTERN void stb_pointer_array_free(void *p, int len);
STB_EXTERN void **stb_array_block_alloc(int count, int blocksize);

#define stb_arrcount(x)   (sizeof(x)/sizeof((x)[0]))


STB_EXTERN int  stb__record_fileline(char *f, int n);

#ifdef STB_DEFINE

static char *stb__file;
static int   stb__line;

int  stb__record_fileline(char *f, int n)
{
   stb__file = f;
   stb__line = n;
   return 0;
}

void stb_fatal(char *s, ...)
{
   va_list a;
   if (stb__file)
      fprintf(stderr, "[%s:%d] ", stb__file, stb__line);
   va_start(a,s);
   fputs("Fatal error: ", stderr);
   vfprintf(stderr, s, a);
   va_end(a);
   fputs("\n", stderr);
   #ifdef STB_DEBUG
   #ifdef _MSC_VER
   #ifndef STB_PTR64
   __asm int 3;   // trap to debugger!
   #else
   __debugbreak();
   #endif
   #else
   __builtin_trap();
   #endif
   #endif
   exit(1);
}

static int stb__log_active=1, stb__log_fileline=1;

void stb_log(int active)
{
   stb__log_active = active;
}

void stb_log_fileline(int active)
{
   stb__log_fileline = active;
}

#ifdef STB_NO_STB_STRINGS
char *stb__log_filename = "temp.log";
#else
char *stb__log_filename = "stb.log";
#endif

void stb_log_name(char *s)
{
   stb__log_filename = s;
}

void stb_(char *s, ...)
{
   if (stb__log_active) {
      FILE *f = fopen(stb__log_filename, "a");
      if (f) {
         va_list a;
         if (stb__log_fileline && stb__file)
            fprintf(f, "[%s:%4d] ", stb__file, stb__line);
         va_start(a,s);
         vfprintf(f, s, a);
         va_end(a);
         fputs("\n", f);
         fclose(f);
      }
   }
}

void stb_append_to_file(char *filename, char *s, ...)
{
   FILE *f = fopen(filename, "a");
   if (f) {
      va_list a;
      va_start(a,s);
      vfprintf(f, s, a);
      va_end(a);
      fputs("\n", f);
      fclose(f);
   }
}


typedef struct { char d[4]; } stb__4;
typedef struct { char d[8]; } stb__8;

// optimize the small cases, though you shouldn't be calling this for those!
void stb_swap(void *p, void *q, size_t sz)
{
   char buffer[256];
   if (p == q) return;
   if (sz == 4) {
      stb__4 temp    = * ( stb__4 *) p;
      * (stb__4 *) p = * ( stb__4 *) q;
      * (stb__4 *) q = temp;
      return;
   } else if (sz == 8) {
      stb__8 temp    = * ( stb__8 *) p;
      * (stb__8 *) p = * ( stb__8 *) q;
      * (stb__8 *) q = temp;
      return;
   }

   while (sz > sizeof(buffer)) {
      stb_swap(p, q, sizeof(buffer));
      p = (char *) p + sizeof(buffer);
      q = (char *) q + sizeof(buffer);
      sz -= sizeof(buffer);
   }

   memcpy(buffer, p     , sz);
   memcpy(p     , q     , sz);
   memcpy(q     , buffer, sz);
}

void *stb_copy(void *p, size_t sz)
{
   void *q = malloc(sz);
   memcpy(q, p, sz);
   return q;
}

void stb_pointer_array_free(void *q, int len)
{
   void **p = (void **) q;
   int i;
   for (i=0; i < len; ++i)
      free(p[i]);
}

void **stb_array_block_alloc(int count, int blocksize)
{
   int i;
   char *p = (char *) malloc(sizeof(void *) * count + count * blocksize);
   void **q;
   if (p == NULL) return NULL;
   q = (void **) p;
   p += sizeof(void *) * count;
   for (i=0; i < count; ++i)
      q[i] = p + i * blocksize;
   return q;
}
#endif

#ifdef STB_DEBUG
   // tricky hack to allow recording FILE,LINE even in varargs functions
   #define STB__RECORD_FILE(x)  (stb__record_fileline(__FILE__, __LINE__),(x))
   #define stb_log              STB__RECORD_FILE(stb_log)
   #define stb_                 STB__RECORD_FILE(stb_)
   #ifndef STB_FATAL_CLEAN
   #define stb_fatal            STB__RECORD_FILE(stb_fatal)
   #endif
   #define STB__DEBUG(x)        x
#else
   #define STB__DEBUG(x)
#endif

//////////////////////////////////////////////////////////////////////////////
//
//                         stb_temp
//

#define stb_temp(block, sz)     stb__temp(block, sizeof(block), (sz))

STB_EXTERN void * stb__temp(void *b, int b_sz, int want_sz);
STB_EXTERN void   stb_tempfree(void *block, void *ptr);

#ifdef STB_DEFINE

void * stb__temp(void *b, int b_sz, int want_sz)
{
   if (b_sz >= want_sz)
      return b;
   else
      return malloc(want_sz);
}

void   stb_tempfree(void *b, void *p)
{
   if (p != b)
      free(p);
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
//                         bit operations
//

#define stb_big32(c)    (((c)[0]<<24) + (c)[1]*65536 + (c)[2]*256 + (c)[3])
#define stb_little32(c) (((c)[3]<<24) + (c)[2]*65536 + (c)[1]*256 + (c)[0])
#define stb_big16(c)    ((c)[0]*256 + (c)[1])
#define stb_little16(c) ((c)[1]*256 + (c)[0])

STB_EXTERN          int stb_bitcount(unsigned int a);
STB_EXTERN unsigned int stb_bitreverse8(unsigned char n);
STB_EXTERN unsigned int stb_bitreverse(unsigned int n);

STB_EXTERN          int stb_is_pow2(unsigned int n);
STB_EXTERN          int stb_log2_ceil(unsigned int n);
STB_EXTERN          int stb_log2_floor(unsigned int n);

STB_EXTERN          int stb_lowbit8(unsigned int n);
STB_EXTERN          int stb_highbit8(unsigned int n);

#ifdef STB_DEFINE
int stb_bitcount(unsigned int a)
{
   a = (a & 0x55555555) + ((a >>  1) & 0x55555555); // max 2
   a = (a & 0x33333333) + ((a >>  2) & 0x33333333); // max 4
   a = (a + (a >> 4)) & 0x0f0f0f0f; // max 8 per 4, now 8 bits
   a = (a + (a >> 8)); // max 16 per 8 bits
   a = (a + (a >> 16)); // max 32 per 8 bits
   return a & 0xff;
}

unsigned int stb_bitreverse8(unsigned char n)
{
   n = ((n & 0xAA) >> 1) + ((n & 0x55) << 1);
   n = ((n & 0xCC) >> 2) + ((n & 0x33) << 2);
   return (unsigned char) ((n >> 4) + (n << 4));
}

unsigned int stb_bitreverse(unsigned int n)
{
  n = ((n & 0xAAAAAAAA) >>  1) | ((n & 0x55555555) << 1);
  n = ((n & 0xCCCCCCCC) >>  2) | ((n & 0x33333333) << 2);
  n = ((n & 0xF0F0F0F0) >>  4) | ((n & 0x0F0F0F0F) << 4);
  n = ((n & 0xFF00FF00) >>  8) | ((n & 0x00FF00FF) << 8);
  return (n >> 16) | (n << 16);
}

int stb_is_pow2(unsigned int n)
{
   return (n & (n-1)) == 0;
}

// tricky use of 4-bit table to identify 5 bit positions (note the '-1')
// 3-bit table would require another tree level; 5-bit table wouldn't save one
#if defined(_WIN32) && !defined(__MINGW32__)
#pragma warning(push)
#pragma warning(disable: 4035)  // disable warning about no return value
int stb_log2_floor(unsigned int n)
{
   #if _MSC_VER > 1700
   unsigned long i;
   _BitScanReverse(&i, n);
   return i != 0 ? i : -1;
   #else
   __asm {
      bsr eax,n
      jnz done
      mov eax,-1
   }
   done:;
   #endif
}
#pragma warning(pop)
#else
int stb_log2_floor(unsigned int n)
{
   static signed char log2_4[16] = { -1,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3 };

   // 2 compares if n < 16, 3 compares otherwise
   if (n < (1U << 14))
        if (n < (1U <<  4))        return     0 + log2_4[n      ];
        else if (n < (1U <<  9))      return  5 + log2_4[n >>  5];
             else                     return 10 + log2_4[n >> 10];
   else if (n < (1U << 24))
             if (n < (1U << 19))      return 15 + log2_4[n >> 15];
             else                     return 20 + log2_4[n >> 20];
        else if (n < (1U << 29))      return 25 + log2_4[n >> 25];
             else                     return 30 + log2_4[n >> 30];
}
#endif

// define ceil from floor
int stb_log2_ceil(unsigned int n)
{
   if (stb_is_pow2(n))  return     stb_log2_floor(n);
   else                 return 1 + stb_log2_floor(n);
}

int stb_highbit8(unsigned int n)
{
   return stb_log2_ceil(n&255);
}

int stb_lowbit8(unsigned int n)
{
   static signed char lowbit4[16] = { -1,0,1,0, 2,0,1,0, 3,0,1,0, 2,0,1,0 };
   int k = lowbit4[n & 15];
   if (k >= 0) return k;
   k = lowbit4[(n >> 4) & 15];
   if (k >= 0) return k+4;
   return k;
}
#endif



//////////////////////////////////////////////////////////////////////////////
//
//                            qsort Compare Routines
//

#ifdef _WIN32
   #define stb_stricmp(a,b) stricmp(a,b)
   #define stb_strnicmp(a,b,n) strnicmp(a,b,n)
#else
   #define stb_stricmp(a,b) strcasecmp(a,b)
   #define stb_strnicmp(a,b,n) strncasecmp(a,b,n)
#endif


STB_EXTERN int (*stb_intcmp(int offset))(const void *a, const void *b);
STB_EXTERN int (*stb_qsort_strcmp(int offset))(const void *a, const void *b);
STB_EXTERN int (*stb_qsort_stricmp(int offset))(const void *a, const void *b);
STB_EXTERN int (*stb_floatcmp(int offset))(const void *a, const void *b);
STB_EXTERN int (*stb_doublecmp(int offset))(const void *a, const void *b);
STB_EXTERN int (*stb_charcmp(int offset))(const void *a, const void *b);

#ifdef STB_DEFINE
static int stb__intcmpoffset, stb__charcmpoffset, stb__strcmpoffset;
static int stb__floatcmpoffset, stb__doublecmpoffset;

int stb__intcmp(const void *a, const void *b)
{
   const int p = *(const int *) ((const char *) a + stb__intcmpoffset);
   const int q = *(const int *) ((const char *) b + stb__intcmpoffset);
   return p < q ? -1 : p > q;
}

int stb__charcmp(const void *a, const void *b)
{
   const int p = *(const unsigned char *) ((const char *) a + stb__charcmpoffset);
   const int q = *(const unsigned char *) ((const char *) b + stb__charcmpoffset);
   return p < q ? -1 : p > q;
}

int stb__floatcmp(const void *a, const void *b)
{
   const float p = *(const float *) ((const char *) a + stb__floatcmpoffset);
   const float q = *(const float *) ((const char *) b + stb__floatcmpoffset);
   return p < q ? -1 : p > q;
}

int stb__doublecmp(const void *a, const void *b)
{
   const double p = *(const double *) ((const char *) a + stb__doublecmpoffset);
   const double q = *(const double *) ((const char *) b + stb__doublecmpoffset);
   return p < q ? -1 : p > q;
}

int stb__qsort_strcmp(const void *a, const void *b)
{
   const char *p = *(const char **) ((const char *) a + stb__strcmpoffset);
   const char *q = *(const char **) ((const char *) b + stb__strcmpoffset);
   return strcmp(p,q);
}

int stb__qsort_stricmp(const void *a, const void *b)
{
   const char *p = *(const char **) ((const char *) a + stb__strcmpoffset);
   const char *q = *(const char **) ((const char *) b + stb__strcmpoffset);
   return stb_stricmp(p,q);
}

int (*stb_intcmp(int offset))(const void *, const void *)
{
   stb__intcmpoffset = offset;
   return &stb__intcmp;
}

int (*stb_charcmp(int offset))(const void *, const void *)
{
   stb__charcmpoffset = offset;
   return &stb__charcmp;
}

int (*stb_qsort_strcmp(int offset))(const void *, const void *)
{
   stb__strcmpoffset = offset;
   return &stb__qsort_strcmp;
}

int (*stb_qsort_stricmp(int offset))(const void *, const void *)
{
   stb__strcmpoffset = offset;
   return &stb__qsort_stricmp;
}

int (*stb_floatcmp(int offset))(const void *, const void *)
{
   stb__floatcmpoffset = offset;
   return &stb__floatcmp;
}

int (*stb_doublecmp(int offset))(const void *, const void *)
{
   stb__doublecmpoffset = offset;
   return &stb__doublecmp;
}

#endif

//////////////////////////////////////////////////////////////////////////////
//
//                           String Processing
//

#define stb_prefixi(s,t)  (0==stb_strnicmp((s),(t),strlen(t)))

enum stb_splitpath_flag
{
   STB_PATH = 1,
   STB_FILE = 2,
   STB_EXT  = 4,
   STB_PATH_FILE = STB_PATH + STB_FILE,
   STB_FILE_EXT  = STB_FILE + STB_EXT,
   STB_EXT_NO_PERIOD = 8,
};

STB_EXTERN char * stb_skipwhite(char *s);
STB_EXTERN char * stb_trimwhite(char *s);
STB_EXTERN char * stb_skipnewline(char *s);
STB_EXTERN char * stb_strncpy(char *s, char *t, int n);
STB_EXTERN char * stb_substr(char *t, int n);
STB_EXTERN char * stb_duplower(char *s);
STB_EXTERN void   stb_tolower (char *s);
STB_EXTERN char * stb_strchr2 (char *s, char p1, char p2);
STB_EXTERN char * stb_strrchr2(char *s, char p1, char p2);
STB_EXTERN char * stb_strtok(char *output, char *src, char *delimit);
STB_EXTERN char * stb_strtok_keep(char *output, char *src, char *delimit);
STB_EXTERN char * stb_strtok_invert(char *output, char *src, char *allowed);
STB_EXTERN char * stb_dupreplace(char *s, char *find, char *replace);
STB_EXTERN void   stb_replaceinplace(char *s, char *find, char *replace);
STB_EXTERN char * stb_splitpath(char *output, char *src, int flag);
STB_EXTERN char * stb_splitpathdup(char *src, int flag);
STB_EXTERN char * stb_replacedir(char *output, char *src, char *dir);
STB_EXTERN char * stb_replaceext(char *output, char *src, char *ext);
STB_EXTERN void   stb_fixpath(char *path);
STB_EXTERN char * stb_shorten_path_readable(char *path, int max_len);
STB_EXTERN int    stb_suffix (char *s, char *t);
STB_EXTERN int    stb_suffixi(char *s, char *t);
STB_EXTERN int    stb_prefix (char *s, char *t);
STB_EXTERN char * stb_strichr(char *s, char t);
STB_EXTERN char * stb_stristr(char *s, char *t);
STB_EXTERN int    stb_prefix_count(char *s, char *t);
STB_EXTERN char * stb_plural(int n);  // "s" or ""

STB_EXTERN char **stb_tokens(char *src, char *delimit, int *count);
STB_EXTERN char **stb_tokens_nested(char *src, char *delimit, int *count, char *nest_in, char *nest_out);
STB_EXTERN char **stb_tokens_nested_empty(char *src, char *delimit, int *count, char *nest_in, char *nest_out);
STB_EXTERN char **stb_tokens_allowempty(char *src, char *delimit, int *count);
STB_EXTERN char **stb_tokens_stripwhite(char *src, char *delimit, int *count);
STB_EXTERN char **stb_tokens_withdelim(char *src, char *delimit, int *count);
STB_EXTERN char **stb_tokens_quoted(char *src, char *delimit, int *count);
// with 'quoted', allow delimiters to appear inside quotation marks, and don't
// strip whitespace inside them (and we delete the quotation marks unless they
// appear back to back, in which case they're considered escaped)

#ifdef STB_DEFINE

char *stb_plural(int n)
{
   return n == 1 ? "" : "s";
}

int stb_prefix(char *s, char *t)
{
   while (*t)
      if (*s++ != *t++)
         return STB_FALSE;
   return STB_TRUE;
}

int stb_prefix_count(char *s, char *t)
{
   int c=0;
   while (*t) {
      if (*s++ != *t++)
         break;
      ++c;
   }
   return c;
}

int stb_suffix(char *s, char *t)
{
   size_t n = strlen(s);
   size_t m = strlen(t);
   if (m <= n)
      return 0 == strcmp(s+n-m, t);
   else
      return 0;
}

int stb_suffixi(char *s, char *t)
{
   size_t n = strlen(s);
   size_t m = strlen(t);
   if (m <= n)
      return 0 == stb_stricmp(s+n-m, t);
   else
      return 0;
}

// originally I was using this table so that I could create known sentinel
// values--e.g. change whitetable[0] to be true if I was scanning for whitespace,
// and false if I was scanning for nonwhite. I don't appear to be using that
// functionality anymore (I do for tokentable, though), so just replace it
// with isspace()
char *stb_skipwhite(char *s)
{
   while (isspace((unsigned char) *s)) ++s;
   return s;
}

char *stb_skipnewline(char *s)
{
   if (s[0] == '\r' || s[0] == '\n') {
      if (s[0]+s[1] == '\r' + '\n') ++s;
      ++s;
   }
   return s;
}

char *stb_trimwhite(char *s)
{
   int i,n;
   s = stb_skipwhite(s);
   n = (int) strlen(s);
   for (i=n-1; i >= 0; --i)
      if (!isspace(s[i]))
         break;
   s[i+1] = 0;
   return s;
}

char *stb_strncpy(char *s, char *t, int n)
{
   strncpy(s,t,n);
   s[n-1] = 0;
   return s;
}

char *stb_substr(char *t, int n)
{
   char *a;
   int z = (int) strlen(t);
   if (z < n) n = z;
   a = (char *) malloc(n+1);
   strncpy(a,t,n);
   a[n] = 0;
   return a;
}

char *stb_duplower(char *s)
{
   char *p = strdup(s), *q = p;
   while (*q) {
      *q = tolower(*q);
      ++q;
   }
   return p;
}

void stb_tolower(char *s)
{
   while (*s) {
      *s = tolower(*s);
      ++s;
   }
}

char *stb_strchr2(char *s, char x, char y)
{
   for(; *s; ++s)
      if (*s == x || *s == y)
         return s;
   return NULL;
}

char *stb_strrchr2(char *s, char x, char y)
{
   char *r = NULL;
   for(; *s; ++s)
      if (*s == x || *s == y)
         r = s;
   return r;
}

char *stb_strichr(char *s, char t)
{
   if (tolower(t) == toupper(t))
      return strchr(s,t);
   return stb_strchr2(s, (char) tolower(t), (char) toupper(t));
}

char *stb_stristr(char *s, char *t)
{
   size_t n = strlen(t);
   char *z;
   if (n==0) return s;
   while ((z = stb_strichr(s, *t)) != NULL) {
      if (0==stb_strnicmp(z, t, n))
         return z;
      s = z+1;
   }
   return NULL;
}

static char *stb_strtok_raw(char *output, char *src, char *delimit, int keep, int invert)
{
   if (invert) {
      while (*src && strchr(delimit, *src) != NULL) {
         *output++ = *src++;
      }
   } else {
      while (*src && strchr(delimit, *src) == NULL) {
         *output++ = *src++;
      }
   }
   *output = 0;
   if (keep)
      return src;
   else
      return *src ? src+1 : src;
}

char *stb_strtok(char *output, char *src, char *delimit)
{
   return stb_strtok_raw(output, src, delimit, 0, 0);
}

char *stb_strtok_keep(char *output, char *src, char *delimit)
{
   return stb_strtok_raw(output, src, delimit, 1, 0);
}

char *stb_strtok_invert(char *output, char *src, char *delimit)
{
   return stb_strtok_raw(output, src, delimit, 1,1);
}

static char **stb_tokens_raw(char *src_, char *delimit, int *count,
                             int stripwhite, int allow_empty, char *start, char *end)
{
   int nested = 0;
   unsigned char *src = (unsigned char *) src_;
   static char stb_tokentable[256]; // rely on static initializion to 0
   static char stable[256],etable[256];
   char *out;
   char **result;
   int num=0;
   unsigned char *s;

   s = (unsigned char *) delimit; while (*s) stb_tokentable[*s++] = 1;
   if (start) {
      s = (unsigned char *) start;         while (*s) stable[*s++] = 1;
      s = (unsigned char *) end;   if (s)  while (*s) stable[*s++] = 1;
      s = (unsigned char *) end;   if (s)  while (*s) etable[*s++] = 1;
   }
   stable[0] = 1;

   // two passes through: the first time, counting how many
   s = (unsigned char *) src;
   while (*s) {
      // state: just found delimiter
      // skip further delimiters
      if (!allow_empty) {
         stb_tokentable[0] = 0;
         while (stb_tokentable[*s])
            ++s;
         if (!*s) break;
      }
      ++num;
      // skip further non-delimiters
      stb_tokentable[0] = 1;
      if (stripwhite == 2) { // quoted strings
         while (!stb_tokentable[*s]) {
            if (*s != '"')
               ++s;
            else {
               ++s;
               if (*s == '"')
                  ++s;   // "" -> ", not start a string
               else {
                  // begin a string
                  while (*s) {
                     if (s[0] == '"') {
                        if (s[1] == '"') s += 2; // "" -> "
                        else { ++s; break; } // terminating "
                     } else
                        ++s;
                  }
               }
            }
         }
      } else 
         while (nested || !stb_tokentable[*s]) {
            if (stable[*s]) {
               if (!*s) break;
               if (end ? etable[*s] : nested)
                  --nested;
               else
                  ++nested;
            }
            ++s;
         }
      if (allow_empty) {
         if (*s) ++s;
      }
   }
   // now num has the actual count... malloc our output structure
   // need space for all the strings: strings won't be any longer than
   // original input, since for every '\0' there's at least one delimiter
   result = (char **) malloc(sizeof(*result) * (num+1) + (s-src+1));
   if (result == NULL) return result;
   out = (char *) (result + (num+1));
   // second pass: copy out the data
   s = (unsigned char *) src;
   num = 0;
   nested = 0;
   while (*s) {
      char *last_nonwhite;
      // state: just found delimiter
      // skip further delimiters
      if (!allow_empty) {
         stb_tokentable[0] = 0;
         if (stripwhite)
            while (stb_tokentable[*s] || isspace(*s))
               ++s;
         else
            while (stb_tokentable[*s])
               ++s;
      } else if (stripwhite) {
         while (isspace(*s)) ++s;
      }
      if (!*s) break;
      // we're past any leading delimiters and whitespace
      result[num] = out;
      ++num;
      // copy non-delimiters
      stb_tokentable[0] = 1;
      last_nonwhite = out-1;
      if (stripwhite == 2) {
         while (!stb_tokentable[*s]) {
            if (*s != '"') {
               if (!isspace(*s)) last_nonwhite = out;
               *out++ = *s++;
            } else {
               ++s;
               if (*s == '"') {
                  if (!isspace(*s)) last_nonwhite = out;
                  *out++ = *s++; // "" -> ", not start string
               } else {
                  // begin a quoted string
                  while (*s) {
                     if (s[0] == '"') {
                        if (s[1] == '"') { *out++ = *s; s += 2; }
                        else { ++s; break; } // terminating "
                     } else
                        *out++ = *s++;
                  }
                  last_nonwhite = out-1; // all in quotes counts as non-white
               }
            }
         }
      } else {
         while (nested || !stb_tokentable[*s]) {
            if (!isspace(*s)) last_nonwhite = out;
            if (stable[*s]) {
               if (!*s) break;
               if (end ? etable[*s] : nested)
                  --nested;
               else
                  ++nested;
            }
            *out++ = *s++;
         }
      }

      if (stripwhite) // rewind to last non-whitespace char
         out = last_nonwhite+1;
      *out++ = '\0';

      if (*s) ++s; // skip delimiter
   }
   s = (unsigned char *) delimit; while (*s) stb_tokentable[*s++] = 0;
   if (start) {
      s = (unsigned char *) start;         while (*s) stable[*s++] = 1;
      s = (unsigned char *) end;   if (s)  while (*s) stable[*s++] = 1;
      s = (unsigned char *) end;   if (s)  while (*s) etable[*s++] = 1;
   }
   if (count != NULL) *count = num;
   result[num] = 0;
   return result;
}

char **stb_tokens(char *src, char *delimit, int *count)
{
   return stb_tokens_raw(src,delimit,count,0,0,0,0);
}

char **stb_tokens_nested(char *src, char *delimit, int *count, char *nest_in, char *nest_out)
{
   return stb_tokens_raw(src,delimit,count,0,0,nest_in,nest_out);
}

char **stb_tokens_nested_empty(char *src, char *delimit, int *count, char *nest_in, char *nest_out)
{
   return stb_tokens_raw(src,delimit,count,0,1,nest_in,nest_out);
}

char **stb_tokens_allowempty(char *src, char *delimit, int *count)
{
   return stb_tokens_raw(src,delimit,count,0,1,0,0);
}

char **stb_tokens_stripwhite(char *src, char *delimit, int *count)
{
   return stb_tokens_raw(src,delimit,count,1,1,0,0);
}

char **stb_tokens_quoted(char *src, char *delimit, int *count)
{
   return stb_tokens_raw(src,delimit,count,2,1,0,0);
}

char *stb_dupreplace(char *src, char *find, char *replace)
{
   size_t len_find = strlen(find);
   size_t len_replace = strlen(replace);
   int count = 0;

   char *s,*p,*q;

   s = strstr(src, find);
   if (s == NULL) return strdup(src);
   do {
      ++count;
      s = strstr(s + len_find, find);
   } while (s != NULL);

   p = (char *)  malloc(strlen(src) + count * (len_replace - len_find) + 1);
   if (p == NULL) return p;
   q = p;
   s = src;
   for (;;) {
      char *t = strstr(s, find);
      if (t == NULL) {
         strcpy(q,s);
         assert(strlen(p) == strlen(src) + count*(len_replace-len_find));
         return p;
      }
      memcpy(q, s, t-s);
      q += t-s;
      memcpy(q, replace, len_replace);
      q += len_replace;
      s = t + len_find;
   }
}

void stb_replaceinplace(char *src, char *find, char *replace)
{
   size_t len_find = strlen(find);
   size_t len_replace = strlen(replace);
   int delta;

   char *s,*p,*q;

   delta = len_replace - len_find;
   assert(delta <= 0);
   if (delta > 0) return;

   p = strstr(src, find);
   if (p == NULL) return;

   s = q = p;
   while (*s) {
      memcpy(q, replace, len_replace);
      p += len_find;
      q += len_replace;
      s = strstr(p, find);
      if (s == NULL) s = p + strlen(p);
      memmove(q, p, s-p);
      q += s-p;
      p = s;
   }
   *q = 0;
}

void stb_fixpath(char *path)
{
   for(; *path; ++path)
      if (*path == '\\')
         *path = '/';
}

void stb__add_section(char *buffer, char *data, int curlen, int newlen)
{
   if (newlen < curlen) {
      int z1 = newlen >> 1, z2 = newlen-z1;
      memcpy(buffer, data, z1-1);
      buffer[z1-1] = '.';
      buffer[z1-0] = '.';
      memcpy(buffer+z1+1, data+curlen-z2+1, z2-1);
   } else
      memcpy(buffer, data, curlen);
}

char * stb_shorten_path_readable(char *path, int len)
{
   static char buffer[1024];
   int n = strlen(path),n1,n2,r1,r2;
   char *s;
   if (n <= len) return path;
   if (len > 1024) return path;
   s = stb_strrchr2(path, '/', '\\');
   if (s) {
      n1 = s - path + 1;
      n2 = n - n1;
      ++s;
   } else {
      n1 = 0;
      n2 = n;
      s = path;
   }
   // now we need to reduce r1 and r2 so that they fit in len
   if (n1 < len>>1) {
      r1 = n1;
      r2 = len - r1;
   } else if (n2 < len >> 1) {
      r2 = n2;
      r1 = len - r2;
   } else {
      r1 = n1 * len / n;
      r2 = n2 * len / n;
      if (r1 < len>>2) r1 = len>>2, r2 = len-r1;
      if (r2 < len>>2) r2 = len>>2, r1 = len-r2;
   }
   assert(r1 <= n1 && r2 <= n2);
   if (n1)
      stb__add_section(buffer, path, n1, r1);
   stb__add_section(buffer+r1, s, n2, r2);
   buffer[len] = 0;
   return buffer;
}

static char *stb__splitpath_raw(char *buffer, char *path, int flag)
{
   int len=0,x,y, n = (int) strlen(path), f1,f2;
   char *s = stb_strrchr2(path, '/', '\\');
   char *t = strrchr(path, '.');
   if (s && t && t < s) t = NULL;
   if (s) ++s;

   if (flag == STB_EXT_NO_PERIOD)
      flag |= STB_EXT;

   if (!(flag & (STB_PATH | STB_FILE | STB_EXT))) return NULL;

   f1 = s == NULL ? 0 : s-path; // start of filename
   f2 = t == NULL ? n : t-path; // just past end of filename

   if (flag & STB_PATH) {
      x = 0; if (f1 == 0 && flag == STB_PATH) len=2;
   } else if (flag & STB_FILE) {
      x = f1;
   } else {
      x = f2;
      if (flag & STB_EXT_NO_PERIOD)
         if (buffer[x] == '.')
            ++x;
   }

   if (flag & STB_EXT)
      y = n;
   else if (flag & STB_FILE)
      y = f2;
   else
      y = f1;

   if (buffer == NULL) {
      buffer = (char *) malloc(y-x + len + 1);
      if (!buffer) return NULL;
   }

   if (len) { strcpy(buffer, "./"); return buffer; }
   strncpy(buffer, path+x, y-x);
   buffer[y-x] = 0;
   return buffer;
}

char *stb_splitpath(char *output, char *src, int flag)
{
   return stb__splitpath_raw(output, src, flag);
}

char *stb_splitpathdup(char *src, int flag)
{
   return stb__splitpath_raw(NULL, src, flag);
}

char *stb_replacedir(char *output, char *src, char *dir)
{
   char buffer[4096];
   stb_splitpath(buffer, src, STB_FILE | STB_EXT);
   if (dir)
      sprintf(output, "%s/%s", dir, buffer);
   else
      strcpy(output, buffer);
   return output;
}

char *stb_replaceext(char *output, char *src, char *ext)
{
   char buffer[4096];
   stb_splitpath(buffer, src, STB_PATH | STB_FILE);
   if (ext)
      sprintf(output, "%s.%s", buffer, ext[0] == '.' ? ext+1 : ext);
   else
      strcpy(output, buffer);
   return output;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
//                   stb_alloc - hierarchical allocator
//
//                                     inspired by http://swapped.cc/halloc
//
//
// When you alloc a given block through stb_alloc, you have these choices:
//
//       1. does it have a parent?
//       2. can it have children?
//       3. can it be freed directly?
//       4. is it transferrable?
//       5. what is its alignment?
//
// Here are interesting combinations of those:
//
//                              children   free    transfer     alignment
//  arena                          Y         Y         N           n/a
//  no-overhead, chunked           N         N         N         normal
//  string pool alloc              N         N         N            1
//  parent-ptr, chunked            Y         N         N         normal
//  low-overhead, unchunked        N         Y         Y         normal
//  general purpose alloc          Y         Y         Y         normal
//
// Unchunked allocations will probably return 16-aligned pointers. If
// we 16-align the results, we have room for 4 pointers. For smaller
// allocations that allow finer alignment, we can reduce the pointers.
//
// The strategy is that given a pointer, assuming it has a header (only
// the no-overhead allocations have no header), we can determine the
// type of the header fields, and the number of them, by stepping backwards
// through memory and looking at the tags in the bottom bits.
//
// Implementation strategy:
//     chunked allocations come from the middle of chunks, and can't
//     be freed. thefore they do not need to be on a sibling chain.
//     they may need child pointers if they have children.
//
// chunked, with-children
//     void *parent;
//
// unchunked, no-children -- reduced storage
//     void *next_sibling;
//     void *prev_sibling_nextp;
//
// unchunked, general
//     void *first_child;
//     void *next_sibling;
//     void *prev_sibling_nextp;
//     void *chunks;
//
// so, if we code each of these fields with different bit patterns
// (actually same one for next/prev/child), then we can identify which
// each one is from the last field.

STB_EXTERN void  stb_free(void *p);
STB_EXTERN void *stb_malloc_global(size_t size);
STB_EXTERN void *stb_malloc(void *context, size_t size);
STB_EXTERN void *stb_malloc_nofree(void *context, size_t size);
STB_EXTERN void *stb_malloc_leaf(void *context, size_t size);
STB_EXTERN void *stb_malloc_raw(void *context, size_t size);
STB_EXTERN void *stb_realloc(void *ptr, size_t newsize);

STB_EXTERN void stb_reassign(void *new_context, void *ptr);
STB_EXTERN void stb_malloc_validate(void *p, void *parent);

extern int stb_alloc_chunk_size ;
extern int stb_alloc_count_free ;
extern int stb_alloc_count_alloc;
extern int stb_alloc_alignment  ;

#ifdef STB_DEFINE

int stb_alloc_chunk_size  = 65536;
int stb_alloc_count_free  = 0;
int stb_alloc_count_alloc = 0;
int stb_alloc_alignment   = -16;

typedef struct stb__chunk
{
   struct stb__chunk *next;
   int                data_left;
   int                alloc;
} stb__chunk;

typedef struct
{
   void *  next;
   void ** prevn;
} stb__nochildren;

typedef struct
{
   void ** prevn;
   void *  child;
   void *  next;
   stb__chunk *chunks;
} stb__alloc;

typedef struct
{
   stb__alloc *parent;
} stb__chunked;

#define STB__PARENT          1
#define STB__CHUNKS          2

typedef enum
{
   STB__nochildren = 0,
   STB__chunked    = STB__PARENT,
   STB__alloc      = STB__CHUNKS,

   STB__chunk_raw  = 4,
} stb__alloc_type;

// these functions set the bottom bits of a pointer efficiently
#define STB__DECODE(x,v)  ((void *) ((char *) (x) - (v)))
#define STB__ENCODE(x,v)  ((void *) ((char *) (x) + (v)))

#define stb__parent(z)       (stb__alloc *) STB__DECODE((z)->parent, STB__PARENT)
#define stb__chunks(z)       (stb__chunk *) STB__DECODE((z)->chunks, STB__CHUNKS)

#define stb__setparent(z,p)  (z)->parent = (stb__alloc *) STB__ENCODE((p), STB__PARENT)
#define stb__setchunks(z,c)  (z)->chunks = (stb__chunk *) STB__ENCODE((c), STB__CHUNKS)

static stb__alloc stb__alloc_global =
{
   NULL,
   NULL,
   NULL,
   (stb__chunk *) STB__ENCODE(NULL, STB__CHUNKS)
};

static stb__alloc_type stb__identify(void *p)
{
   void **q = (void **) p;
   return (stb__alloc_type) ((stb_uinta) q[-1] & 3);
}

static void *** stb__prevn(void *p)
{
   if (stb__identify(p) == STB__alloc) {
      stb__alloc      *s = (stb__alloc *) p - 1;
      return &s->prevn;
   } else {
      stb__nochildren *s = (stb__nochildren *) p - 1;
      return &s->prevn;
   }
}

void stb_free(void *p)
{
   if (p == NULL) return;

   // count frees so that unit tests can see what's happening
   ++stb_alloc_count_free;

   switch(stb__identify(p)) {
      case STB__chunked:
         // freeing a chunked-block with children does nothing;
         // they only get freed when the parent does
         // surely this is wrong, and it should free them immediately?
         // otherwise how are they getting put on the right chain?
         return;
      case STB__nochildren: {
         stb__nochildren *s = (stb__nochildren *) p - 1;
         // unlink from sibling chain
         *(s->prevn) = s->next;
         if (s->next)
            *stb__prevn(s->next) = s->prevn;
         free(s);
         return;
      }
      case STB__alloc: {
         stb__alloc *s = (stb__alloc *) p - 1;
         stb__chunk *c, *n;
         void *q;

         // unlink from sibling chain, if any
         *(s->prevn) = s->next;
         if (s->next)
            *stb__prevn(s->next) = s->prevn;

         // first free chunks
         c = (stb__chunk *) stb__chunks(s);
         while (c != NULL) {
            n = c->next;
            stb_alloc_count_free += c->alloc;
            free(c);
            c = n;
         }

         // validating
         stb__setchunks(s,NULL);
         s->prevn = NULL;
         s->next = NULL;

         // now free children
         while ((q = s->child) != NULL) {
            stb_free(q);
         }

         // now free self
         free(s);
         return;
      }
      default:
         assert(0); /* NOTREACHED */
   }
}

void stb_malloc_validate(void *p, void *parent)
{
   if (p == NULL) return;

   switch(stb__identify(p)) {
      case STB__chunked:
         return;
      case STB__nochildren: {
         stb__nochildren *n = (stb__nochildren *) p - 1;
         if (n->prevn)
            assert(*n->prevn == p);
         if (n->next) {
            assert(*stb__prevn(n->next) == &n->next);
            stb_malloc_validate(n, parent);
         }
         return;
      }
      case STB__alloc: {
         stb__alloc *s = (stb__alloc *) p - 1;

         if (s->prevn)
            assert(*s->prevn == p);

         if (s->child) {
            assert(*stb__prevn(s->child) == &s->child);
            stb_malloc_validate(s->child, p);
         }

         if (s->next) {
            assert(*stb__prevn(s->next) == &s->next);
            stb_malloc_validate(s->next, parent);
         }
         return;
      }
      default:
         assert(0); /* NOTREACHED */
   }
}

static void * stb__try_chunk(stb__chunk *c, int size, int align, int pre_align)
{
   char *memblock = (char *) (c+1), *q;
   stb_inta iq;
   int start_offset;

   // we going to allocate at the end of the chunk, not the start. confusing,
   // but it means we don't need both a 'limit' and a 'cur', just a 'cur'.
   // the block ends at: p + c->data_left
   //   then we move back by size
   start_offset = c->data_left - size;

   // now we need to check the alignment of that
   q = memblock + start_offset;
   iq = (stb_inta) q;
   assert(sizeof(q) == sizeof(iq));

   // suppose align = 2
   // then we need to retreat iq far enough that (iq & (2-1)) == 0
   // to get (iq & (align-1)) = 0 requires subtracting (iq & (align-1))

   start_offset -= iq & (align-1);
   assert(((stb_uinta) (memblock+start_offset) & (align-1)) == 0);

   // now, if that + pre_align works, go for it!
   start_offset -= pre_align;

   if (start_offset >= 0) {
      c->data_left = start_offset;
      return memblock + start_offset;
   }

   return NULL;
}

static void stb__sort_chunks(stb__alloc *src)
{
   // of the first two chunks, put the chunk with more data left in it first
   stb__chunk *c = stb__chunks(src), *d;
   if (c == NULL) return;
   d = c->next;
   if (d == NULL) return;
   if (c->data_left > d->data_left) return;

   c->next = d->next;
   d->next = c;
   stb__setchunks(src, d);
}

static void * stb__alloc_chunk(stb__alloc *src, int size, int align, int pre_align)
{
   void *p;
   stb__chunk *c = stb__chunks(src);

   if (c && size <= stb_alloc_chunk_size) {

      p = stb__try_chunk(c, size, align, pre_align);
      if (p) { ++c->alloc; return p; }

      // try a second chunk to reduce wastage
      if (c->next) {
         p = stb__try_chunk(c->next, size, align, pre_align);
         if (p) { ++c->alloc; return p; }
   
         // put the bigger chunk first, since the second will get buried
         // the upshot of this is that, until it gets allocated from, chunk #2
         // is always the largest remaining chunk. (could formalize
         // this with a heap!)
         stb__sort_chunks(src);
         c = stb__chunks(src);
      }
   }

   // allocate a new chunk
   {
      stb__chunk *n;

      int chunk_size = stb_alloc_chunk_size;
      // we're going to allocate a new chunk to put this in
      if (size > chunk_size)
         chunk_size = size;

      assert(sizeof(*n) + pre_align <= 16);

      // loop trying to allocate a large enough chunk
      // the loop is because the alignment may cause problems if it's big...
      // and we don't know what our chunk alignment is going to be
      while (1) {
         n = (stb__chunk *) malloc(16 + chunk_size);
         if (n == NULL) return NULL;

         n->data_left = chunk_size - sizeof(*n);

         p = stb__try_chunk(n, size, align, pre_align);
         if (p != NULL) {
            n->next = c;
            stb__setchunks(src, n);

            // if we just used up the whole block immediately,
            // move the following chunk up
            n->alloc = 1;
            if (size == chunk_size)
               stb__sort_chunks(src);

            return p;
         }

         free(n);
         chunk_size += 16+align;
      }
   }
}

static stb__alloc * stb__get_context(void *context)
{
   if (context == NULL) {
      return &stb__alloc_global;
   } else {
      int u = stb__identify(context);
      // if context is chunked, grab parent
      if (u == STB__chunked) {
         stb__chunked *s = (stb__chunked *) context - 1;
         return stb__parent(s);
      } else {
         return (stb__alloc *) context - 1;
      }
   }
}

static void stb__insert_alloc(stb__alloc *src, stb__alloc *s)
{
   s->prevn = &src->child;
   s->next  = src->child;
   src->child = s+1;
   if (s->next)
      *stb__prevn(s->next) = &s->next;
}

static void stb__insert_nochild(stb__alloc *src, stb__nochildren *s)
{
   s->prevn = &src->child;
   s->next  = src->child;
   src->child = s+1;
   if (s->next)
      *stb__prevn(s->next) = &s->next;
}

static void * malloc_base(void *context, size_t size, stb__alloc_type t, int align)
{
   void *p;

   stb__alloc *src = stb__get_context(context);

   if (align <= 0) {
      // compute worst-case C packed alignment
      // e.g. a 24-byte struct is 8-aligned
      int align_proposed = 1 << stb_lowbit8(size);

      if (align_proposed < 0)
         align_proposed = 4;

      if (align_proposed == 0) {
         if (size == 0)
            align_proposed = 1;
         else
            align_proposed = 256;
      }

      // a negative alignment means 'don't align any larger
      // than this'; so -16 means we align 1,2,4,8, or 16

      if (align < 0) {
         if (align_proposed > -align)
            align_proposed = -align;
      }

      align = align_proposed;
   }

   assert(stb_is_pow2(align));

   // don't cause misalignment when allocating nochildren
   if (t == STB__nochildren && align > 8)
      t = STB__alloc;

   switch (t) {
      case STB__alloc: {
         stb__alloc *s = (stb__alloc *) malloc(size + sizeof(*s));
         if (s == NULL) return NULL;
         p = s+1;
         s->child = NULL;
         stb__insert_alloc(src, s);

         stb__setchunks(s,NULL);
         break;
      }

      case STB__nochildren: {
         stb__nochildren *s = (stb__nochildren *) malloc(size + sizeof(*s));
         if (s == NULL) return NULL;
         p = s+1;
         stb__insert_nochild(src, s);
         break;
      }

      case STB__chunk_raw: {
         p = stb__alloc_chunk(src, size, align, 0);
         if (p == NULL) return NULL;
         break;
      }

      case STB__chunked: {
         stb__chunked *s;
         if (align < sizeof(stb_uintptr)) align = sizeof(stb_uintptr);
         s = (stb__chunked *) stb__alloc_chunk(src, size, align, sizeof(*s));
         if (s == NULL) return NULL;
         stb__setparent(s, src);
         p = s+1;
         break;
      }

      default: p = NULL; assert(0); /* NOTREACHED */
   }

   ++stb_alloc_count_alloc;
   return p;
}

void *stb_malloc_global(size_t size)
{
   return malloc_base(NULL, size, STB__alloc, stb_alloc_alignment);
}

void *stb_malloc(void *context, size_t size)
{
   return malloc_base(context, size, STB__alloc, stb_alloc_alignment);
}

void *stb_malloc_nofree(void *context, size_t size)
{
   return malloc_base(context, size, STB__chunked, stb_alloc_alignment);
}

void *stb_malloc_leaf(void *context, size_t size)
{
   return malloc_base(context, size, STB__nochildren, stb_alloc_alignment);
}

void *stb_malloc_raw(void *context, size_t size)
{
   return malloc_base(context, size, STB__chunk_raw, stb_alloc_alignment);
}

char *stb_malloc_string(void *context, size_t size)
{
   return (char *) malloc_base(context, size, STB__chunk_raw, 1);
}

void *stb_realloc(void *ptr, size_t newsize)
{
   stb__alloc_type t;

   if (ptr == NULL) return stb_malloc(NULL, newsize);
   if (newsize == 0) { stb_free(ptr); return NULL; }
   
   t = stb__identify(ptr);
   assert(t == STB__alloc || t == STB__nochildren);

   if (t == STB__alloc) {
      stb__alloc *s = (stb__alloc *) ptr - 1;

      s = (stb__alloc *) realloc(s, newsize + sizeof(*s));
      if (s == NULL) return NULL;

      ptr = s+1;

      // update pointers
      (*s->prevn) = ptr;
      if (s->next)
         *stb__prevn(s->next) = &s->next;

      if (s->child)
         *stb__prevn(s->child) = &s->child;

      return ptr;
   } else {
      stb__nochildren *s = (stb__nochildren *) ptr - 1;

      s = (stb__nochildren *) realloc(ptr, newsize + sizeof(s));
      if (s == NULL) return NULL;

      // update pointers
      (*s->prevn) = s+1;
      if (s->next)
         *stb__prevn(s->next) = &s->next;

      return s+1;
   }
}

void *stb_realloc_c(void *context, void *ptr, size_t newsize)
{
   if (ptr == NULL) return stb_malloc(context, newsize);
   if (newsize == 0) { stb_free(ptr); return NULL; }
   // @TODO: verify you haven't changed contexts
   return stb_realloc(ptr, newsize);
}

void stb_reassign(void *new_context, void *ptr)
{
   stb__alloc *src = stb__get_context(new_context);

   stb__alloc_type t = stb__identify(ptr);
   assert(t == STB__alloc || t == STB__nochildren);

   if (t == STB__alloc) {
      stb__alloc *s = (stb__alloc *) ptr - 1;

      // unlink from old
      *(s->prevn) = s->next;
      if (s->next)
         *stb__prevn(s->next) = s->prevn;

      stb__insert_alloc(src, s);
   } else {
      stb__nochildren *s = (stb__nochildren *) ptr - 1;

      // unlink from old
      *(s->prevn) = s->next;
      if (s->next)
         *stb__prevn(s->next) = s->prevn;

      stb__insert_nochild(src, s);
   }
}

#endif


//////////////////////////////////////////////////////////////////////////////
//
//                                stb_arr
//
//  An stb_arr is directly useable as a pointer (use the actual type in your
//  definition), but when it resizes, it returns a new pointer and you can't
//  use the old one, so you have to be careful to copy-in-out as necessary.
//
//  Use a NULL pointer as a 0-length array.
//
//     float *my_array = NULL, *temp;
//
//     // add elements on the end one at a time
//     stb_arr_push(my_array, 0.0f);
//     stb_arr_push(my_array, 1.0f);
//     stb_arr_push(my_array, 2.0f);
//
//     assert(my_array[1] == 2.0f);
//
//     // add an uninitialized element at the end, then assign it
//     *stb_arr_add(my_array) = 3.0f;
//
//     // add three uninitialized elements at the end
//     temp = stb_arr_addn(my_array,3);
//     temp[0] = 4.0f;
//     temp[1] = 5.0f;
//     temp[2] = 6.0f;
//
//     assert(my_array[5] == 5.0f);
//
//     // remove the last one
//     stb_arr_pop(my_array);
//
//     assert(stb_arr_len(my_array) == 6);


#ifdef STB_MALLOC_WRAPPER
  #define STB__PARAMS    , char *file, int line
  #define STB__ARGS      ,       file,     line
#else
  #define STB__PARAMS
  #define STB__ARGS
#endif

// calling this function allocates an empty stb_arr attached to p
// (whereas NULL isn't attached to anything)
STB_EXTERN void stb_arr_malloc(void **target, void *context);

// call this function with a non-NULL value to have all successive
// stbs that are created be attached to the associated parent. Note
// that once a given stb_arr is non-empty, it stays attached to its
// current parent, even if you call this function again.
// it turns the previous value, so you can restore it
STB_EXTERN void* stb_arr_malloc_parent(void *p);

// simple functions written on top of other functions
#define stb_arr_empty(a)       (  stb_arr_len(a) == 0 )
#define stb_arr_add(a)         (  stb_arr_addn((a),1) )
#define stb_arr_push(a,v)      ( *stb_arr_add(a)=(v)  )

typedef struct
{
   int len, limit;
   int stb_malloc;
   unsigned int signature;
} stb__arr;

#define stb_arr_signature      0x51bada7b  // ends with 0123 in decimal

// access the header block stored before the data
#define stb_arrhead(a)         /*lint --e(826)*/ (((stb__arr *) (a)) - 1)
#define stb_arrhead2(a)        /*lint --e(826)*/ (((stb__arr *) (a)) - 1)

#ifdef STB_DEBUG
#define stb_arr_check(a)       assert(!a || stb_arrhead(a)->signature == stb_arr_signature)
#define stb_arr_check2(a)      assert(!a || stb_arrhead2(a)->signature == stb_arr_signature)
#else
#define stb_arr_check(a)       ((void) 0)
#define stb_arr_check2(a)      ((void) 0)
#endif

// ARRAY LENGTH

// get the array length; special case if pointer is NULL
#define stb_arr_len(a)         (a ? stb_arrhead(a)->len : 0)
#define stb_arr_len2(a)        ((stb__arr *) (a) ? stb_arrhead2(a)->len : 0)
#define stb_arr_lastn(a)       (stb_arr_len(a)-1)

// check whether a given index is valid -- tests 0 <= i < stb_arr_len(a) 
#define stb_arr_valid(a,i)     (a ? (int) (i) < stb_arrhead(a)->len : 0)

// change the array length so is is exactly N entries long, creating
// uninitialized entries as needed
#define stb_arr_setlen(a,n)  \
            (stb__arr_setlen((void **) &(a), sizeof(a[0]), (n)))

// change the array length so that N is a valid index (that is, so
// it is at least N entries long), creating uninitialized entries as needed
#define stb_arr_makevalid(a,n)  \
            (stb_arr_len(a) < (n)+1 ? stb_arr_setlen(a,(n)+1),(a) : (a))

// remove the last element of the array, returning it
#define stb_arr_pop(a)         ((stb_arr_check(a), (a))[--stb_arrhead(a)->len])

// access the last element in the array
#define stb_arr_last(a)        ((stb_arr_check(a), (a))[stb_arr_len(a)-1])

// is iterator at end of list?
#define stb_arr_end(a,i)       ((i) >= &(a)[stb_arr_len(a)])

// (internal) change the allocated length of the array
#define stb_arr__grow(a,n)     (stb_arr_check(a), stb_arrhead(a)->len += (n))

// add N new unitialized elements to the end of the array
#define stb_arr__addn(a,n)     /*lint --e(826)*/ \
                               ((stb_arr_len(a)+(n) > stb_arrcurmax(a))      \
                                 ? (stb__arr_addlen((void **) &(a),sizeof(*a),(n)),0) \
                                 : ((stb_arr__grow(a,n), 0)))

// add N new unitialized elements to the end of the array, and return
// a pointer to the first new one
#define stb_arr_addn(a,n)      (stb_arr__addn((a),n),(a)+stb_arr_len(a)-(n))

// add N new uninitialized elements starting at index 'i'
#define stb_arr_insertn(a,i,n) (stb__arr_insertn((void **) &(a), sizeof(*a), i, n))

// insert an element at i
#define stb_arr_insert(a,i,v)  (stb__arr_insertn((void **) &(a), sizeof(*a), i, n), ((a)[i] = v))

// delete N elements from the middle starting at index 'i'
#define stb_arr_deleten(a,i,n) (stb__arr_deleten((void **) &(a), sizeof(*a), i, n))

// delete the i'th element
#define stb_arr_delete(a,i)   stb_arr_deleten(a,i,1)

// delete the i'th element, swapping down from the end
#define stb_arr_fastdelete(a,i)  \
   (stb_swap(&a[i], &a[stb_arrhead(a)->len-1], sizeof(*a)), stb_arr_pop(a))


// ARRAY STORAGE

// get the array maximum storage; special case if NULL
#define stb_arrcurmax(a)       (a ? stb_arrhead(a)->limit : 0)
#define stb_arrcurmax2(a)      (a ? stb_arrhead2(a)->limit : 0)

// set the maxlength of the array to n in anticipation of further growth
#define stb_arr_setsize(a,n)   (stb_arr_check(a), stb__arr_setsize((void **) &(a),sizeof((a)[0]),n))

// make sure maxlength is large enough for at least N new allocations
#define stb_arr_atleast(a,n)   (stb_arr_len(a)+(n) > stb_arrcurmax(a)      \
                                 ? stb_arr_setsize((a), (n)) : 0)

// make a copy of a given array (copies contents via 'memcpy'!)
#define stb_arr_copy(a)        stb__arr_copy(a, sizeof((a)[0]))

// compute the storage needed to store all the elements of the array
#define stb_arr_storage(a)     (stb_arr_len(a) * sizeof((a)[0]))

#define stb_arr_for(v,arr)     for((v)=(arr); (v) < (arr)+stb_arr_len(arr); ++(v))

// IMPLEMENTATION

STB_EXTERN void stb_arr_free_(void **p);
STB_EXTERN void *stb__arr_copy_(void *p, int elem_size);
STB_EXTERN void stb__arr_setsize_(void **p, int size, int limit  STB__PARAMS);
STB_EXTERN void stb__arr_setlen_(void **p, int size, int newlen  STB__PARAMS);
STB_EXTERN void stb__arr_addlen_(void **p, int size, int addlen  STB__PARAMS);
STB_EXTERN void stb__arr_deleten_(void **p, int size, int loc, int n  STB__PARAMS);
STB_EXTERN void stb__arr_insertn_(void **p, int size, int loc, int n  STB__PARAMS);

#define stb_arr_free(p)            stb_arr_free_((void **) &(p))
#define stb__arr_copy              stb__arr_copy_

#ifndef STB_MALLOC_WRAPPER
  #define stb__arr_setsize         stb__arr_setsize_
  #define stb__arr_setlen          stb__arr_setlen_
  #define stb__arr_addlen          stb__arr_addlen_
  #define stb__arr_deleten         stb__arr_deleten_
  #define stb__arr_insertn         stb__arr_insertn_
#else
  #define stb__arr_addlen(p,s,n)    stb__arr_addlen_(p,s,n,__FILE__,__LINE__)
  #define stb__arr_setlen(p,s,n)    stb__arr_setlen_(p,s,n,__FILE__,__LINE__)
  #define stb__arr_setsize(p,s,n)   stb__arr_setsize_(p,s,n,__FILE__,__LINE__)
  #define stb__arr_deleten(p,s,i,n) stb__arr_deleten_(p,s,i,n,__FILE__,__LINE__)
  #define stb__arr_insertn(p,s,i,n) stb__arr_insertn_(p,s,i,n,__FILE__,__LINE__)
#endif

#ifdef STB_DEFINE
static void *stb__arr_context;

void *stb_arr_malloc_parent(void *p)
{
   void *q = stb__arr_context;
   stb__arr_context = p;
   return q;
}

void stb_arr_malloc(void **target, void *context)
{
   stb__arr *q = (stb__arr *) stb_malloc(context, sizeof(*q));
   q->len = q->limit = 0;
   q->stb_malloc = 1;
   q->signature = stb_arr_signature;
   *target = (void *) (q+1);
}

static void * stb__arr_malloc(int size)
{
   if (stb__arr_context)
      return stb_malloc(stb__arr_context, size);
   return malloc(size);
}

void * stb__arr_copy_(void *p, int elem_size)
{
   stb__arr *q;
   if (p == NULL) return p;
   q = (stb__arr *) stb__arr_malloc(sizeof(*q) + elem_size * stb_arrhead2(p)->limit);
   stb_arr_check2(p);
   memcpy(q, stb_arrhead2(p), sizeof(*q) + elem_size * stb_arrhead2(p)->len);
   q->stb_malloc = !!stb__arr_context;
   return q+1;
}

void stb_arr_free_(void **pp)
{
   void *p = *pp;
   stb_arr_check2(p);
   if (p) {
      stb__arr *q = stb_arrhead2(p);
      if (q->stb_malloc)
         stb_free(q);
      else
         free(q);
   }
   *pp = NULL;
}

static void stb__arrsize_(void **pp, int size, int limit, int len  STB__PARAMS)
{
   void *p = *pp;
   stb__arr *a;
   stb_arr_check2(p);
   if (p == NULL) {
      if (len == 0 && size == 0) return;
      a = (stb__arr *) stb__arr_malloc(sizeof(*a) + size*limit);
      a->limit = limit;
      a->len   = len;
      a->stb_malloc = !!stb__arr_context;
      a->signature = stb_arr_signature;
   } else {
      a = stb_arrhead2(p);
      a->len = len;
      if (a->limit < limit) {
         void *p;
         if (a->limit >= 4 && limit < a->limit * 2)
            limit = a->limit * 2;
         if (a->stb_malloc)
            p = stb_realloc(a, sizeof(*a) + limit*size);
         else
            #ifdef STB_MALLOC_WRAPPER
            p = stb__realloc(a, sizeof(*a) + limit*size, file, line);
            #else
            p = realloc(a, sizeof(*a) + limit*size);
            #endif
         if (p) {
            a = (stb__arr *) p;
            a->limit = limit;
         } else {
            // throw an error!
         }
      }
   }
   a->len   = stb_min(a->len, a->limit);
   *pp = a+1;
}

void stb__arr_setsize_(void **pp, int size, int limit  STB__PARAMS)
{
   void *p = *pp;
   stb_arr_check2(p);
   stb__arrsize_(pp, size, limit, stb_arr_len2(p)  STB__ARGS);
}

void stb__arr_setlen_(void **pp, int size, int newlen  STB__PARAMS)
{
   void *p = *pp;
   stb_arr_check2(p);
   if (stb_arrcurmax2(p) < newlen || p == NULL) {
      stb__arrsize_(pp, size, newlen, newlen  STB__ARGS);
   } else {
      stb_arrhead2(p)->len = newlen;
   }
}

void stb__arr_addlen_(void **p, int size, int addlen  STB__PARAMS)
{
   stb__arr_setlen_(p, size, stb_arr_len2(*p) + addlen  STB__ARGS);
}

void stb__arr_insertn_(void **pp, int size, int i, int n  STB__PARAMS)
{
   void *p = *pp;
   if (n) {
      int z;

      if (p == NULL) {
         stb__arr_addlen_(pp, size, n  STB__ARGS);
         return;
      }

      z = stb_arr_len2(p);
      stb__arr_addlen_(&p, size, i  STB__ARGS);
      memmove((char *) p + (i+n)*size, (char *) p + i*size, size * (z-i));
   }
   *pp = p;
}

void stb__arr_deleten_(void **pp, int size, int i, int n  STB__PARAMS)
{
   void *p = *pp;
   if (n) {
      memmove((char *) p + i*size, (char *) p + (i+n)*size, size * (stb_arr_len2(p)-i));
      stb_arrhead2(p)->len -= n;
   }
   *pp = p;
}

#endif

//////////////////////////////////////////////////////////////////////////////
//
//                               Hashing
//
//      typical use for this is to make a power-of-two hash table.
//
//      let N = size of table (2^n)
//      let H = stb_hash(str)
//      let S = stb_rehash(H) | 1
//
//      then hash probe sequence P(i) for i=0..N-1
//         P(i) = (H + S*i) & (N-1)
//
//      the idea is that H has 32 bits of hash information, but the
//      table has only, say, 2^20 entries so only uses 20 of the bits.
//      then by rehashing the original H we get 2^12 different probe
//      sequences for a given initial probe location. (So it's optimal
//      for 64K tables and its optimality decreases past that.)
//
//      ok, so I've added something that generates _two separate_
//      32-bit hashes simultaneously which should scale better to
//      very large tables.


STB_EXTERN unsigned int stb_hash(const char *str);
STB_EXTERN unsigned int stb_hashptr(void *p);
STB_EXTERN unsigned int stb_hashlen(char *str, int len);
STB_EXTERN unsigned int stb_rehash_improved(unsigned int v);
STB_EXTERN unsigned int stb_hash_fast(void *p, int len);
STB_EXTERN unsigned int stb_hash2(char *str, unsigned int *hash2_ptr);
STB_EXTERN unsigned int stb_hash_number(unsigned int hash);

#define stb_rehash(x)  ((x) + ((x) >> 6) + ((x) >> 19))

#ifdef STB_DEFINE
unsigned int stb_hash(const char *str)
{
   unsigned int hash = 0;
   while (*str)
      hash = (hash << 7) + (hash >> 25) + *str++;
   return hash + (hash >> 16);
}

unsigned int stb_hashlen(char *str, int len)
{
   unsigned int hash = 0;
   while (len-- > 0 && *str)
      hash = (hash << 7) + (hash >> 25) + *str++;
   return hash + (hash >> 16);
}

unsigned int stb_hashptr(void *p)
{
   unsigned int x = (unsigned int) (size_t) p;

   // typically lacking in low bits and high bits
   x = stb_rehash(x);
   x += x << 16;

   // pearson's shuffle
   x ^= x << 3;
   x += x >> 5;
   x ^= x << 2;
   x += x >> 15;
   x ^= x << 10;
   return stb_rehash(x);
}

unsigned int stb_rehash_improved(unsigned int v)
{
   return stb_hashptr((void *)(size_t) v);
}

unsigned int stb_hash2(char *str, unsigned int *hash2_ptr)
{
   unsigned int hash1 = 0x3141592c;
   unsigned int hash2 = 0x77f044ed;
   while (*str) {
      hash1 = (hash1 << 7) + (hash1 >> 25) + *str;
      hash2 = (hash2 << 11) + (hash2 >> 21) + *str;
      ++str;
   }
   *hash2_ptr = hash2 + (hash1 >> 16);
   return       hash1 + (hash2 >> 16);
}

// Paul Hsieh hash
#define stb__get16_slow(p) ((p)[0] + ((p)[1] << 8))
#if defined(_MSC_VER)
   #define stb__get16(p) (*((unsigned short *) (p)))
#else
   #define stb__get16(p) stb__get16_slow(p)
#endif

unsigned int stb_hash_fast(void *p, int len)
{
   unsigned char *q = (unsigned char *) p;
   unsigned int hash = len;

   if (len <= 0 || q == NULL) return 0;

   /* Main loop */
   if (((size_t) q & 1) == 0) {
      for (;len > 3; len -= 4) {
         unsigned int val;
         hash +=  stb__get16(q);
         val   = (stb__get16(q+2) << 11);
         hash  = (hash << 16) ^ hash ^ val;
         q    += 4;
         hash += hash >> 11;
      }
   } else {
      for (;len > 3; len -= 4) {
         unsigned int val;
         hash +=  stb__get16_slow(q);
         val   = (stb__get16_slow(q+2) << 11);
         hash  = (hash << 16) ^ hash ^ val;
         q    += 4;
         hash += hash >> 11;
      }
   }

   /* Handle end cases */
   switch (len) {
      case 3: hash += stb__get16_slow(q);
              hash ^= hash << 16;
              hash ^= q[2] << 18;
              hash += hash >> 11;
              break;
      case 2: hash += stb__get16_slow(q);
              hash ^= hash << 11;
              hash += hash >> 17;
              break;
      case 1: hash += q[0];
              hash ^= hash << 10;
              hash += hash >> 1;
              break;
      case 0: break;
   }

   /* Force "avalanching" of final 127 bits */
   hash ^= hash << 3;
   hash += hash >> 5;
   hash ^= hash << 4;
   hash += hash >> 17;
   hash ^= hash << 25;
   hash += hash >> 6;

   return hash;
}

unsigned int stb_hash_number(unsigned int hash)
{
   hash ^= hash << 3;
   hash += hash >> 5;
   hash ^= hash << 4;
   hash += hash >> 17;
   hash ^= hash << 25;
   hash += hash >> 6;
   return hash;
}

#endif

//////////////////////////////////////////////////////////////////////////////
//
//                     Perfect hashing for ints/pointers
//
//   This is mainly useful for making faster pointer-indexed tables
//   that don't change frequently. E.g. for stb_ischar().
//

typedef struct
{
   stb_uint32  addend;
   stb_uint    multiplicand;
   stb_uint    b_mask;
   stb_uint8   small_bmap[16];
   stb_uint16  *large_bmap;

   stb_uint table_mask;
   stb_uint32 *table;
} stb_perfect;

STB_EXTERN int stb_perfect_create(stb_perfect *,unsigned int*,int n);
STB_EXTERN void stb_perfect_destroy(stb_perfect *);
STB_EXTERN int stb_perfect_hash(stb_perfect *, unsigned int x);
extern int stb_perfect_hash_max_failures;

#ifdef STB_DEFINE

int stb_perfect_hash_max_failures;

int stb_perfect_hash(stb_perfect *p, unsigned int x)
{
   stb_uint m = x * p->multiplicand;
   stb_uint y = x >> 16;
   stb_uint bv = (m >> 24) + y;
   stb_uint av = (m + y) >> 12;
   if (p->table == NULL) return -1;  // uninitialized table fails
   bv &= p->b_mask;
   av &= p->table_mask;
   if (p->large_bmap)
      av ^= p->large_bmap[bv];
   else
      av ^= p->small_bmap[bv];
   return p->table[av] == x ? av : -1;
}

static void stb__perfect_prehash(stb_perfect *p, stb_uint x, stb_uint16 *a, stb_uint16 *b)
{
   stb_uint m = x * p->multiplicand;
   stb_uint y = x >> 16;
   stb_uint bv = (m >> 24) + y;
   stb_uint av = (m + y) >> 12;
   bv &= p->b_mask;
   av &= p->table_mask;
   *b = bv;
   *a = av;
}

static unsigned long stb__perfect_rand(void)
{
   static unsigned long stb__rand;
   stb__rand = stb__rand * 2147001325 + 715136305;
   return 0x31415926 ^ ((stb__rand >> 16) + (stb__rand << 16));
}

typedef struct {
   unsigned short count;
   unsigned short b;
   unsigned short map;
   unsigned short *entries;
} stb__slot;

static int stb__slot_compare(const void *p, const void *q)
{
   stb__slot *a = (stb__slot *) p;
   stb__slot *b = (stb__slot *) q;
   return a->count > b->count ? -1 : a->count < b->count;  // sort large to small
}

int stb_perfect_create(stb_perfect *p, unsigned int *v, int n)
{
   unsigned int buffer1[64], buffer2[64], buffer3[64], buffer4[64], buffer5[32];
   unsigned short *as = (unsigned short *) stb_temp(buffer1, sizeof(*v)*n);
   unsigned short *bs = (unsigned short *) stb_temp(buffer2, sizeof(*v)*n);
   unsigned short *entries = (unsigned short *) stb_temp(buffer4, sizeof(*entries) * n);
   int size = 1 << stb_log2_ceil(n), bsize=8;
   int failure = 0,i,j,k;

   assert(n <= 32768);
   p->large_bmap = NULL;

   for(;;) {
      stb__slot *bcount = (stb__slot *) stb_temp(buffer3, sizeof(*bcount) * bsize);
      unsigned short *bloc = (unsigned short *) stb_temp(buffer5, sizeof(*bloc) * bsize);
      unsigned short *e;
      int bad=0;

      p->addend = stb__perfect_rand();
      p->multiplicand = stb__perfect_rand() | 1;
      p->table_mask = size-1;
      p->b_mask = bsize-1;
      p->table = (stb_uint32 *) malloc(size * sizeof(*p->table));

      for (i=0; i < bsize; ++i) {
         bcount[i].b     = i;
         bcount[i].count = 0;
         bcount[i].map   = 0;
      }
      for (i=0; i < n; ++i) {
         stb__perfect_prehash(p, v[i], as+i, bs+i);
         ++bcount[bs[i]].count;
      }
      qsort(bcount, bsize, sizeof(*bcount), stb__slot_compare);
      e = entries; // now setup up their entries index
      for (i=0; i < bsize; ++i) {
         bcount[i].entries = e;
         e += bcount[i].count;
         bcount[i].count = 0;
         bloc[bcount[i].b] = i;
      }
      // now fill them out
      for (i=0; i < n; ++i) {
         int b = bs[i];
         int w = bloc[b];
         bcount[w].entries[bcount[w].count++] = i;
      }
      stb_tempfree(buffer5,bloc);
      // verify
      for (i=0; i < bsize; ++i)
         for (j=0; j < bcount[i].count; ++j)
            assert(bs[bcount[i].entries[j]] == bcount[i].b);
      memset(p->table, 0, size*sizeof(*p->table));

      // check if any b has duplicate a
      for (i=0; i < bsize; ++i) {
         if (bcount[i].count > 1) {
            for (j=0; j < bcount[i].count; ++j) {
               if (p->table[as[bcount[i].entries[j]]])
                  bad = 1;
               p->table[as[bcount[i].entries[j]]] = 1;
            }
            for (j=0; j < bcount[i].count; ++j) {
               p->table[as[bcount[i].entries[j]]] = 0;
            }
            if (bad) break;
         }
      }

      if (!bad) {
         // go through the bs and populate the table, first fit
         for (i=0; i < bsize; ++i) {
            if (bcount[i].count) {
               // go through the candidate table[b] values
               for (j=0; j < size; ++j) {
                  // go through the a values and see if they fit
                  for (k=0; k < bcount[i].count; ++k) {
                     int a = as[bcount[i].entries[k]];
                     if (p->table[(a^j)&p->table_mask]) {
                        break; // fails
                     }
                  }
                  // if succeeded, accept
                  if (k == bcount[i].count) {
                     bcount[i].map = j;
                     for (k=0; k < bcount[i].count; ++k) {
                        int a = as[bcount[i].entries[k]];
                        p->table[(a^j)&p->table_mask] = 1;
                     }
                     break;
                  }
               }
               if (j == size)
                  break; // no match for i'th entry, so break out in failure
            }
         }
         if (i == bsize) {
            // success... fill out map
            if (bsize <= 16 && size <= 256) {
               p->large_bmap = NULL;
               for (i=0; i < bsize; ++i)
                  p->small_bmap[bcount[i].b] = (stb_uint8) bcount[i].map;
            } else {
               p->large_bmap = (unsigned short *) malloc(sizeof(*p->large_bmap) * bsize);
               for (i=0; i < bsize; ++i)
                  p->large_bmap[bcount[i].b] = bcount[i].map;
            }

            // initialize table to v[0], so empty slots will fail
            for (i=0; i < size; ++i)
               p->table[i] = v[0];

            for (i=0; i < n; ++i)
               if (p->large_bmap)
                  p->table[as[i] ^ p->large_bmap[bs[i]]] = v[i];
               else
                  p->table[as[i] ^ p->small_bmap[bs[i]]] = v[i];

            // and now validate that none of them collided
            for (i=0; i < n; ++i)
               assert(stb_perfect_hash(p, v[i]) >= 0);

            stb_tempfree(buffer3, bcount);
            break;
         }
      }
      free(p->table);
      p->table = NULL;
      stb_tempfree(buffer3, bcount);

      ++failure;
      if (failure >= 4 && bsize < size) bsize *= 2;
      if (failure >= 8 && (failure & 3) == 0 && size < 4*n) {
         size *= 2;
         bsize *= 2;
      }
      if (failure == 6) {
         // make sure the input data is unique, so we don't infinite loop
         unsigned int *data = (unsigned int *) stb_temp(buffer3, n * sizeof(*data));
         memcpy(data, v, sizeof(*data) * n);
         qsort(data, n, sizeof(*data), stb_intcmp(0));
         for (i=1; i < n; ++i) {
            if (data[i] == data[i-1])
               size = 0; // size is return value, so 0 it
         }
         stb_tempfree(buffer3, data);
         if (!size) break;
      }
   }

   if (failure > stb_perfect_hash_max_failures)
      stb_perfect_hash_max_failures = failure;

   stb_tempfree(buffer1, as);
   stb_tempfree(buffer2, bs);
   stb_tempfree(buffer4, entries);

   return size;
}

void stb_perfect_destroy(stb_perfect *p)
{
   if (p->large_bmap) free(p->large_bmap);
   if (p->table     ) free(p->table);
   p->large_bmap = NULL;
   p->table      = NULL;
   p->b_mask     = 0;
   p->table_mask = 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
//                     Instantiated data structures
//
// This is an attempt to implement a templated data structure.
//
// Hash table: call stb_define_hash(TYPE,N,KEY,K1,K2,HASH,VALUE)
//     TYPE     -- will define a structure type containing the hash table
//     N        -- the name, will prefix functions named:
//                        N create
//                        N destroy
//                        N get
//                        N set, N add, N update,
//                        N remove
//     KEY      -- the type of the key. 'x == y' must be valid
//       K1,K2  -- keys never used by the app, used as flags in the hashtable
//       HASH   -- a piece of code ending with 'return' that hashes key 'k'
//     VALUE    -- the type of the value. 'x = y' must be valid
//
//  Note that stb_define_hash_base can be used to define more sophisticated
//  hash tables, e.g. those that make copies of the key or use special
//  comparisons (e.g. strcmp).

#define STB_(prefix,name)     stb__##prefix##name
#define STB__(prefix,name)    prefix##name
#define STB__use(x)           x
#define STB__skip(x)

#define stb_declare_hash(PREFIX,TYPE,N,KEY,VALUE) \
   typedef struct stb__st_##TYPE TYPE;\
   PREFIX int STB__(N, init)(TYPE *h, int count);\
   PREFIX int STB__(N, memory_usage)(TYPE *h);\
   PREFIX TYPE * STB__(N, create)(void);\
   PREFIX TYPE * STB__(N, copy)(TYPE *h);\
   PREFIX void STB__(N, destroy)(TYPE *h);\
   PREFIX int STB__(N,get_flag)(TYPE *a, KEY k, VALUE *v);\
   PREFIX VALUE STB__(N,get)(TYPE *a, KEY k);\
   PREFIX int STB__(N, set)(TYPE *a, KEY k, VALUE v);\
   PREFIX int STB__(N, add)(TYPE *a, KEY k, VALUE v);\
   PREFIX int STB__(N, update)(TYPE*a,KEY k,VALUE v);\
   PREFIX int STB__(N, remove)(TYPE *a, KEY k, VALUE *v);

#define STB_nocopy(x)        (x)
#define STB_nodelete(x)      0
#define STB_nofields         
#define STB_nonullvalue(x)
#define STB_nullvalue(x)     x
#define STB_safecompare(x)   x
#define STB_nosafe(x)
#define STB_noprefix

#ifdef __GNUC__
#define STB__nogcc(x)
#else
#define STB__nogcc(x)  x
#endif

#define stb_define_hash_base(PREFIX,TYPE,FIELDS,N,NC,LOAD_FACTOR,             \
                             KEY,EMPTY,DEL,COPY,DISPOSE,SAFE,                 \
                             VCOMPARE,CCOMPARE,HASH,                          \
                             VALUE,HASVNULL,VNULL)                            \
                                                                              \
typedef struct                                                                \
{                                                                             \
   KEY   k;                                                                   \
   VALUE v;                                                                   \
} STB_(N,_hashpair);                                                          \
                                                                              \
STB__nogcc( typedef struct stb__st_##TYPE TYPE;  )                            \
struct stb__st_##TYPE {                                                       \
   FIELDS                                                                     \
   STB_(N,_hashpair) *table;                                                  \
   unsigned int mask;                                                         \
   int count, limit;                                                          \
   int deleted;                                                               \
                                                                              \
   int delete_threshhold;                                                     \
   int grow_threshhold;                                                       \
   int shrink_threshhold;                                                     \
   unsigned char alloced, has_empty, has_del;                                 \
   VALUE ev; VALUE dv;                                                        \
};                                                                            \
                                                                              \
static unsigned int STB_(N, hash)(KEY k)                                      \
{                                                                             \
   HASH                                                                       \
}                                                                             \
                                                                              \
PREFIX int STB__(N, init)(TYPE *h, int count)                                        \
{                                                                             \
   int i;                                                                     \
   if (count < 4) count = 4;                                                  \
   h->limit = count;                                                          \
   h->count = 0;                                                              \
   h->mask  = count-1;                                                        \
   h->deleted = 0;                                                            \
   h->grow_threshhold = (int) (count * LOAD_FACTOR);                          \
   h->has_empty = h->has_del = 0;                                             \
   h->alloced = 0;                                                            \
   if (count <= 64)                                                           \
      h->shrink_threshhold = 0;                                               \
   else                                                                       \
      h->shrink_threshhold = (int) (count * (LOAD_FACTOR/2.25));              \
   h->delete_threshhold = (int) (count * (1-LOAD_FACTOR)/2);                  \
   h->table = (STB_(N,_hashpair)*) malloc(sizeof(h->table[0]) * count);       \
   if (h->table == NULL) return 0;                                            \
   /* ideally this gets turned into a memset32 automatically */               \
   for (i=0; i < count; ++i)                                                  \
      h->table[i].k = EMPTY;                                                  \
   return 1;                                                                  \
}                                                                             \
                                                                              \
PREFIX int STB__(N, memory_usage)(TYPE *h)                                           \
{                                                                             \
   return sizeof(*h) + h->limit * sizeof(h->table[0]);                        \
}                                                                             \
                                                                              \
PREFIX TYPE * STB__(N, create)(void)                                                 \
{                                                                             \
   TYPE *h = (TYPE *) malloc(sizeof(*h));                                     \
   if (h) {                                                                   \
      if (STB__(N, init)(h, 16))                                              \
         h->alloced = 1;                                                      \
      else { free(h); h=NULL; }                                               \
   }                                                                          \
   return h;                                                                  \
}                                                                             \
                                                                              \
PREFIX void STB__(N, destroy)(TYPE *a)                                               \
{                                                                             \
   int i;                                                                     \
   for (i=0; i < a->limit; ++i)                                               \
      if (!CCOMPARE(a->table[i].k,EMPTY) && !CCOMPARE(a->table[i].k, DEL))    \
         DISPOSE(a->table[i].k);                                              \
   free(a->table);                                                            \
   if (a->alloced)                                                            \
      free(a);                                                                \
}                                                                             \
                                                                              \
static void STB_(N, rehash)(TYPE *a, int count);                              \
                                                                              \
PREFIX int STB__(N,get_flag)(TYPE *a, KEY k, VALUE *v)                               \
{                                                                             \
   unsigned int h = STB_(N, hash)(k);                                         \
   unsigned int n = h & a->mask, s;                                           \
   if (CCOMPARE(k,EMPTY)){ if (a->has_empty) *v = a->ev; return a->has_empty;}\
   if (CCOMPARE(k,DEL)) { if (a->has_del  ) *v = a->dv; return a->has_del;   }\
   if (CCOMPARE(a->table[n].k,EMPTY)) return 0;                               \
   SAFE(if (!CCOMPARE(a->table[n].k,DEL)))                                    \
   if (VCOMPARE(a->table[n].k,k)) { *v = a->table[n].v; return 1; }            \
   s = stb_rehash(h) | 1;                                                     \
   for(;;) {                                                                  \
      n = (n + s) & a->mask;                                                  \
      if (CCOMPARE(a->table[n].k,EMPTY)) return 0;                            \
      SAFE(if (CCOMPARE(a->table[n].k,DEL)) continue;)                        \
      if (VCOMPARE(a->table[n].k,k))                                           \
         { *v = a->table[n].v; return 1; }                                    \
   }                                                                          \
}                                                                             \
                                                                              \
HASVNULL(                                                                     \
   PREFIX VALUE STB__(N,get)(TYPE *a, KEY k)                                         \
   {                                                                          \
      VALUE v;                                                                \
      if (STB__(N,get_flag)(a,k,&v)) return v;                                \
      else                           return VNULL;                            \
   }                                                                          \
)                                                                             \
                                                                              \
PREFIX int STB__(N,getkey)(TYPE *a, KEY k, KEY *kout)                                \
{                                                                             \
   unsigned int h = STB_(N, hash)(k);                                         \
   unsigned int n = h & a->mask, s;                                           \
   if (CCOMPARE(k,EMPTY)||CCOMPARE(k,DEL)) return 0;                          \
   if (CCOMPARE(a->table[n].k,EMPTY)) return 0;                               \
   SAFE(if (!CCOMPARE(a->table[n].k,DEL)))                                    \
   if (VCOMPARE(a->table[n].k,k)) { *kout = a->table[n].k; return 1; }         \
   s = stb_rehash(h) | 1;                                                     \
   for(;;) {                                                                  \
      n = (n + s) & a->mask;                                                  \
      if (CCOMPARE(a->table[n].k,EMPTY)) return 0;                            \
      SAFE(if (CCOMPARE(a->table[n].k,DEL)) continue;)                        \
      if (VCOMPARE(a->table[n].k,k))                                          \
         { *kout = a->table[n].k; return 1; }                                 \
   }                                                                          \
}                                                                             \
                                                                              \
static int STB_(N,addset)(TYPE *a, KEY k, VALUE v,                            \
                             int allow_new, int allow_old, int copy)          \
{                                                                             \
   unsigned int h = STB_(N, hash)(k);                                         \
   unsigned int n = h & a->mask;                                              \
   int b = -1;                                                                \
   if (CCOMPARE(k,EMPTY)) {                                                   \
      if (a->has_empty ? allow_old : allow_new) {                             \
          n=a->has_empty; a->ev = v; a->has_empty = 1; return !n;             \
      } else return 0;                                                        \
   }                                                                          \
   if (CCOMPARE(k,DEL)) {                                                     \
      if (a->has_del ? allow_old : allow_new) {                               \
          n=a->has_del; a->dv = v; a->has_del = 1; return !n;                 \
      } else return 0;                                                        \
   }                                                                          \
   if (!CCOMPARE(a->table[n].k, EMPTY)) {                                     \
      unsigned int s;                                                         \
      if (CCOMPARE(a->table[n].k, DEL))                                       \
         b = n;                                                               \
      else if (VCOMPARE(a->table[n].k,k)) {                                   \
         if (allow_old)                                                       \
            a->table[n].v = v;                                                \
         return !allow_new;                                                   \
      }                                                                       \
      s = stb_rehash(h) | 1;                                                  \
      for(;;) {                                                               \
         n = (n + s) & a->mask;                                               \
         if (CCOMPARE(a->table[n].k, EMPTY)) break;                           \
         if (CCOMPARE(a->table[n].k, DEL)) {                                  \
            if (b < 0) b = n;                                                 \
         } else if (VCOMPARE(a->table[n].k,k)) {                              \
            if (allow_old)                                                    \
               a->table[n].v = v;                                             \
            return !allow_new;                                                \
         }                                                                    \
      }                                                                       \
   }                                                                          \
   if (!allow_new) return 0;                                                  \
   if (b < 0) b = n; else --a->deleted;                                       \
   a->table[b].k = copy ? COPY(k) : k;                                        \
   a->table[b].v = v;                                                         \
   ++a->count;                                                                \
   if (a->count > a->grow_threshhold)                                         \
      STB_(N,rehash)(a, a->limit*2);                                          \
   return 1;                                                                  \
}                                                                             \
                                                                              \
PREFIX int STB__(N, set)(TYPE *a, KEY k, VALUE v){return STB_(N,addset)(a,k,v,1,1,1);}\
PREFIX int STB__(N, add)(TYPE *a, KEY k, VALUE v){return STB_(N,addset)(a,k,v,1,0,1);}\
PREFIX int STB__(N, update)(TYPE*a,KEY k,VALUE v){return STB_(N,addset)(a,k,v,0,1,1);}\
                                                                              \
PREFIX int STB__(N, remove)(TYPE *a, KEY k, VALUE *v)                                \
{                                                                             \
   unsigned int h = STB_(N, hash)(k);                                         \
   unsigned int n = h & a->mask, s;                                           \
   if (CCOMPARE(k,EMPTY)) { if (a->has_empty) { if(v)*v = a->ev; a->has_empty=0; return 1; } return 0; } \
   if (CCOMPARE(k,DEL))   { if (a->has_del  ) { if(v)*v = a->dv; a->has_del  =0; return 1; } return 0; } \
   if (CCOMPARE(a->table[n].k,EMPTY)) return 0;                               \
   if (SAFE(CCOMPARE(a->table[n].k,DEL) || ) !VCOMPARE(a->table[n].k,k)) {     \
      s = stb_rehash(h) | 1;                                                  \
      for(;;) {                                                               \
         n = (n + s) & a->mask;                                               \
         if (CCOMPARE(a->table[n].k,EMPTY)) return 0;                         \
         SAFE(if (CCOMPARE(a->table[n].k, DEL)) continue;)                    \
         if (VCOMPARE(a->table[n].k,k)) break;                                 \
      }                                                                       \
   }                                                                          \
   DISPOSE(a->table[n].k);                                                    \
   a->table[n].k = DEL;                                                       \
   --a->count;                                                                \
   ++a->deleted;                                                              \
   if (v != NULL)                                                             \
      *v = a->table[n].v;                                                     \
   if (a->count < a->shrink_threshhold)                                       \
      STB_(N, rehash)(a, a->limit >> 1);                                      \
   else if (a->deleted > a->delete_threshhold)                                \
      STB_(N, rehash)(a, a->limit);                                           \
   return 1;                                                                  \
}                                                                             \
                                                                              \
PREFIX TYPE * STB__(NC, copy)(TYPE *a)                                        \
{                                                                             \
   int i;                                                                     \
   TYPE *h = (TYPE *) malloc(sizeof(*h));                                     \
   if (!h) return NULL;                                                       \
   if (!STB__(N, init)(h, a->limit)) { free(h); return NULL; }                \
   h->count = a->count;                                                       \
   h->deleted = a->deleted;                                                   \
   h->alloced = 1;                                                            \
   h->ev = a->ev; h->dv = a->dv;                                              \
   h->has_empty = a->has_empty; h->has_del = a->has_del;                      \
   memcpy(h->table, a->table, h->limit * sizeof(h->table[0]));                \
   for (i=0; i < a->limit; ++i)                                               \
      if (!CCOMPARE(h->table[i].k,EMPTY) && !CCOMPARE(h->table[i].k,DEL))     \
         h->table[i].k = COPY(h->table[i].k);                                 \
   return h;                                                                  \
}                                                                             \
                                                                              \
static void STB_(N, rehash)(TYPE *a, int count)                               \
{                                                                             \
   int i;                                                                     \
   TYPE b;                                                                    \
   STB__(N, init)(&b, count);                                                 \
   for (i=0; i < a->limit; ++i)                                               \
      if (!CCOMPARE(a->table[i].k,EMPTY) && !CCOMPARE(a->table[i].k,DEL))     \
         STB_(N,addset)(&b, a->table[i].k, a->table[i].v,1,1,0);              \
   free(a->table);                                                            \
   a->table = b.table;                                                        \
   a->mask = b.mask;                                                          \
   a->count = b.count;                                                        \
   a->limit = b.limit;                                                        \
   a->deleted = b.deleted;                                                    \
   a->delete_threshhold = b.delete_threshhold;                                \
   a->grow_threshhold = b.grow_threshhold;                                    \
   a->shrink_threshhold = b.shrink_threshhold;                                \
}

#define STB_equal(a,b)  ((a) == (b))

#define stb_define_hash(TYPE,N,KEY,EMPTY,DEL,HASH,VALUE)                      \
   stb_define_hash_base(STB_noprefix, TYPE,STB_nofields,N,NC,0.85f,              \
              KEY,EMPTY,DEL,STB_nocopy,STB_nodelete,STB_nosafe,               \
              STB_equal,STB_equal,HASH,                                       \
              VALUE,STB_nonullvalue,0)

#define stb_define_hash_vnull(TYPE,N,KEY,EMPTY,DEL,HASH,VALUE,VNULL)          \
   stb_define_hash_base(STB_noprefix, TYPE,STB_nofields,N,NC,0.85f,              \
              KEY,EMPTY,DEL,STB_nocopy,STB_nodelete,STB_nosafe,               \
              STB_equal,STB_equal,HASH,                                       \
              VALUE,STB_nullvalue,VNULL)


//////////////////////////////////////////////////////////////////////////////
//
//                        stb_ptrmap
//
// An stb_ptrmap data structure is an O(1) hash table between pointers. One
// application is to let you store "extra" data associated with pointers,
// which is why it was originally called stb_extra.

stb_declare_hash(STB_EXTERN, stb_ptrmap, stb_ptrmap_, void *, void *)
stb_declare_hash(STB_EXTERN, stb_idict, stb_idict_, stb_int32, stb_int32)

STB_EXTERN void        stb_ptrmap_delete(stb_ptrmap *e, void (*free_func)(void *));
STB_EXTERN stb_ptrmap *stb_ptrmap_new(void);

STB_EXTERN stb_idict * stb_idict_new_size(int size);
STB_EXTERN void        stb_idict_remove_all(stb_idict *e);

#ifdef STB_DEFINE

#define STB_EMPTY ((void *) 2)
#define STB_EDEL  ((void *) 6)

stb_define_hash_base(STB_noprefix,stb_ptrmap, STB_nofields, stb_ptrmap_,stb_ptrmap_,0.85f,
              void *,STB_EMPTY,STB_EDEL,STB_nocopy,STB_nodelete,STB_nosafe,
              STB_equal,STB_equal,return stb_hashptr(k);,
              void *,STB_nullvalue,NULL)

stb_ptrmap *stb_ptrmap_new(void)
{
   return stb_ptrmap_create();
}

void stb_ptrmap_delete(stb_ptrmap *e, void (*free_func)(void *))
{
   int i;
   if (free_func)
      for (i=0; i < e->limit; ++i)
         if (e->table[i].k != STB_EMPTY && e->table[i].k != STB_EDEL) {
            if (free_func == free)
               free(e->table[i].v); // allow STB_MALLOC_WRAPPER to operate
            else
               free_func(e->table[i].v);
         }
   stb_ptrmap_destroy(e);
}

// extra fields needed for stua_dict
#define STB_IEMPTY  ((int) 1)
#define STB_IDEL    ((int) 3)
stb_define_hash_base(STB_noprefix, stb_idict, short type; short gc; STB_nofields, stb_idict_,stb_idict_,0.85f,
              stb_int32,STB_IEMPTY,STB_IDEL,STB_nocopy,STB_nodelete,STB_nosafe,
              STB_equal,STB_equal,
              return stb_rehash_improved(k);,stb_int32,STB_nonullvalue,0)

stb_idict * stb_idict_new_size(int size)
{
   stb_idict *e = (stb_idict *) malloc(sizeof(*e));
   if (e) {
      if (!stb_is_pow2(size))
         size = 1 << stb_log2_ceil(size);
      stb_idict_init(e, size);
      e->alloced = 1;
   }
   return e;
}

void stb_idict_remove_all(stb_idict *e)
{
   int n;
   for (n=0; n < e->limit; ++n)
      e->table[n].k = STB_IEMPTY;
   e->has_empty = e->has_del = 0;
}
#endif


//////////////////////////////////////////////////////////////////////////////
//
//                             File Processing
//


#ifdef _MSC_VER
  #define stb_rename(x,y)   _wrename((const wchar_t *)stb__from_utf8(x), (const wchar_t *)stb__from_utf8_alt(y))
  #define stb_mktemp   _mktemp
#else
  #define stb_mktemp   mktemp
  #define stb_rename   rename
#endif

STB_EXTERN void     stb_fput_varlen64(FILE *f, stb_uint64 v);
STB_EXTERN stb_uint64  stb_fget_varlen64(FILE *f);
STB_EXTERN int      stb_size_varlen64(stb_uint64 v);


#define stb_filec    (char *) stb_file
#define stb_fileu    (unsigned char *) stb_file
STB_EXTERN void *  stb_file(const char *filename, size_t *length);
STB_EXTERN void *  stb_file_max(const char *filename, size_t *length);
STB_EXTERN size_t  stb_filelen(FILE *f);
STB_EXTERN int     stb_filewrite(const char *filename, const void *data, size_t length);
STB_EXTERN int     stb_filewritestr(const char *filename, const char *data);
STB_EXTERN char ** stb_stringfile(const char *filename, int *len);
STB_EXTERN char ** stb_stringfile_trimmed(const char *name, int *len, char comm);
STB_EXTERN char *  stb_fgets(char *buffer, int buflen, FILE *f);
STB_EXTERN char *  stb_fgets_malloc(FILE *f);
STB_EXTERN int     stb_fexists(const char *filename);
STB_EXTERN int     stb_fcmp(const char *s1, const char *s2);
STB_EXTERN int     stb_feq(const char *s1, const char *s2);
STB_EXTERN time_t  stb_ftimestamp(const char *filename);

STB_EXTERN int     stb_fullpath(char *abs, int abs_size, const char *rel);
STB_EXTERN FILE *  stb_fopen(const char *filename, const char *mode);
STB_EXTERN int     stb_fclose(FILE *f, int keep);

enum
{
   stb_keep_no = 0,
   stb_keep_yes = 1,
   stb_keep_if_different = 2,
};

STB_EXTERN int     stb_copyfile(const char *src, const char *dest);

STB_EXTERN void     stb_fput_varlen64(FILE *f, stb_uint64 v);
STB_EXTERN stb_uint64  stb_fget_varlen64(FILE *f);
STB_EXTERN int      stb_size_varlen64(stb_uint64 v);

STB_EXTERN void    stb_fwrite32(FILE *f, stb_uint32 datum);
STB_EXTERN void    stb_fput_varlen (FILE *f, int v);
STB_EXTERN void    stb_fput_varlenu(FILE *f, unsigned int v);
STB_EXTERN int     stb_fget_varlen (FILE *f);
STB_EXTERN stb_uint stb_fget_varlenu(FILE *f);
STB_EXTERN void    stb_fput_ranged (FILE *f, int v, int b, stb_uint n);
STB_EXTERN int     stb_fget_ranged (FILE *f, int b, stb_uint n);
STB_EXTERN int     stb_size_varlen (int v);
STB_EXTERN int     stb_size_varlenu(unsigned int v);
STB_EXTERN int     stb_size_ranged (int b, stb_uint n);

STB_EXTERN int     stb_fread(void *data, size_t len, size_t count, void *f);
STB_EXTERN int     stb_fwrite(void *data, size_t len, size_t count, void *f);


#ifdef STB_DEFINE

void stb_fwrite32(FILE *f, stb_uint32 x)
{
   fwrite(&x, 4, 1, f);
}

#if defined(_MSC_VER) || defined(__MINGW32__)
   #define stb__stat   _stat
#else
   #define stb__stat   stat
#endif

int stb_fexists(const char *filename)
{
   struct stb__stat buf;
   return stb__windows(
             _wstat((const wchar_t *)stb__from_utf8(filename), &buf),
               stat(filename,&buf)
          ) == 0;
}

time_t stb_ftimestamp(const char *filename)
{
   struct stb__stat buf;
   if (stb__windows(
             _wstat((const wchar_t *)stb__from_utf8(filename), &buf),
               stat(filename,&buf)
          ) == 0)
   {
      return buf.st_mtime;
   } else {
      return 0;
   }
}

size_t  stb_filelen(FILE *f)
{
   size_t len, pos;
   pos = ftell(f);
   fseek(f, 0, SEEK_END);
   len = ftell(f);
   fseek(f, pos, SEEK_SET);
   return len;
}

void *stb_file(const char *filename, size_t *length)
{
   FILE *f = stb__fopen(filename, "rb");
   char *buffer;
   size_t len, len2;
   if (!f) return NULL;
   len = stb_filelen(f);
   buffer = (char *) malloc(len+2); // nul + extra
   len2 = fread(buffer, 1, len, f);
   if (len2 == len) {
      if (length) *length = len;
      buffer[len] = 0;
   } else {
      free(buffer);
      buffer = NULL;
   }
   fclose(f);
   return buffer;
}

int stb_filewrite(const char *filename, const void *data, size_t length)
{
   FILE *f = stb_fopen(filename, "wb");
   if (f) {
      fwrite(data, 1, length, f);
      stb_fclose(f, stb_keep_if_different);
   }
   return f != NULL;
}

int stb_filewritestr(const char *filename, const char *data)
{
   return stb_filewrite(filename, data, strlen(data));
}

void *  stb_file_max(const char *filename, size_t *length)
{
   FILE *f = stb__fopen(filename, "rb");
   char *buffer;
   size_t len, maxlen;
   if (!f) return NULL;
   maxlen = *length;
   buffer = (char *) malloc(maxlen+1);
   len = fread(buffer, 1, maxlen, f);
   buffer[len] = 0;
   fclose(f);
   *length = len;
   return buffer;
}

char ** stb_stringfile(const char *filename, int *plen)
{
   FILE *f = stb__fopen(filename, "rb");
   char *buffer, **list=NULL, *s;
   size_t len, count, i;

   if (!f) return NULL;
   len = stb_filelen(f);
   buffer = (char *) malloc(len+1);
   len = fread(buffer, 1, len, f);
   buffer[len] = 0;
   fclose(f);

   // two passes through: first time count lines, second time set them
   for (i=0; i < 2; ++i) {
      s = buffer;
      if (i == 1)
         list[0] = s;
      count = 1;
      while (*s) {
         if (*s == '\n' || *s == '\r') {
            // detect if both cr & lf are together
            int crlf = (s[0] + s[1]) == ('\n' + '\r');
            if (i == 1) *s = 0;
            if (crlf) ++s;
            if (s[1]) {  // it's not over yet
               if (i == 1) list[count] = s+1;
               ++count;
            }
         }
         ++s;
      }
      if (i == 0) {
         list = (char **) malloc(sizeof(*list) * (count+1) + len+1);
         if (!list) return NULL;
         list[count] = 0;
         // recopy the file so there's just a single allocation to free
         memcpy(&list[count+1], buffer, len+1);
         free(buffer);
         buffer = (char *) &list[count+1];
         if (plen) *plen = count;
      }
   }
   return list;
}

char ** stb_stringfile_trimmed(const char *name, int *len, char comment)
{
   int i,n,o=0;
   char **s = stb_stringfile(name, &n);
   if (s == NULL) return NULL;
   for (i=0; i < n; ++i) {
      char *p = stb_skipwhite(s[i]);
      if (*p && *p != comment)
         s[o++] = p;
   }
   s[o] = NULL;
   if (len) *len = o;
   return s;
}

char * stb_fgets(char *buffer, int buflen, FILE *f)
{
   char *p;
   buffer[0] = 0;
   p = fgets(buffer, buflen, f);
   if (p) {
      int n = strlen(p)-1;
      if (n >= 0)
         if (p[n] == '\n')
            p[n] = 0;
   }
   return p;
}

char * stb_fgets_malloc(FILE *f)
{
   // avoid reallocing for small strings
   char quick_buffer[800];
   quick_buffer[sizeof(quick_buffer)-2] = 0;
   if (!fgets(quick_buffer, sizeof(quick_buffer), f))
      return NULL;

   if (quick_buffer[sizeof(quick_buffer)-2] == 0) {
      int n = strlen(quick_buffer);
      if (n > 0 && quick_buffer[n-1] == '\n')
         quick_buffer[n-1] = 0;
      return strdup(quick_buffer);
   } else {
      char *p;
      char *a = strdup(quick_buffer);
      int len = sizeof(quick_buffer)-1;

      while (!feof(f)) {
         if (a[len-1] == '\n') break;
         a = (char *) realloc(a, len*2);
         p = &a[len];
         p[len-2] = 0;
         if (!fgets(p, len, f))
            break;
         if (p[len-2] == 0) {
            len += strlen(p);
            break;
         }
         len = len + (len-1);
      }
      if (a[len-1] == '\n')
         a[len-1] = 0;
      return a;
   }
}

int stb_fullpath(char *abs, int abs_size, const char *rel)
{
   #ifdef _MSC_VER
   return _fullpath(abs, rel, abs_size) != NULL;
   #else
   if (rel[0] == '/' || rel[0] == '~') {
      if ((int) strlen(rel) >= abs_size)
         return 0;
      strcpy(abs,rel);
      return STB_TRUE;
   } else {
      int n;
      getcwd(abs, abs_size);
      n = strlen(abs);
      if (n+(int) strlen(rel)+2 <= abs_size) {
         abs[n] = '/';
         strcpy(abs+n+1, rel);
         return STB_TRUE;
      } else {
         return STB_FALSE;
      }
   }
   #endif
}

static int stb_fcmp_core(FILE *f, FILE *g)
{
   char buf1[1024],buf2[1024];
   int n1,n2, res=0;

   while (1) {
      n1 = fread(buf1, 1, sizeof(buf1), f);
      n2 = fread(buf2, 1, sizeof(buf2), g);
      res = memcmp(buf1,buf2,stb_min(n1,n2));
      if (res)
         break;
      if (n1 != n2) {
         res = n1 < n2 ? -1 : 1;
         break;
      }
      if (n1 == 0)
         break;
   }

   fclose(f);
   fclose(g);
   return res;
}

int stb_fcmp(const char *s1, const char *s2)
{
   FILE *f = stb__fopen(s1, "rb");
   FILE *g = stb__fopen(s2, "rb");

   if (f == NULL || g == NULL) {
      if (f) fclose(f);
      if (g) {
         fclose(g);
         return STB_TRUE;
      }
      return f != NULL;
   }

   return stb_fcmp_core(f,g);
}

int stb_feq(const char *s1, const char *s2)
{
   FILE *f = stb__fopen(s1, "rb");
   FILE *g = stb__fopen(s2, "rb");

   if (f == NULL || g == NULL) {
      if (f) fclose(f);
      if (g) fclose(g);
      return f == g;
   }

   // feq is faster because it shortcuts if they're different length
   if (stb_filelen(f) != stb_filelen(g)) {
      fclose(f);
      fclose(g);
      return 0;
   }

   return !stb_fcmp_core(f,g);
}

static stb_ptrmap *stb__files;

typedef struct
{
   char *temp_name;
   char *name;
   int   errors;
} stb__file_data;

FILE *  stb_fopen(const char *filename, const char *mode)
{
   FILE *f;
   char name_full[4096];
   char temp_full[sizeof(name_full) + 12];
   int p;
#ifdef _MSC_VER
   int j;
#endif
   if (mode[0] != 'w' && !strchr(mode, '+'))
      return stb__fopen(filename, mode);

   // save away the full path to the file so if the program
   // changes the cwd everything still works right! unix has
   // better ways to do this, but we have to work in windows
   name_full[0] = '\0'; // stb_fullpath reads name_full[0]
   if (stb_fullpath(name_full, sizeof(name_full), filename)==0)
      return 0;

   // try to generate a temporary file in the same directory
   p = strlen(name_full)-1;
   while (p > 0 && name_full[p] != '/' && name_full[p] != '\\'
                && name_full[p] != ':' && name_full[p] != '~')
      --p;
   ++p;

   memcpy(temp_full, name_full, p);

   #ifdef _MSC_VER
   // try multiple times to make a temp file... just in
   // case some other process makes the name first
   for (j=0; j < 32; ++j) {
      strcpy(temp_full+p, "stmpXXXXXX");
      if (stb_mktemp(temp_full) == NULL)
         return 0;

      f = fopen(temp_full, mode);
      if (f != NULL)
         break;
   }
   #else
   {
      strcpy(temp_full+p, "stmpXXXXXX");
      #ifdef __MINGW32__
         int fd = open(mktemp(temp_full), O_RDWR);
      #else
         int fd = mkstemp(temp_full);
      #endif
      if (fd == -1) return NULL;
      f = fdopen(fd, mode);
      if (f == NULL) {
         unlink(temp_full);
         close(fd);
         return NULL;
      }
   }
   #endif
   if (f != NULL) {
      stb__file_data *d = (stb__file_data *) malloc(sizeof(*d));
      if (!d) { assert(0);  /* NOTREACHED */fclose(f); return NULL; }
      if (stb__files == NULL) stb__files = stb_ptrmap_create();
      d->temp_name = strdup(temp_full);
      d->name      = strdup(name_full);
      d->errors    = 0;
      stb_ptrmap_add(stb__files, f, d);
      return f;
   }

   return NULL;
}

int     stb_fclose(FILE *f, int keep)
{
   stb__file_data *d;

   int ok = STB_FALSE;
   if (f == NULL) return 0;

   if (ferror(f))
      keep = stb_keep_no;

   fclose(f);

   if (stb__files && stb_ptrmap_remove(stb__files, f, (void **) &d)) {
      if (stb__files->count == 0) {
         stb_ptrmap_destroy(stb__files);
         stb__files = NULL;
      }
   } else
      return STB_TRUE; // not special

   if (keep == stb_keep_if_different) {
      // check if the files are identical
      if (stb_feq(d->name, d->temp_name)) {
         keep = stb_keep_no;
         ok = STB_TRUE;  // report success if no change
      }
   }

   if (keep != stb_keep_no) {
      if (stb_fexists(d->name) && remove(d->name)) {
         // failed to delete old, so don't keep new
         keep = stb_keep_no;
      } else {
         if (!stb_rename(d->temp_name, d->name))
            ok = STB_TRUE;
         else
            keep=stb_keep_no;
      }
   }

   if (keep == stb_keep_no)
      remove(d->temp_name);

   free(d->temp_name);
   free(d->name);
   free(d);

   return ok;
}

int stb_copyfile(const char *src, const char *dest)
{
   char raw_buffer[1024];
   char *buffer;
   int buf_size = 65536;

   FILE *f, *g;

   // if file already exists at destination, do nothing
   if (stb_feq(src, dest)) return STB_TRUE;

   // open file
   f = stb__fopen(src, "rb");
   if (f == NULL) return STB_FALSE;

   // open file for writing
   g = stb__fopen(dest, "wb");
   if (g == NULL) {
      fclose(f);
      return STB_FALSE;
   }

   buffer = (char *) malloc(buf_size);
   if (buffer == NULL) {
      buffer = raw_buffer;
      buf_size = sizeof(raw_buffer);
   }

   while (!feof(f)) {
      int n = fread(buffer, 1, buf_size, f);
      if (n != 0)
         fwrite(buffer, 1, n, g);
   }

   fclose(f);
   if (buffer != raw_buffer)
      free(buffer);

   fclose(g);
   return STB_TRUE;
}

// varlen:
//    v' = (v >> 31) + (v < 0 ? ~v : v)<<1;  // small abs(v) => small v'
// output v as big endian v'+k for v' <= k:
//   1 byte :  v' <= 0x00000080          (  -64 <= v <   64)   7 bits
//   2 bytes:  v' <= 0x00004000          (-8192 <= v < 8192)  14 bits
//   3 bytes:  v' <= 0x00200000                               21 bits
//   4 bytes:  v' <= 0x10000000                               28 bits
// the number of most significant 1-bits in the first byte
// equals the number of bytes after the first

#define stb__varlen_xform(v)     (v<0 ? (~v << 1)+1 : (v << 1))

int stb_size_varlen(int v) { return stb_size_varlenu(stb__varlen_xform(v)); }
int stb_size_varlenu(unsigned int v)
{
   if (v < 0x00000080) return 1;
   if (v < 0x00004000) return 2;
   if (v < 0x00200000) return 3;
   if (v < 0x10000000) return 4;
   return 5;
}

void    stb_fput_varlen(FILE *f, int v) { stb_fput_varlenu(f, stb__varlen_xform(v)); }

void    stb_fput_varlenu(FILE *f, unsigned int z)
{
   if (z >= 0x10000000) fputc(0xF0,f);
   if (z >= 0x00200000) fputc((z < 0x10000000 ? 0xE0 : 0)+(z>>24),f);
   if (z >= 0x00004000) fputc((z < 0x00200000 ? 0xC0 : 0)+(z>>16),f);
   if (z >= 0x00000080) fputc((z < 0x00004000 ? 0x80 : 0)+(z>> 8),f);
   fputc(z,f);
}

#define stb_fgetc(f)    ((unsigned char) fgetc(f))

int     stb_fget_varlen(FILE *f)
{
   unsigned int z = stb_fget_varlenu(f);
   return (z & 1) ? ~(z>>1) : (z>>1);
}

unsigned int stb_fget_varlenu(FILE *f)
{
   unsigned int z;
   unsigned char d;
   d = stb_fgetc(f);

   if (d >= 0x80) {
      if (d >= 0xc0) {
         if (d >= 0xe0) {
            if (d == 0xf0) z = stb_fgetc(f) << 24;
            else           z = (d - 0xe0) << 24;
            z += stb_fgetc(f) << 16;
         }
         else
            z = (d - 0xc0) << 16;
         z += stb_fgetc(f) << 8;
      } else
         z = (d - 0x80) <<  8;
      z += stb_fgetc(f);
   } else
      z = d;
   return z;
}

stb_uint64   stb_fget_varlen64(FILE *f)
{
   stb_uint64 z;
   unsigned char d;
   d = stb_fgetc(f);

   if (d >= 0x80) {
      if (d >= 0xc0) {
         if (d >= 0xe0) {
            if (d >= 0xf0) {
               if (d >= 0xf8) {
                  if (d >= 0xfc) {
                     if (d >= 0xfe) {
                        if (d >= 0xff)
                           z = (stb_uint64) stb_fgetc(f) << 56;
                        else
                           z = (stb_uint64) (d - 0xfe) << 56;
                        z |= (stb_uint64) stb_fgetc(f) << 48;
                     } else z = (stb_uint64) (d - 0xfc) << 48;
                     z |= (stb_uint64) stb_fgetc(f) << 40;
                  } else z = (stb_uint64) (d - 0xf8) << 40;
                  z |= (stb_uint64) stb_fgetc(f) << 32;
               } else z = (stb_uint64) (d - 0xf0) << 32;
               z |= (stb_uint) stb_fgetc(f) << 24;
            } else z = (stb_uint) (d - 0xe0) << 24;
            z |= (stb_uint) stb_fgetc(f) << 16;
         } else z = (stb_uint) (d - 0xc0) << 16;
         z |= (stb_uint) stb_fgetc(f) << 8;
      } else z = (stb_uint) (d - 0x80) << 8;
      z |= stb_fgetc(f);
   } else
      z = d;

   return (z & 1) ? ~(z >> 1) : (z >> 1);
}

int stb_size_varlen64(stb_uint64 v)
{
   if (v < 0x00000080) return 1;
   if (v < 0x00004000) return 2;
   if (v < 0x00200000) return 3;
   if (v < 0x10000000) return 4;
   if (v < STB_IMM_UINT64(0x0000000800000000)) return 5;
   if (v < STB_IMM_UINT64(0x0000040000000000)) return 6;
   if (v < STB_IMM_UINT64(0x0002000000000000)) return 7;
   if (v < STB_IMM_UINT64(0x0100000000000000)) return 8; 
   return 9;
}

void    stb_fput_varlen64(FILE *f, stb_uint64 v)
{
   stb_uint64 z = stb__varlen_xform(v);
   int first=1;
   if (z >= STB_IMM_UINT64(0x100000000000000)) {
      fputc(0xff,f);
      first=0;
   }
   if (z >= STB_IMM_UINT64(0x02000000000000)) fputc((first ? 0xFE : 0)+(char)(z>>56),f), first=0;
   if (z >= STB_IMM_UINT64(0x00040000000000)) fputc((first ? 0xFC : 0)+(char)(z>>48),f), first=0;
   if (z >= STB_IMM_UINT64(0x00000800000000)) fputc((first ? 0xF8 : 0)+(char)(z>>40),f), first=0;
   if (z >= STB_IMM_UINT64(0x00000010000000)) fputc((first ? 0xF0 : 0)+(char)(z>>32),f), first=0;
   if (z >= STB_IMM_UINT64(0x00000000200000)) fputc((first ? 0xE0 : 0)+(char)(z>>24),f), first=0;
   if (z >= STB_IMM_UINT64(0x00000000004000)) fputc((first ? 0xC0 : 0)+(char)(z>>16),f), first=0;
   if (z >= STB_IMM_UINT64(0x00000000000080)) fputc((first ? 0x80 : 0)+(char)(z>> 8),f), first=0; 
   fputc((char)z,f);
}

void    stb_fput_ranged(FILE *f, int v, int b, stb_uint n)
{
   v -= b;
   if (n <= (1 << 31))
      assert((stb_uint) v < n);
   if (n > (1 << 24)) fputc(v >> 24, f);
   if (n > (1 << 16)) fputc(v >> 16, f);
   if (n > (1 <<  8)) fputc(v >>  8, f);
   fputc(v,f);
}

int     stb_fget_ranged(FILE *f, int b, stb_uint n)
{
   unsigned int v=0;
   if (n > (1 << 24)) v += stb_fgetc(f) << 24;
   if (n > (1 << 16)) v += stb_fgetc(f) << 16;
   if (n > (1 <<  8)) v += stb_fgetc(f) <<  8;
   v += stb_fgetc(f);
   return b+v;
}

int     stb_size_ranged(int b, stb_uint n)
{
   if (n > (1 << 24)) return 4;
   if (n > (1 << 16)) return 3;
   if (n > (1 <<  8)) return 2;
   return 1;
}

void stb_fput_string(FILE *f, char *s)
{
   int len = strlen(s);
   stb_fput_varlenu(f, len);
   fwrite(s, 1, len, f);
}

// inverse of the above algorithm
char *stb_fget_string(FILE *f, void *p)
{
   char *s;
   int len = stb_fget_varlenu(f);
   if (len > 4096) return NULL;
   s = p ? stb_malloc_string(p, len+1) : (char *) malloc(len+1);
   fread(s, 1, len, f);
   s[len] = 0;
   return s;
}

char *stb_strdup(char *str, void *pool)
{
   int len = strlen(str);
   char *p = stb_malloc_string(pool, len+1);
   strcpy(p, str);
   return p;
}

// strip the trailing '/' or '\\' from a directory so we can refer to it
// as a file for _stat()
char *stb_strip_final_slash(char *t)
{
   if (t[0]) {
      char *z = t + strlen(t) - 1;
      // *z is the last character
      if (*z == '\\' || *z == '/')
         if (z != t+2 || t[1] != ':') // but don't strip it if it's e.g. "c:/"
            *z = 0;
      if (*z == '\\')
         *z = '/'; // canonicalize to make sure it matches db
   }
   return t;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
//                 Portable directory reading
//

STB_EXTERN char **stb_readdir_files  (char *dir);
STB_EXTERN char **stb_readdir_files_mask(char *dir, char *wild);
STB_EXTERN char **stb_readdir_subdirs(char *dir);
STB_EXTERN char **stb_readdir_subdirs_mask(char *dir, char *wild);
STB_EXTERN void   stb_readdir_free   (char **files);
STB_EXTERN char **stb_readdir_recursive(char *dir, char *filespec);
STB_EXTERN void stb_delete_directory_recursive(char *dir);

#ifdef STB_DEFINE

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#include <dirent.h>
#endif

void stb_readdir_free(char **files)
{
   char **f2 = files;
   int i;
   for (i=0; i < stb_arr_len(f2); ++i)
      free(f2[i]);
   stb_arr_free(f2);
}

STB_EXTERN int stb_wildmatchi(char *expr, char *candidate);
static char **readdir_raw(char *dir, int return_subdirs, char *mask)
{
   char **results = NULL;
   char buffer[512], with_slash[512];
   size_t n;

   #ifdef _MSC_VER
      stb__wchar *ws;
      struct _wfinddata_t data;
   #ifdef _WIN64
      const intptr_t none = -1;
      intptr_t z;
   #else
      const long none = -1;
      long z;
   #endif
   #else // !_MSC_VER
      const DIR *none = NULL;
      DIR *z;
   #endif

   strcpy(buffer,dir);
   stb_fixpath(buffer);
   n = strlen(buffer);

   if (n > 0 && (buffer[n-1] != '/')) {
      buffer[n++] = '/';
   }
   buffer[n] = 0;
   strcpy(with_slash, buffer);

   #ifdef _MSC_VER
      strcpy(buffer+n, "*.*");
      ws = stb__from_utf8(buffer);
      z = _wfindfirst((const wchar_t *)ws, &data);
   #else
      z = opendir(dir);
   #endif


   if (z != none) {
      int nonempty = STB_TRUE;
      #ifndef _MSC_VER
      struct dirent *data = readdir(z);
      nonempty = (data != NULL);
      #endif

      if (nonempty) {

         do {
            int is_subdir;
            #ifdef _MSC_VER
            char *name = stb__to_utf8((stb__wchar *)data.name);
            if (name == NULL) {
               fprintf(stderr, "%s to convert '%S' to %s!\n", "Unable", data.name, "utf8");
               continue;
            }
            is_subdir = !!(data.attrib & _A_SUBDIR);
            #else
            char *name = data->d_name;
            strcpy(buffer+n,name);
            DIR *y = opendir(buffer);
            is_subdir = (y != NULL);
            if (y != NULL) closedir(y);
            #endif
        
            if (is_subdir == return_subdirs) {
               if (!is_subdir || name[0] != '.') {
                  if (!mask || stb_wildmatchi(mask, name)) {
                     char buffer[512],*p=buffer;
                     sprintf(buffer, "%s%s", with_slash, name);
                     if (buffer[0] == '.' && buffer[1] == '/')
                        p = buffer+2;
                     stb_arr_push(results, strdup(p));
                  }
               }
            }
         }
         #ifdef _MSC_VER
         while (0 == _wfindnext(z, &data));
         #else
         while ((data = readdir(z)) != NULL);
         #endif
      }
      #ifdef _MSC_VER
         _findclose(z);
      #else
         closedir(z);
      #endif
   }
   return results;
}

char **stb_readdir_files  (char *dir) { return readdir_raw(dir, 0, NULL); }
char **stb_readdir_subdirs(char *dir) { return readdir_raw(dir, 1, NULL); }
char **stb_readdir_files_mask(char *dir, char *wild) { return readdir_raw(dir, 0, wild); }
char **stb_readdir_subdirs_mask(char *dir, char *wild) { return readdir_raw(dir, 1, wild); }

int stb__rec_max=0x7fffffff;
static char **stb_readdir_rec(char **sofar, char *dir, char *filespec)
{
   char **files;
   char ** dirs;
   char **p;

   if (stb_arr_len(sofar) >= stb__rec_max) return sofar;

   files = stb_readdir_files_mask(dir, filespec);
   stb_arr_for(p, files) {
      stb_arr_push(sofar, strdup(*p));
      if (stb_arr_len(sofar) >= stb__rec_max) break;
   }
   stb_readdir_free(files);
   if (stb_arr_len(sofar) >= stb__rec_max) return sofar;

   dirs = stb_readdir_subdirs(dir);
   stb_arr_for(p, dirs)
      sofar = stb_readdir_rec(sofar, *p, filespec);
   stb_readdir_free(dirs);
   return sofar;
}

char **stb_readdir_recursive(char *dir, char *filespec)
{
   return stb_readdir_rec(NULL, dir, filespec);
}

void stb_delete_directory_recursive(char *dir)
{
   char **list = stb_readdir_subdirs(dir);
   int i;
   for (i=0; i < stb_arr_len(list); ++i)
      stb_delete_directory_recursive(list[i]);
   stb_arr_free(list);
   list = stb_readdir_files(dir);
   for (i=0; i < stb_arr_len(list); ++i)
      if (!remove(list[i])) {
         // on windows, try again after making it writeable; don't ALWAYS
         // do this first since that would be slow in the normal case
         #ifdef _MSC_VER
         _chmod(list[i], _S_IWRITE);
         remove(list[i]);
         #endif
      }
   stb_arr_free(list);
   stb__windows(_rmdir,rmdir)(dir);
}

#endif

//////////////////////////////////////////////////////////////////////////////
//
//   construct trees from filenames; useful for cmirror summaries

typedef struct stb_dirtree2 stb_dirtree2;

struct stb_dirtree2
{
   stb_dirtree2 **subdirs;

   // make convenient for stb_summarize_tree
   int num_subdir;
   float weight;

   // actual data
   char *fullpath;
   char *relpath;
   char **files;
};

STB_EXTERN stb_dirtree2 *stb_dirtree2_from_files_relative(char *src, char **filelist, int count);
STB_EXTERN stb_dirtree2 *stb_dirtree2_from_files(char **filelist, int count);
STB_EXTERN int stb_dir_is_prefix(char *dir, int dirlen, char *file);

#ifdef STB_DEFINE

int stb_dir_is_prefix(char *dir, int dirlen, char *file)
{
   if (dirlen == 0) return STB_TRUE;
   if (stb_strnicmp(dir, file, dirlen)) return STB_FALSE;
   if (file[dirlen] == '/' || file[dirlen] == '\\') return STB_TRUE;
   return STB_FALSE;
}

stb_dirtree2 *stb_dirtree2_from_files_relative(char *src, char **filelist, int count)
{
   char buffer1[1024];
   int i;
   int dlen = strlen(src), elen;
   stb_dirtree2 *d;
   char ** descendents = NULL;
   char ** files = NULL;
   char *s;
   if (!count) return NULL;
   // first find all the ones that belong here... note this is will take O(NM) with N files and M subdirs
   for (i=0; i < count; ++i) {
      if (stb_dir_is_prefix(src, dlen, filelist[i])) {
         stb_arr_push(descendents, filelist[i]);
      }
   }
   if (descendents == NULL)
      return NULL;
   elen = dlen;
   // skip a leading slash
   if (elen == 0 && (descendents[0][0] == '/' || descendents[0][0] == '\\'))
      ++elen;
   else if (elen)
      ++elen;
   // now extract all the ones that have their root here
   for (i=0; i < stb_arr_len(descendents);) {
      if (!stb_strchr2(descendents[i]+elen, '/', '\\')) {
         stb_arr_push(files, descendents[i]);
         descendents[i] = descendents[stb_arr_len(descendents)-1];
         stb_arr_pop(descendents);
      } else
         ++i;
   }
   // now create a record
   d = (stb_dirtree2 *) malloc(sizeof(*d));
   d->files = files;
   d->subdirs = NULL;
   d->fullpath = strdup(src);
   s = stb_strrchr2(d->fullpath, '/', '\\');
   if (s)
      ++s;
   else
      s = d->fullpath;
   d->relpath = s;
   // now create the children
   qsort(descendents, stb_arr_len(descendents), sizeof(char *), stb_qsort_stricmp(0));
   buffer1[0] = 0;
   for (i=0; i < stb_arr_len(descendents); ++i) {
      char buffer2[1024];
      char *s = descendents[i] + elen, *t;
      t = stb_strchr2(s, '/', '\\');
      assert(t);
      stb_strncpy(buffer2, descendents[i], t-descendents[i]+1);
      if (stb_stricmp(buffer1, buffer2)) {
         stb_dirtree2 *t = stb_dirtree2_from_files_relative(buffer2, descendents, stb_arr_len(descendents));
         assert(t != NULL);
         strcpy(buffer1, buffer2);
         stb_arr_push(d->subdirs, t);
      }
   }
   d->num_subdir = stb_arr_len(d->subdirs);
   d->weight = 0;
   return d;
}

stb_dirtree2 *stb_dirtree2_from_files(char **filelist, int count)
{
   return stb_dirtree2_from_files_relative("", filelist, count);
}
#endif


//////////////////////////////////////////////////////////////////////////////
//
//         stb_match:    wildcards and regexping
//

STB_EXTERN int stb_wildmatch (char *expr, char *candidate);
STB_EXTERN int stb_wildmatchi(char *expr, char *candidate);
STB_EXTERN int stb_wildfind  (char *expr, char *candidate);
STB_EXTERN int stb_wildfindi (char *expr, char *candidate);

STB_EXTERN int stb_regex(char *regex, char *candidate);

typedef struct stb_matcher stb_matcher;

STB_EXTERN stb_matcher *stb_regex_matcher(char *regex);
STB_EXTERN int stb_matcher_match(stb_matcher *m, char *str);
STB_EXTERN int stb_matcher_find(stb_matcher *m, char *str);
STB_EXTERN void stb_matcher_free(stb_matcher *f);

STB_EXTERN stb_matcher *stb_lex_matcher(void);
STB_EXTERN int stb_lex_item(stb_matcher *m, char *str, int result);
STB_EXTERN int stb_lex_item_wild(stb_matcher *matcher, char *regex, int result);
STB_EXTERN int stb_lex(stb_matcher *m, char *str, int *len);



#ifdef STB_DEFINE

static int stb__match_qstring(char *candidate, char *qstring, int qlen, int insensitive)
{
   int i;
   if (insensitive) {
      for (i=0; i < qlen; ++i)
         if (qstring[i] == '?') {
            if (!candidate[i]) return 0;
         } else
            if (tolower(qstring[i]) != tolower(candidate[i]))
               return 0;
   } else {
      for (i=0; i < qlen; ++i)
         if (qstring[i] == '?') {
            if (!candidate[i]) return 0;
         } else
            if (qstring[i] != candidate[i])
               return 0;
   }
   return 1;
}

static int stb__find_qstring(char *candidate, char *qstring, int qlen, int insensitive)
{
   char c;

   int offset=0;
   while (*qstring == '?') {
      ++qstring;
      --qlen;
      ++candidate;
      if (qlen == 0) return 0;
      if (*candidate == 0) return -1;
   }

   c = *qstring++;
   --qlen;
   if (insensitive) c = tolower(c);

   while (candidate[offset]) {
      if (c == (insensitive ? tolower(candidate[offset]) : candidate[offset]))
         if (stb__match_qstring(candidate+offset+1, qstring, qlen, insensitive))
            return offset;
      ++offset;
   }

   return -1;
}

int stb__wildmatch_raw2(char *expr, char *candidate, int search, int insensitive)
{
   int where=0;
   int start = -1;
   
   if (!search) {
      // parse to first '*'
      if (*expr != '*')
         start = 0;
      while (*expr != '*') {
         if (!*expr)
            return *candidate == 0 ? 0 : -1;
         if (*expr == '?') {
            if (!*candidate) return -1;
         } else {
            if (insensitive) {
               if (tolower(*candidate) != tolower(*expr))
                  return -1;
            } else 
               if (*candidate != *expr)
                  return -1;
         }
         ++candidate, ++expr, ++where;
      }
   } else {
      // 0-length search string
      if (!*expr)
         return 0;
   }

   assert(search || *expr == '*');
   if (!search)
      ++expr;

   // implicit '*' at this point
      
   while (*expr) {
      int o=0;
      // combine redundant * characters
      while (expr[0] == '*') ++expr;

      // ok, at this point, expr[-1] == '*',
      // and expr[0] != '*'

      if (!expr[0]) return start >= 0 ? start : 0;

      // now find next '*'
      o = 0;
      while (expr[o] != '*') {
         if (expr[o] == 0)
            break;
         ++o;
      }
      // if no '*', scan to end, then match at end
      if (expr[o] == 0 && !search) {
         int z;
         for (z=0; z < o; ++z)
            if (candidate[z] == 0)
               return -1;
         while (candidate[z])
            ++z;
         // ok, now check if they match
         if (stb__match_qstring(candidate+z-o, expr, o, insensitive))
            return start >= 0 ? start : 0;
         return -1; 
      } else {
         // if yes '*', then do stb__find_qmatch on the intervening chars
         int n = stb__find_qstring(candidate, expr, o, insensitive);
         if (n < 0)
            return -1;
         if (start < 0)
            start = where + n;
         expr += o;
         candidate += n+o;
      }

      if (*expr == 0) {
         assert(search);
         return start;
      }

      assert(*expr == '*');
      ++expr;
   }

   return start >= 0 ? start : 0;
}

int stb__wildmatch_raw(char *expr, char *candidate, int search, int insensitive)
{
   char buffer[256];
   // handle multiple search strings
   char *s = strchr(expr, ';');
   char *last = expr;
   while (s) {
      int z;
      // need to allow for non-writeable strings... assume they're small
      if (s - last < 256) {
         stb_strncpy(buffer, last, s-last+1);
         z = stb__wildmatch_raw2(buffer, candidate, search, insensitive);
      } else {
         *s = 0;
         z = stb__wildmatch_raw2(last, candidate, search, insensitive);
         *s = ';';
      }
      if (z >= 0) return z;
      last = s+1;
      s = strchr(last, ';');
   }
   return stb__wildmatch_raw2(last, candidate, search, insensitive);
}

int stb_wildmatch(char *expr, char *candidate)
{
   return stb__wildmatch_raw(expr, candidate, 0,0) >= 0;
}

int stb_wildmatchi(char *expr, char *candidate)
{
   return stb__wildmatch_raw(expr, candidate, 0,1) >= 0;
}

int stb_wildfind(char *expr, char *candidate)
{
   return stb__wildmatch_raw(expr, candidate, 1,0);
}

int stb_wildfindi(char *expr, char *candidate)
{
   return stb__wildmatch_raw(expr, candidate, 1,1);
}

typedef struct
{
   stb_int16 transition[256];
} stb_dfa;

// an NFA node represents a state you're in; it then has
// an arbitrary number of edges dangling off of it
// note this isn't utf8-y
typedef struct
{
   stb_int16  match; // character/set to match
   stb_uint16 node;  // output node to go to
} stb_nfa_edge;

typedef struct
{
   stb_int16 goal;   // does reaching this win the prize?
   stb_uint8 active; // is this in the active list
   stb_nfa_edge *out;
   stb_uint16 *eps;  // list of epsilon closures
} stb_nfa_node;

#define STB__DFA_UNDEF  -1
#define STB__DFA_GOAL   -2
#define STB__DFA_END    -3
#define STB__DFA_MGOAL  -4
#define STB__DFA_VALID  0

#define STB__NFA_STOP_GOAL -1

// compiled regexp
struct stb_matcher
{
   stb_uint16 start_node;
   stb_int16 dfa_start;
   stb_uint32 *charset;
   int num_charset;
   int match_start;
   stb_nfa_node *nodes;
   int does_lex;

   // dfa matcher
   stb_dfa    * dfa;
   stb_uint32 * dfa_mapping;
   stb_int16  * dfa_result;
   int num_words_per_dfa;
};

static int stb__add_node(stb_matcher *matcher)
{
   stb_nfa_node z;
   z.active = 0;
   z.eps    = 0;
   z.goal   = 0;
   z.out    = 0;
   stb_arr_push(matcher->nodes, z);
   return stb_arr_len(matcher->nodes)-1;
}

static void stb__add_epsilon(stb_matcher *matcher, int from, int to)
{
   assert(from != to);
   if (matcher->nodes[from].eps == NULL)
      stb_arr_malloc((void **) &matcher->nodes[from].eps, matcher);
   stb_arr_push(matcher->nodes[from].eps, to);
}

static void stb__add_edge(stb_matcher *matcher, int from, int to, int type)
{
   stb_nfa_edge z = { type, to };
   if (matcher->nodes[from].out == NULL)
      stb_arr_malloc((void **) &matcher->nodes[from].out, matcher);
   stb_arr_push(matcher->nodes[from].out, z);
}

static char *stb__reg_parse_alt(stb_matcher *m, int s, char *r, stb_uint16 *e);
static char *stb__reg_parse(stb_matcher *matcher, int start, char *regex, stb_uint16 *end)
{
   int n;
   int last_start = -1;
   stb_uint16 last_end = start;

   while (*regex) {
      switch (*regex) {
         case '(':
            last_start = last_end;
            regex = stb__reg_parse_alt(matcher, last_end, regex+1, &last_end);
            if (regex == NULL || *regex != ')')
               return NULL;
            ++regex;
            break;

         case '|':
         case ')':
            *end = last_end;
            return regex;

         case '?':
            if (last_start < 0) return NULL;
            stb__add_epsilon(matcher, last_start, last_end);
            ++regex;
            break;

         case '*':
            if (last_start < 0) return NULL;
            stb__add_epsilon(matcher, last_start, last_end);

            // fall through

         case '+':
            if (last_start < 0) return NULL;
            stb__add_epsilon(matcher, last_end, last_start);
            // prevent links back to last_end from chaining to last_start
            n = stb__add_node(matcher);
            stb__add_epsilon(matcher, last_end, n);
            last_end = n;
            ++regex;
            break;

         case '{':   // not supported!
            // @TODO: given {n,m}, clone last_start to last_end m times,
            // and include epsilons from start to first m-n blocks
            return NULL; 

         case '\\':
            ++regex;
            if (!*regex) return NULL;

            // fallthrough
         default: // match exactly this character
            n = stb__add_node(matcher);
            stb__add_edge(matcher, last_end, n, *regex);
            last_start = last_end;
            last_end = n;
            ++regex;
            break;

         case '$':
            n = stb__add_node(matcher);
            stb__add_edge(matcher, last_end, n, '\n');
            last_start = last_end;
            last_end = n;
            ++regex;
            break;

         case '.':
            n = stb__add_node(matcher);
            stb__add_edge(matcher, last_end, n, -1);
            last_start = last_end;
            last_end = n;
            ++regex;
            break;

         case '[': {
            stb_uint8 flags[256];
            int invert = 0,z;
            ++regex;
            if (matcher->num_charset == 0) {
               matcher->charset = (stb_uint *) stb_malloc(matcher, sizeof(*matcher->charset) * 256);
               memset(matcher->charset, 0, sizeof(*matcher->charset) * 256);
            }

            memset(flags,0,sizeof(flags));

            // leading ^ is special
            if (*regex == '^')
               ++regex, invert = 1;

            // leading ] is special
            if (*regex == ']') {
               flags[']'] = 1;
               ++regex;
            }
            while (*regex != ']') {
               stb_uint a;
               if (!*regex) return NULL;
               a = *regex++;
               if (regex[0] == '-' && regex[1] != ']') {
                  stb_uint i,b = regex[1];
                  regex += 2;
                  if (b == 0) return NULL;
                  if (a > b) return NULL;
                  for (i=a; i <= b; ++i)
                     flags[i] = 1;
               } else
                  flags[a] = 1;
            }
            ++regex;
            if (invert) {
               int i;
               for (i=0; i < 256; ++i)
                  flags[i] = 1-flags[i];
            }

            // now check if any existing charset matches
            for (z=0; z < matcher->num_charset; ++z) {
               int i, k[2] = { 0, 1 << z};
               for (i=0; i < 256; ++i) {
                  unsigned int f = k[flags[i]];
                  if ((matcher->charset[i] & k[1]) != f)
                     break;
               }
               if (i == 256) break;
            }

            if (z == matcher->num_charset) {
               int i;
               ++matcher->num_charset;
               if (matcher->num_charset > 32) {
                  assert(0); /* NOTREACHED */
                  return NULL; // too many charsets, oops
               }
               for (i=0; i < 256; ++i)
                  if (flags[i])
                     matcher->charset[i] |= (1 << z);
            }

            n = stb__add_node(matcher);
            stb__add_edge(matcher, last_end, n, -2 - z);
            last_start = last_end;
            last_end = n;
            break;
         }
      }
   }
   *end = last_end;
   return regex;
}

static char *stb__reg_parse_alt(stb_matcher *matcher, int start, char *regex, stb_uint16 *end)
{
   stb_uint16 last_end = start;
   stb_uint16 main_end;

   int head, tail;

   head = stb__add_node(matcher);
   stb__add_epsilon(matcher, start, head);

   regex = stb__reg_parse(matcher, head, regex, &last_end);
   if (regex == NULL) return NULL;
   if (*regex == 0 || *regex == ')') {
      *end = last_end;
      return regex;
   }

   main_end = last_end;
   tail = stb__add_node(matcher);

   stb__add_epsilon(matcher, last_end, tail);

   // start alternatives from the same starting node; use epsilon
   // transitions to combine their endings
   while(*regex && *regex != ')') {
      assert(*regex == '|');
      head = stb__add_node(matcher);
      stb__add_epsilon(matcher, start, head);
      regex = stb__reg_parse(matcher, head, regex+1, &last_end);
      if (regex == NULL)
         return NULL;
      stb__add_epsilon(matcher, last_end, tail);
   }

   *end = tail;
   return regex;
}

static char *stb__wild_parse(stb_matcher *matcher, int start, char *str, stb_uint16 *end)
{
   int n;
   stb_uint16 last_end;

   last_end = stb__add_node(matcher);
   stb__add_epsilon(matcher, start, last_end);

   while (*str) {
      switch (*str) {
            // fallthrough
         default: // match exactly this character
            n = stb__add_node(matcher);
            if (toupper(*str) == tolower(*str)) {
               stb__add_edge(matcher, last_end, n, *str);
            } else {
               stb__add_edge(matcher, last_end, n, tolower(*str));
               stb__add_edge(matcher, last_end, n, toupper(*str));
            }
            last_end = n;
            ++str;
            break;

         case '?':
            n = stb__add_node(matcher);
            stb__add_edge(matcher, last_end, n, -1);
            last_end = n;
            ++str;
            break;

         case '*':
            n = stb__add_node(matcher);
            stb__add_edge(matcher, last_end, n, -1);
            stb__add_epsilon(matcher, last_end, n);
            stb__add_epsilon(matcher, n, last_end);
            last_end = n;
            ++str;
            break;
      }
   }

   // now require end of string to match
   n = stb__add_node(matcher);
   stb__add_edge(matcher, last_end, n, 0);
   last_end = n;

   *end = last_end;
   return str;
}

static int stb__opt(stb_matcher *m, int n)
{
   for(;;) {
      stb_nfa_node *p = &m->nodes[n];
      if (p->goal)                  return n;
      if (stb_arr_len(p->out))      return n;
      if (stb_arr_len(p->eps) != 1) return n;
      n = p->eps[0];
   }
}

static void stb__optimize(stb_matcher *m)
{
   // if the target of any edge is a node with exactly
   // one out-epsilon, shorten it
   int i,j;
   for (i=0; i < stb_arr_len(m->nodes); ++i) {
      stb_nfa_node *p = &m->nodes[i];
      for (j=0; j < stb_arr_len(p->out); ++j)
         p->out[j].node = stb__opt(m,p->out[j].node);
      for (j=0; j < stb_arr_len(p->eps); ++j)
         p->eps[j]      = stb__opt(m,p->eps[j]     );
   }
   m->start_node = stb__opt(m,m->start_node);
}

void stb_matcher_free(stb_matcher *f)
{
   stb_free(f);
}

static stb_matcher *stb__alloc_matcher(void)
{
   stb_matcher *matcher = (stb_matcher *) stb_malloc(0,sizeof(*matcher));

   matcher->start_node  = 0;
   stb_arr_malloc((void **) &matcher->nodes, matcher);
   matcher->num_charset = 0;
   matcher->match_start = 0;
   matcher->does_lex    = 0;

   matcher->dfa_start   = STB__DFA_UNDEF;
   stb_arr_malloc((void **) &matcher->dfa, matcher);
   stb_arr_malloc((void **) &matcher->dfa_mapping, matcher);
   stb_arr_malloc((void **) &matcher->dfa_result, matcher);

   stb__add_node(matcher);

   return matcher;
}

static void stb__lex_reset(stb_matcher *matcher)
{
   // flush cached dfa data
   stb_arr_setlen(matcher->dfa, 0);
   stb_arr_setlen(matcher->dfa_mapping, 0);
   stb_arr_setlen(matcher->dfa_result, 0);
   matcher->dfa_start = STB__DFA_UNDEF;
}

stb_matcher *stb_regex_matcher(char *regex)
{
   char *z;
   stb_uint16 end;
   stb_matcher *matcher = stb__alloc_matcher();
   if (*regex == '^') {
      matcher->match_start = 1;
      ++regex;
   }

   z = stb__reg_parse_alt(matcher, matcher->start_node, regex, &end);

   if (!z || *z) {
      stb_free(matcher);
      return NULL;
   }

   ((matcher->nodes)[(int) end]).goal = STB__NFA_STOP_GOAL;

   return matcher;
}

stb_matcher *stb_lex_matcher(void)
{
   stb_matcher *matcher = stb__alloc_matcher();

   matcher->match_start = 1;
   matcher->does_lex    = 1;

   return matcher;
}

int stb_lex_item(stb_matcher *matcher, char *regex, int result)
{
   char *z;
   stb_uint16 end;

   z = stb__reg_parse_alt(matcher, matcher->start_node, regex, &end);

   if (z == NULL)
      return 0;

   stb__lex_reset(matcher);

   matcher->nodes[(int) end].goal = result;
   return 1;
}

int stb_lex_item_wild(stb_matcher *matcher, char *regex, int result)
{
   char *z;
   stb_uint16 end;

   z = stb__wild_parse(matcher, matcher->start_node, regex, &end);

   if (z == NULL)
      return 0;

   stb__lex_reset(matcher);

   matcher->nodes[(int) end].goal = result;
   return 1;
}

static void stb__clear(stb_matcher *m, stb_uint16 *list)
{
   int i;
   for (i=0; i < stb_arr_len(list); ++i)
      m->nodes[(int) list[i]].active = 0;
}

static int stb__clear_goalcheck(stb_matcher *m, stb_uint16 *list)
{
   int i, t=0;
   for (i=0; i < stb_arr_len(list); ++i) {
      t += m->nodes[(int) list[i]].goal;
      m->nodes[(int) list[i]].active = 0;
   }
   return t;
}

static stb_uint16 * stb__add_if_inactive(stb_matcher *m, stb_uint16 *list, int n)
{
   if (!m->nodes[n].active) {
      stb_arr_push(list, n);
      m->nodes[n].active = 1;
   }
   return list;
}

static stb_uint16 * stb__eps_closure(stb_matcher *m, stb_uint16 *list)
{
   int i,n = stb_arr_len(list);

   for(i=0; i < n; ++i) {
      stb_uint16 *e = m->nodes[(int) list[i]].eps;
      if (e) {
         int j,k = stb_arr_len(e);
         for (j=0; j < k; ++j)
            list = stb__add_if_inactive(m, list, e[j]);
         n = stb_arr_len(list);
      }
   }

   return list;
}

int stb_matcher_match(stb_matcher *m, char *str)
{
   int result = 0;
   int i,j,y,z;
   stb_uint16 *previous = NULL;
   stb_uint16 *current = NULL;
   stb_uint16 *temp;

   stb_arr_setsize(previous, 4);
   stb_arr_setsize(current, 4);

   previous = stb__add_if_inactive(m, previous, m->start_node);
   previous = stb__eps_closure(m,previous);
   stb__clear(m, previous);

   while (*str && stb_arr_len(previous)) {
      y = stb_arr_len(previous);
      for (i=0; i < y; ++i) {
         stb_nfa_node *n = &m->nodes[(int) previous[i]];
         z = stb_arr_len(n->out);
         for (j=0; j < z; ++j) {
            if (n->out[j].match >= 0) {
               if (n->out[j].match == *str)
                  current = stb__add_if_inactive(m, current, n->out[j].node);
            } else if (n->out[j].match == -1) {
               if (*str != '\n')
                  current = stb__add_if_inactive(m, current, n->out[j].node);
            } else if (n->out[j].match < -1) {
               int z = -n->out[j].match - 2;
               if (m->charset[(stb_uint8) *str] & (1 << z))
                  current = stb__add_if_inactive(m, current, n->out[j].node);
            }
         }
      }
      stb_arr_setlen(previous, 0);

      temp = previous;
      previous = current;
      current = temp;

      previous = stb__eps_closure(m,previous);
      stb__clear(m, previous);

      ++str;
   }

   // transition to pick up a '$' at the end
   y = stb_arr_len(previous);
   for (i=0; i < y; ++i)
      m->nodes[(int) previous[i]].active = 1;

   for (i=0; i < y; ++i) {
      stb_nfa_node *n = &m->nodes[(int) previous[i]];
      z = stb_arr_len(n->out);
      for (j=0; j < z; ++j) {
         if (n->out[j].match == '\n')
            current = stb__add_if_inactive(m, current, n->out[j].node);
      }
   }

   previous = stb__eps_closure(m,previous);
   stb__clear(m, previous);

   y = stb_arr_len(previous);
   for (i=0; i < y; ++i)
      if (m->nodes[(int) previous[i]].goal)
         result = 1;

   stb_arr_free(previous);
   stb_arr_free(current);

   return result && *str == 0;
}

stb_int16 stb__get_dfa_node(stb_matcher *m, stb_uint16 *list)
{
   stb_uint16 node;
   stb_uint32 data[8], *state, *newstate;
   int i,j,n;

   state = (stb_uint32 *) stb_temp(data, m->num_words_per_dfa * 4);
   memset(state, 0, m->num_words_per_dfa*4);

   n = stb_arr_len(list);
   for (i=0; i < n; ++i) {
      int x = list[i];
      state[x >> 5] |= 1 << (x & 31);
   }

   // @TODO use a hash table
   n = stb_arr_len(m->dfa_mapping);
   i=j=0;
   for(; j < n; ++i, j += m->num_words_per_dfa) {
      // @TODO special case for <= 32
      if (!memcmp(state, m->dfa_mapping + j, m->num_words_per_dfa*4)) {
         node = i;
         goto done;
      }
   }

   assert(stb_arr_len(m->dfa) == i);
   node = i;

   newstate = stb_arr_addn(m->dfa_mapping, m->num_words_per_dfa);
   memcpy(newstate, state, m->num_words_per_dfa*4);

   // set all transitions to 'unknown'
   stb_arr_add(m->dfa);
   memset(m->dfa[i].transition, -1, sizeof(m->dfa[i].transition));

   if (m->does_lex) {
      int result = -1;
      n = stb_arr_len(list);
      for (i=0; i < n; ++i) {
         if (m->nodes[(int) list[i]].goal > result)
            result = m->nodes[(int) list[i]].goal;
      }

      stb_arr_push(m->dfa_result, result);
   }

done:
   stb_tempfree(data, state);
   return node;
}

static int stb__matcher_dfa(stb_matcher *m, char *str_c, int *len)
{
   stb_uint8 *str = (stb_uint8 *) str_c;
   stb_int16 node,prevnode;
   stb_dfa *trans;
   int match_length = 0;
   stb_int16 match_result=0;

   if (m->dfa_start == STB__DFA_UNDEF) {
      stb_uint16 *list;

      m->num_words_per_dfa = (stb_arr_len(m->nodes)+31) >> 5;
      stb__optimize(m);

      list = stb__add_if_inactive(m, NULL, m->start_node);
      list = stb__eps_closure(m,list);
      if (m->does_lex) {
         m->dfa_start = stb__get_dfa_node(m,list);
         stb__clear(m, list);
         // DON'T allow start state to be a goal state!
         // this allows people to specify regexes that can match 0
         // characters without them actually matching (also we don't
         // check _before_ advancing anyway
         if (m->dfa_start <= STB__DFA_MGOAL)
            m->dfa_start = -(m->dfa_start - STB__DFA_MGOAL);
      } else {
         if (stb__clear_goalcheck(m, list))
            m->dfa_start = STB__DFA_GOAL;
         else
            m->dfa_start = stb__get_dfa_node(m,list);
      }
      stb_arr_free(list);
   }

   prevnode = STB__DFA_UNDEF;
   node = m->dfa_start;
   trans = m->dfa;

   if (m->dfa_start == STB__DFA_GOAL)
      return 1;

   for(;;) {
      assert(node >= STB__DFA_VALID);

      // fast inner DFA loop; especially if STB__DFA_VALID is 0

      do {
         prevnode = node;
         node = trans[node].transition[*str++];
      } while (node >= STB__DFA_VALID);

      assert(node >= STB__DFA_MGOAL - stb_arr_len(m->dfa));
      assert(node < stb_arr_len(m->dfa));

      // special case for lex: need _longest_ match, so notice goal
      // state without stopping
      if (node <= STB__DFA_MGOAL) {
         match_length = str - (stb_uint8 *) str_c;
         node = -(node - STB__DFA_MGOAL);
         match_result = node;
         continue;
      }

      // slow NFA->DFA conversion

      // or we hit the goal or the end of the string, but those
      // can only happen once per search...

      if (node == STB__DFA_UNDEF) {
         // build a list  -- @TODO special case <= 32 states
         // heck, use a more compact data structure for <= 16 and <= 8 ?!

         // @TODO keep states/newstates around instead of reallocating them
         stb_uint16 *states = NULL;
         stb_uint16 *newstates = NULL;
         int i,j,y,z;
         stb_uint32 *flags = &m->dfa_mapping[prevnode * m->num_words_per_dfa];
         assert(prevnode != STB__DFA_UNDEF);
         stb_arr_setsize(states, 4);
         stb_arr_setsize(newstates,4);
         for (j=0; j < m->num_words_per_dfa; ++j) {
            for (i=0; i < 32; ++i) {
               if (*flags & (1 << i))
                  stb_arr_push(states, j*32+i);
            }
            ++flags;
         }
         // states is now the states we were in in the previous node;
         // so now we can compute what node it transitions to on str[-1]

         y = stb_arr_len(states);
         for (i=0; i < y; ++i) {
            stb_nfa_node *n = &m->nodes[(int) states[i]];
            z = stb_arr_len(n->out);
            for (j=0; j < z; ++j) {
               if (n->out[j].match >= 0) {
                  if (n->out[j].match == str[-1] || (str[-1] == 0 && n->out[j].match == '\n'))
                     newstates = stb__add_if_inactive(m, newstates, n->out[j].node);
               } else if (n->out[j].match == -1) {
                  if (str[-1] != '\n' && str[-1])
                     newstates = stb__add_if_inactive(m, newstates, n->out[j].node);
               } else if (n->out[j].match < -1) {
                  int z = -n->out[j].match - 2;
                  if (m->charset[str[-1]] & (1 << z))
                     newstates = stb__add_if_inactive(m, newstates, n->out[j].node);
               }
            }
         }
         // AND add in the start state!
         if (!m->match_start || (str[-1] == '\n' && !m->does_lex))
            newstates = stb__add_if_inactive(m, newstates, m->start_node);
         // AND epsilon close it
         newstates = stb__eps_closure(m, newstates);
         // if it's a goal state, then that's all there is to it
         if (stb__clear_goalcheck(m, newstates)) {
            if (m->does_lex) {
               match_length = str - (stb_uint8 *) str_c;
               node = stb__get_dfa_node(m,newstates);
               match_result = node;
               node = -node + STB__DFA_MGOAL;
               trans = m->dfa; // could have gotten realloc()ed
            } else
               node = STB__DFA_GOAL;
         } else if (str[-1] == 0 || stb_arr_len(newstates) == 0) {
            node = STB__DFA_END;
         } else {
            node = stb__get_dfa_node(m,newstates);
            trans = m->dfa; // could have gotten realloc()ed
         }
         trans[prevnode].transition[str[-1]] = node;
         if (node <= STB__DFA_MGOAL)
            node = -(node - STB__DFA_MGOAL);
         stb_arr_free(newstates);
         stb_arr_free(states);
      }

      if (node == STB__DFA_GOAL) {
         return 1;
      }
      if (node == STB__DFA_END) {
         if (m->does_lex) {
            if (match_result) {
               if (len) *len = match_length;
               return m->dfa_result[(int) match_result];
            }
         }
         return 0;
      }

      assert(node != STB__DFA_UNDEF);
   }
}

int stb_matcher_find(stb_matcher *m, char *str)
{
   assert(m->does_lex == 0);
   return stb__matcher_dfa(m, str, NULL);
}

int stb_lex(stb_matcher *m, char *str, int *len)
{
   assert(m->does_lex);
   return stb__matcher_dfa(m, str, len);
}

int stb_regex(char *regex, char *str)
{
   static stb_perfect p;
   static stb_matcher ** matchers;
   static char        ** regexps;
   static char        ** regexp_cache;
   static unsigned short *mapping;
   int z = stb_perfect_hash(&p, (size_t) regex);
   if (z >= 0) {
      if (strcmp(regex, regexp_cache[(int) mapping[z]])) {
         int i = mapping[z];
         stb_matcher_free(matchers[i]);
         free(regexp_cache[i]);
         regexps[i] = regex;
         regexp_cache[i] = strdup(regex);
         matchers[i] = stb_regex_matcher(regex);
      }
   } else {
      int i,n;
      if (regex == NULL) {
         for (i=0; i < stb_arr_len(matchers); ++i) {
            stb_matcher_free(matchers[i]);
            free(regexp_cache[i]);
         }
         stb_arr_free(matchers);
         stb_arr_free(regexps);
         stb_arr_free(regexp_cache);
         stb_perfect_destroy(&p);
         free(mapping); mapping = NULL;
         return -1;
      }
      stb_arr_push(regexps, regex);
      stb_arr_push(regexp_cache, strdup(regex));
      stb_arr_push(matchers, stb_regex_matcher(regex));
      stb_perfect_destroy(&p);
      n = stb_perfect_create(&p, (unsigned int *) (char **) regexps, stb_arr_len(regexps));
      mapping = (unsigned short *) realloc(mapping, n * sizeof(*mapping));
      for (i=0; i < stb_arr_len(regexps); ++i)
         mapping[stb_perfect_hash(&p, (size_t) regexps[i])] = i;
      z = stb_perfect_hash(&p, (int) (size_t) regex);
   }
   return stb_matcher_find(matchers[(int) mapping[z]], str);
}

#endif // STB_DEFINE

#undef STB_EXTERN
#endif // STB_INCLUDE_STB_H

