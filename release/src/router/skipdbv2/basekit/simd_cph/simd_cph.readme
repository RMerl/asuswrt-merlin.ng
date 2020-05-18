/*****************************************************

   Cross-platform SIMD intrinsics header file

   VERSION: 2004.10.26 (alpha)
   
   Created by Patrick Roberts
   
   This is an on-going project.  Please add functions and
   typedefs as needed, but try to follow the guideline
   below:
   
   The goal of this file is to stay cross-platform.
   Only intrinsics or #defines that mimic another system's
   SIMD instruction should be included, with the only exception
   being instructions that, if non existant, are not
   needed (see bottom)
      
   Currently, the goal is to base support around 128-bit SIMD.
   (Only the Gekko and x86-MMX are 64-bit)
   
   Changelog:
   
   2004.05.09  [Patrick Roberts]
   	*) Created file with some i386, GCC dialect
   2004.10.22  [Patrick Roberts]
        *) Created emulated SIMD
   2004.10.25  [Patrick Roberts]
   	*) Created arm-iwmmx GCC dialect
	*) Fixed sqrt bug in emu dialect
	*) Organized directories
	*) Makefile for test app
	

   To Do:
   
   *( Docs
   *( Add new intrinsics to test app
   *( MinGW x86 dialect (same as GCC on Linux?)
   *( Does 3DNOW buy us anything?
   *( Intel ICC x86 dialect
   *( MSVC .NET x86 dialect
   *( Support for ARM ARM6, VFP and NEON SIMD?  What compilers use these?
   *( PowerPC AltiVec/Velocity/VMX components
   *( MIPS-MMI / PS2-VU components
   *( See if SSE2 buys us anything beyond what the compiler does already
   *( Compaq Alpha components
   
*/

/***************************************************

  Platform Notes:
  
  
     General
     -------
     
        NOTE: Code must be 16-byte aligned. Align to 16 when allocating memory.
	
	X86/XSCALE (Intel) vs. PowerPC/MIPS
	
	While the PowerPC and MIPS SIMD instructions take 2 source vectors
	and a destination vector, the Intel platforms only take a source and
	destination.  Example:
	
	   PPC/MIPS can do:
	   
	      C = A + B
	      
	   X86 can only do:
	   
	     A = A + B   (or A+=B)
	     
	 Code written either way will work on the X86, and still be faster than
	 387 math, but preserving the registers takes significant overhead.
	 (Disassemble the test program for an example.  The prints preserve, the
	 'disassembly test' does not.)   For the fastest code between systems, write
	 your SIMD math as the X86 expects, manually preserving SIMD variables.
	 At least GCC for PPC doesn't seem to have any issues figuring out how to
	 deal with a source and destination memory address being the same.
  
  
     GCC x86
     -------
         
        You must compile with -msse and -mmmx.  I try to avoid mmx as mmx is slower on
	the P4 than on the P3 and XP, but sse doesn't have integer math.
	 
	 You may want to set -msse2 if you have a P4 CPU (-msse2 is set by default
	 for x86-64 CPUS), as some of the simd functions not supported on x86
	 can be sped up by gcc using sse2 commands rather than standard pipeline
	 commands.
	
     
     GCC PowerPC
     -----------
     
        You must compile with the switch -maltivec


     GCC ARM (Xscale only)
     ----------------
        
	GCC ARM only seems to support Intel Wirekess MMX (XSCALE), not ARMv6,
	Neon, or VFP? (Are these all the same beast?)

        You must compile with +iwmmxt 

	
*/
