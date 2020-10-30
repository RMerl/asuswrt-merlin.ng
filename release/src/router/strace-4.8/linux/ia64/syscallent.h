/*
 * Copyright (c) 1999, 2001 Hewlett-Packard Co
 *	David Mosberger-Tang <davidm@hpl.hp.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * IA-32 syscalls that have pointer arguments which are incompatible
 * with 64-bit layout get redirected to printargs.
 */
#define sys_getrlimit		printargs
#define sys_afs_syscall		printargs
#define sys_getpmsg		printargs
#define sys_putpmsg		printargs
#define sys_ugetrlimit		printargs
#define sys_waitpid		printargs
#define sys_time		printargs
#define sys_break		printargs
#define sys_oldstat		printargs
#define sys_lseek		printargs
#define sys_stime		printargs
#define sys_ptrace		printargs
#define sys_oldfstat		printargs
#define sys_pause		printargs
#define sys_utime		printargs
#define sys_stty		printargs
#define sys_gtty		printargs
#define sys_ftime		printargs
#define sys_pipe		printargs
#define sys_times		printargs
#define sys_prof		printargs
#define sys_signal		printargs
#define sys_lock		printargs
#define sys_ioctl		printargs
#define sys_fcntl		printargs
#define sys_mpx			printargs
#define sys_ulimit		printargs
#define sys_oldolduname		printargs
#define sys_sigaction		printargs
#define sys_siggetmask		printargs
#define sys_sigsetmask		printargs
#define sys_sigsuspend		printargs
#define sys_sigpending		printargs
#define sys_setrlimit		printargs
#define sys_getrusage		printargs
#define sys_gettimeofday	printargs
#define sys_settimeofday	printargs
#define sys_getgroups		printargs
#define sys_setgroups		printargs
#define sys_select		printargs
#undef sys_oldlstat
#define sys_oldlstat		printargs
#define sys_readdir		printargs
#define sys_profil		printargs
#define sys_statfs		printargs
#define sys_fstatfs		printargs
#define sys_ioperm		printargs
#define sys_setitimer		printargs
#define sys_getitimer		printargs
#define sys_stat		printargs
#undef sys_lstat
#define sys_lstat		printargs
#define sys_fstat		printargs
#define sys_olduname		printargs
#define sys_iopl		printargs
#define sys_idle		printargs
#define sys_vm86old		printargs
#define sys_wait4		printargs
#define sys_sysinfo		printargs
#define sys_sigreturn		printargs
#define sys_uname		printargs
#define sys_modify_ldt		printargs
#define sys_adjtimex		printargs
#define sys_sigprocmask		printargs
#define sys_create_module	printargs
#define sys_init_module		printargs
#define sys_get_kernel_syms	printargs
#define sys_quotactl		printargs
#define sys_bdflush		printargs
#define sys_personality		printargs
#define sys_getdents		printargs
#define sys__newselect		printargs
#define sys_msync		printargs
#define sys_readv		printargs
#define sys_writev		printargs
#define sys__sysctl		printargs
#define sys_sched_rr_get_interval printargs
#define sys_getresuid		printargs
#define sys_vm86		printargs
#define sys_query_module	printargs
#define sys_nfsservctl		printargs
#define sys_rt_sigreturn	printargs
#define sys_rt_sigaction	printargs
#define sys_rt_sigprocmask	printargs
#define sys_rt_sigtimedwait	printargs
#define sys_rt_sigqueueinfo	printargs
#define sys_rt_sigsuspend	printargs
#define sys_pread		printargs
#define sys_pwrite		printargs
#define sys_sigaltstack		printargs
#define sys_sendfile		printargs
#define sys_truncate64		printargs
#define sys_ftruncate64		printargs
#define sys_stat64		printargs
#undef sys_lstat64
#define sys_lstat64		printargs
#define sys_fstat64		printargs
#define sys_fcntl64		printargs

#include "../i386/syscallent.h"

#undef sys_getrlimit
#undef sys_afs_syscall
#undef sys_getpmsg
#undef sys_putpmsg
#undef sys_ugetrlimit
#undef sys_waitpid
#undef sys_time
#undef sys_break
#undef sys_oldstat
#undef sys_lseek
#undef sys_stime
#undef sys_ptrace
#undef sys_oldfstat
#undef sys_pause
#undef sys_utime
#undef sys_stty
#undef sys_gtty
#undef sys_ftime
#undef sys_pipe
#undef sys_times
#undef sys_prof
#undef sys_signal
#undef sys_lock
#undef sys_ioctl
#undef sys_fcntl
#undef sys_mpx
#undef sys_ulimit
#undef sys_oldolduname
#undef sys_sigaction
#undef sys_siggetmask
#undef sys_sigsetmask
#undef sys_sigsuspend
#undef sys_sigpending
#undef sys_setrlimit
#undef sys_getrusage
#undef sys_gettimeofday
#undef sys_settimeofday
#undef sys_getgroups
#undef sys_setgroups
#undef sys_select
#undef sys_oldlstat
#undef sys_readdir
#undef sys_profil
#undef sys_statfs
#undef sys_fstatfs
#undef sys_ioperm
#undef sys_setitimer
#undef sys_getitimer
#undef sys_stat
#undef sys_lstat
#undef sys_fstat
#undef sys_olduname
#undef sys_iopl
#undef sys_idle
#undef sys_vm86old
#undef sys_wait4
#undef sys_sysinfo
#undef sys_sigreturn
#undef sys_uname
#undef sys_modify_ldt
#undef sys_adjtimex
#undef sys_sigprocmask
#undef sys_create_module
#undef sys_init_module
#undef sys_get_kernel_syms
#undef sys_quotactl
#undef sys_bdflush
#undef sys_personality
#undef sys_getdents
#undef sys__newselect
#undef sys_msync
#undef sys_readv
#undef sys_writev
#undef sys__sysctl
#undef sys_sched_rr_get_interval
#undef sys_getresuid
#undef sys_vm86
#undef sys_query_module
#undef sys_nfsservctl
#undef sys_rt_sigreturn
#undef sys_rt_sigaction
#undef sys_rt_sigprocmask
#undef sys_rt_sigtimedwait
#undef sys_rt_sigqueueinfo
#undef sys_rt_sigsuspend
#undef sys_pread
#undef sys_pwrite
#undef sys_sigaltstack
#undef sys_sendfile
#undef sys_truncate64
#undef sys_ftruncate64
#undef sys_stat64
#undef sys_lstat64
#undef sys_fstat64
#undef sys_fcntl64

#include "../dummy.h"

/* You must be careful to check ../i386/syscallent.h so that this table
   starts where that one leaves off.
*/
#if SYS_ipc_subcall + SYS_ipc_nsubcalls != 445
# error fix me
#endif

	{ MA,	0,	NULL,			NULL		}, /* 445 */
	{ MA,	0,	NULL,			NULL		}, /* 446 */
	{ MA,	0,	NULL,			NULL		}, /* 447 */
	{ MA,	0,	NULL,			NULL		}, /* 448 */
	{ MA,	0,	NULL,			NULL		}, /* 449 */
	{ MA,	0,	NULL,			NULL		}, /* 450 */
	{ MA,	0,	NULL,			NULL		}, /* 451 */
	{ MA,	0,	NULL,			NULL		}, /* 452 */
	{ MA,	0,	NULL,			NULL		}, /* 453 */
	{ MA,	0,	NULL,			NULL		}, /* 454 */
	{ MA,	0,	NULL,			NULL		}, /* 455 */
	{ MA,	0,	NULL,			NULL		}, /* 456 */
	{ MA,	0,	NULL,			NULL		}, /* 457 */
	{ MA,	0,	NULL,			NULL		}, /* 458 */
	{ MA,	0,	NULL,			NULL		}, /* 459 */
	{ MA,	0,	NULL,			NULL		}, /* 460 */
	{ MA,	0,	NULL,			NULL		}, /* 461 */
	{ MA,	0,	NULL,			NULL		}, /* 462 */
	{ MA,	0,	NULL,			NULL		}, /* 463 */
	{ MA,	0,	NULL,			NULL		}, /* 464 */
	{ MA,	0,	NULL,			NULL		}, /* 465 */
	{ MA,	0,	NULL,			NULL		}, /* 466 */
	{ MA,	0,	NULL,			NULL		}, /* 467 */
	{ MA,	0,	NULL,			NULL		}, /* 468 */
	{ MA,	0,	NULL,			NULL		}, /* 469 */
	{ MA,	0,	NULL,			NULL		}, /* 470 */
	{ MA,	0,	NULL,			NULL		}, /* 471 */
	{ MA,	0,	NULL,			NULL		}, /* 472 */
	{ MA,	0,	NULL,			NULL		}, /* 473 */
	{ MA,	0,	NULL,			NULL		}, /* 474 */
	{ MA,	0,	NULL,			NULL		}, /* 475 */
	{ MA,	0,	NULL,			NULL		}, /* 476 */
	{ MA,	0,	NULL,			NULL		}, /* 477 */
	{ MA,	0,	NULL,			NULL		}, /* 478 */
	{ MA,	0,	NULL,			NULL		}, /* 479 */
	{ MA,	0,	NULL,			NULL		}, /* 480 */
	{ MA,	0,	NULL,			NULL		}, /* 481 */
	{ MA,	0,	NULL,			NULL		}, /* 482 */
	{ MA,	0,	NULL,			NULL		}, /* 483 */
	{ MA,	0,	NULL,			NULL		}, /* 484 */
	{ MA,	0,	NULL,			NULL		}, /* 485 */
	{ MA,	0,	NULL,			NULL		}, /* 486 */
	{ MA,	0,	NULL,			NULL		}, /* 487 */
	{ MA,	0,	NULL,			NULL		}, /* 488 */
	{ MA,	0,	NULL,			NULL		}, /* 489 */
	{ MA,	0,	NULL,			NULL		}, /* 490 */
	{ MA,	0,	NULL,			NULL		}, /* 491 */
	{ MA,	0,	NULL,			NULL		}, /* 492 */
	{ MA,	0,	NULL,			NULL		}, /* 493 */
	{ MA,	0,	NULL,			NULL		}, /* 494 */
	{ MA,	0,	NULL,			NULL		}, /* 495 */
	{ MA,	0,	NULL,			NULL		}, /* 496 */
	{ MA,	0,	NULL,			NULL		}, /* 497 */
	{ MA,	0,	NULL,			NULL		}, /* 498 */
	{ MA,	0,	NULL,			NULL		}, /* 499 */
	{ MA,	0,	NULL,			NULL		}, /* 500 */
	{ MA,	0,	NULL,			NULL		}, /* 501 */
	{ MA,	0,	NULL,			NULL		}, /* 502 */
	{ MA,	0,	NULL,			NULL		}, /* 503 */
	{ MA,	0,	NULL,			NULL		}, /* 504 */
	{ MA,	0,	NULL,			NULL		}, /* 505 */
	{ MA,	0,	NULL,			NULL		}, /* 506 */
	{ MA,	0,	NULL,			NULL		}, /* 507 */
	{ MA,	0,	NULL,			NULL		}, /* 508 */
	{ MA,	0,	NULL,			NULL		}, /* 509 */
	{ MA,	0,	NULL,			NULL		}, /* 510 */
	{ MA,	0,	NULL,			NULL		}, /* 511 */
	{ MA,	0,	NULL,			NULL		}, /* 512 */
	{ MA,	0,	NULL,			NULL		}, /* 513 */
	{ MA,	0,	NULL,			NULL		}, /* 514 */
	{ MA,	0,	NULL,			NULL		}, /* 515 */
	{ MA,	0,	NULL,			NULL		}, /* 516 */
	{ MA,	0,	NULL,			NULL		}, /* 517 */
	{ MA,	0,	NULL,			NULL		}, /* 518 */
	{ MA,	0,	NULL,			NULL		}, /* 519 */
	{ MA,	0,	NULL,			NULL		}, /* 520 */
	{ MA,	0,	NULL,			NULL		}, /* 521 */
	{ MA,	0,	NULL,			NULL		}, /* 522 */
	{ MA,	0,	NULL,			NULL		}, /* 523 */
	{ MA,	0,	NULL,			NULL		}, /* 524 */
	{ MA,	0,	NULL,			NULL		}, /* 525 */
	{ MA,	0,	NULL,			NULL		}, /* 526 */
	{ MA,	0,	NULL,			NULL		}, /* 527 */
	{ MA,	0,	NULL,			NULL		}, /* 528 */
	{ MA,	0,	NULL,			NULL		}, /* 529 */
	{ MA,	0,	NULL,			NULL		}, /* 530 */
	{ MA,	0,	NULL,			NULL		}, /* 531 */
	{ MA,	0,	NULL,			NULL		}, /* 532 */
	{ MA,	0,	NULL,			NULL		}, /* 533 */
	{ MA,	0,	NULL,			NULL		}, /* 534 */
	{ MA,	0,	NULL,			NULL		}, /* 535 */
	{ MA,	0,	NULL,			NULL		}, /* 536 */
	{ MA,	0,	NULL,			NULL		}, /* 537 */
	{ MA,	0,	NULL,			NULL		}, /* 538 */
	{ MA,	0,	NULL,			NULL		}, /* 539 */
	{ MA,	0,	NULL,			NULL		}, /* 540 */
	{ MA,	0,	NULL,			NULL		}, /* 541 */
	{ MA,	0,	NULL,			NULL		}, /* 542 */
	{ MA,	0,	NULL,			NULL		}, /* 543 */
	{ MA,	0,	NULL,			NULL		}, /* 544 */
	{ MA,	0,	NULL,			NULL		}, /* 545 */
	{ MA,	0,	NULL,			NULL		}, /* 546 */
	{ MA,	0,	NULL,			NULL		}, /* 547 */
	{ MA,	0,	NULL,			NULL		}, /* 548 */
	{ MA,	0,	NULL,			NULL		}, /* 549 */
	{ MA,	0,	NULL,			NULL		}, /* 550 */
	{ MA,	0,	NULL,			NULL		}, /* 551 */
	{ MA,	0,	NULL,			NULL		}, /* 552 */
	{ MA,	0,	NULL,			NULL		}, /* 553 */
	{ MA,	0,	NULL,			NULL		}, /* 554 */
	{ MA,	0,	NULL,			NULL		}, /* 555 */
	{ MA,	0,	NULL,			NULL		}, /* 556 */
	{ MA,	0,	NULL,			NULL		}, /* 557 */
	{ MA,	0,	NULL,			NULL		}, /* 558 */
	{ MA,	0,	NULL,			NULL		}, /* 559 */
	{ MA,	0,	NULL,			NULL		}, /* 560 */
	{ MA,	0,	NULL,			NULL		}, /* 561 */
	{ MA,	0,	NULL,			NULL		}, /* 562 */
	{ MA,	0,	NULL,			NULL		}, /* 563 */
	{ MA,	0,	NULL,			NULL		}, /* 564 */
	{ MA,	0,	NULL,			NULL		}, /* 565 */
	{ MA,	0,	NULL,			NULL		}, /* 566 */
	{ MA,	0,	NULL,			NULL		}, /* 567 */
	{ MA,	0,	NULL,			NULL		}, /* 568 */
	{ MA,	0,	NULL,			NULL		}, /* 569 */
	{ MA,	0,	NULL,			NULL		}, /* 570 */
	{ MA,	0,	NULL,			NULL		}, /* 571 */
	{ MA,	0,	NULL,			NULL		}, /* 572 */
	{ MA,	0,	NULL,			NULL		}, /* 573 */
	{ MA,	0,	NULL,			NULL		}, /* 574 */
	{ MA,	0,	NULL,			NULL		}, /* 575 */
	{ MA,	0,	NULL,			NULL		}, /* 576 */
	{ MA,	0,	NULL,			NULL		}, /* 577 */
	{ MA,	0,	NULL,			NULL		}, /* 578 */
	{ MA,	0,	NULL,			NULL		}, /* 579 */
	{ MA,	0,	NULL,			NULL		}, /* 580 */
	{ MA,	0,	NULL,			NULL		}, /* 581 */
	{ MA,	0,	NULL,			NULL		}, /* 582 */
	{ MA,	0,	NULL,			NULL		}, /* 583 */
	{ MA,	0,	NULL,			NULL		}, /* 584 */
	{ MA,	0,	NULL,			NULL		}, /* 585 */
	{ MA,	0,	NULL,			NULL		}, /* 586 */
	{ MA,	0,	NULL,			NULL		}, /* 587 */
	{ MA,	0,	NULL,			NULL		}, /* 588 */
	{ MA,	0,	NULL,			NULL		}, /* 589 */
	{ MA,	0,	NULL,			NULL		}, /* 590 */
	{ MA,	0,	NULL,			NULL		}, /* 591 */
	{ MA,	0,	NULL,			NULL		}, /* 592 */
	{ MA,	0,	NULL,			NULL		}, /* 593 */
	{ MA,	0,	NULL,			NULL		}, /* 594 */
	{ MA,	0,	NULL,			NULL		}, /* 595 */
	{ MA,	0,	NULL,			NULL		}, /* 596 */
	{ MA,	0,	NULL,			NULL		}, /* 597 */
	{ MA,	0,	NULL,			NULL		}, /* 598 */
	{ MA,	0,	NULL,			NULL		}, /* 599 */
	{ MA,	0,	NULL,			NULL		}, /* 600 */
	{ MA,	0,	NULL,			NULL		}, /* 601 */
	{ MA,	0,	NULL,			NULL		}, /* 602 */
	{ MA,	0,	NULL,			NULL		}, /* 603 */
	{ MA,	0,	NULL,			NULL		}, /* 604 */
	{ MA,	0,	NULL,			NULL		}, /* 605 */
	{ MA,	0,	NULL,			NULL		}, /* 606 */
	{ MA,	0,	NULL,			NULL		}, /* 607 */
	{ MA,	0,	NULL,			NULL		}, /* 608 */
	{ MA,	0,	NULL,			NULL		}, /* 609 */
	{ MA,	0,	NULL,			NULL		}, /* 610 */
	{ MA,	0,	NULL,			NULL		}, /* 611 */
	{ MA,	0,	NULL,			NULL		}, /* 612 */
	{ MA,	0,	NULL,			NULL		}, /* 613 */
	{ MA,	0,	NULL,			NULL		}, /* 614 */
	{ MA,	0,	NULL,			NULL		}, /* 615 */
	{ MA,	0,	NULL,			NULL		}, /* 616 */
	{ MA,	0,	NULL,			NULL		}, /* 617 */
	{ MA,	0,	NULL,			NULL		}, /* 618 */
	{ MA,	0,	NULL,			NULL		}, /* 619 */
	{ MA,	0,	NULL,			NULL		}, /* 620 */
	{ MA,	0,	NULL,			NULL		}, /* 621 */
	{ MA,	0,	NULL,			NULL		}, /* 622 */
	{ MA,	0,	NULL,			NULL		}, /* 623 */
	{ MA,	0,	NULL,			NULL		}, /* 624 */
	{ MA,	0,	NULL,			NULL		}, /* 625 */
	{ MA,	0,	NULL,			NULL		}, /* 626 */
	{ MA,	0,	NULL,			NULL		}, /* 627 */
	{ MA,	0,	NULL,			NULL		}, /* 628 */
	{ MA,	0,	NULL,			NULL		}, /* 629 */
	{ MA,	0,	NULL,			NULL		}, /* 630 */
	{ MA,	0,	NULL,			NULL		}, /* 631 */
	{ MA,	0,	NULL,			NULL		}, /* 632 */
	{ MA,	0,	NULL,			NULL		}, /* 633 */
	{ MA,	0,	NULL,			NULL		}, /* 634 */
	{ MA,	0,	NULL,			NULL		}, /* 635 */
	{ MA,	0,	NULL,			NULL		}, /* 636 */
	{ MA,	0,	NULL,			NULL		}, /* 637 */
	{ MA,	0,	NULL,			NULL		}, /* 638 */
	{ MA,	0,	NULL,			NULL		}, /* 639 */
	{ MA,	0,	NULL,			NULL		}, /* 640 */
	{ MA,	0,	NULL,			NULL		}, /* 641 */
	{ MA,	0,	NULL,			NULL		}, /* 642 */
	{ MA,	0,	NULL,			NULL		}, /* 643 */
	{ MA,	0,	NULL,			NULL		}, /* 644 */
	{ MA,	0,	NULL,			NULL		}, /* 645 */
	{ MA,	0,	NULL,			NULL		}, /* 646 */
	{ MA,	0,	NULL,			NULL		}, /* 647 */
	{ MA,	0,	NULL,			NULL		}, /* 648 */
	{ MA,	0,	NULL,			NULL		}, /* 649 */
	{ MA,	0,	NULL,			NULL		}, /* 650 */
	{ MA,	0,	NULL,			NULL		}, /* 651 */
	{ MA,	0,	NULL,			NULL		}, /* 652 */
	{ MA,	0,	NULL,			NULL		}, /* 653 */
	{ MA,	0,	NULL,			NULL		}, /* 654 */
	{ MA,	0,	NULL,			NULL		}, /* 655 */
	{ MA,	0,	NULL,			NULL		}, /* 656 */
	{ MA,	0,	NULL,			NULL		}, /* 657 */
	{ MA,	0,	NULL,			NULL		}, /* 658 */
	{ MA,	0,	NULL,			NULL		}, /* 659 */
	{ MA,	0,	NULL,			NULL		}, /* 660 */
	{ MA,	0,	NULL,			NULL		}, /* 661 */
	{ MA,	0,	NULL,			NULL		}, /* 662 */
	{ MA,	0,	NULL,			NULL		}, /* 663 */
	{ MA,	0,	NULL,			NULL		}, /* 664 */
	{ MA,	0,	NULL,			NULL		}, /* 665 */
	{ MA,	0,	NULL,			NULL		}, /* 666 */
	{ MA,	0,	NULL,			NULL		}, /* 667 */
	{ MA,	0,	NULL,			NULL		}, /* 668 */
	{ MA,	0,	NULL,			NULL		}, /* 669 */
	{ MA,	0,	NULL,			NULL		}, /* 670 */
	{ MA,	0,	NULL,			NULL		}, /* 671 */
	{ MA,	0,	NULL,			NULL		}, /* 672 */
	{ MA,	0,	NULL,			NULL		}, /* 673 */
	{ MA,	0,	NULL,			NULL		}, /* 674 */
	{ MA,	0,	NULL,			NULL		}, /* 675 */
	{ MA,	0,	NULL,			NULL		}, /* 676 */
	{ MA,	0,	NULL,			NULL		}, /* 677 */
	{ MA,	0,	NULL,			NULL		}, /* 678 */
	{ MA,	0,	NULL,			NULL		}, /* 679 */
	{ MA,	0,	NULL,			NULL		}, /* 680 */
	{ MA,	0,	NULL,			NULL		}, /* 681 */
	{ MA,	0,	NULL,			NULL		}, /* 682 */
	{ MA,	0,	NULL,			NULL		}, /* 683 */
	{ MA,	0,	NULL,			NULL		}, /* 684 */
	{ MA,	0,	NULL,			NULL		}, /* 685 */
	{ MA,	0,	NULL,			NULL		}, /* 686 */
	{ MA,	0,	NULL,			NULL		}, /* 687 */
	{ MA,	0,	NULL,			NULL		}, /* 688 */
	{ MA,	0,	NULL,			NULL		}, /* 689 */
	{ MA,	0,	NULL,			NULL		}, /* 690 */
	{ MA,	0,	NULL,			NULL		}, /* 691 */
	{ MA,	0,	NULL,			NULL		}, /* 692 */
	{ MA,	0,	NULL,			NULL		}, /* 693 */
	{ MA,	0,	NULL,			NULL		}, /* 694 */
	{ MA,	0,	NULL,			NULL		}, /* 695 */
	{ MA,	0,	NULL,			NULL		}, /* 696 */
	{ MA,	0,	NULL,			NULL		}, /* 697 */
	{ MA,	0,	NULL,			NULL		}, /* 698 */
	{ MA,	0,	NULL,			NULL		}, /* 699 */
	{ MA,	0,	NULL,			NULL		}, /* 700 */
	{ MA,	0,	NULL,			NULL		}, /* 701 */
	{ MA,	0,	NULL,			NULL		}, /* 702 */
	{ MA,	0,	NULL,			NULL		}, /* 703 */
	{ MA,	0,	NULL,			NULL		}, /* 704 */
	{ MA,	0,	NULL,			NULL		}, /* 705 */
	{ MA,	0,	NULL,			NULL		}, /* 706 */
	{ MA,	0,	NULL,			NULL		}, /* 707 */
	{ MA,	0,	NULL,			NULL		}, /* 708 */
	{ MA,	0,	NULL,			NULL		}, /* 709 */
	{ MA,	0,	NULL,			NULL		}, /* 710 */
	{ MA,	0,	NULL,			NULL		}, /* 711 */
	{ MA,	0,	NULL,			NULL		}, /* 712 */
	{ MA,	0,	NULL,			NULL		}, /* 713 */
	{ MA,	0,	NULL,			NULL		}, /* 714 */
	{ MA,	0,	NULL,			NULL		}, /* 715 */
	{ MA,	0,	NULL,			NULL		}, /* 716 */
	{ MA,	0,	NULL,			NULL		}, /* 717 */
	{ MA,	0,	NULL,			NULL		}, /* 718 */
	{ MA,	0,	NULL,			NULL		}, /* 719 */
	{ MA,	0,	NULL,			NULL		}, /* 720 */
	{ MA,	0,	NULL,			NULL		}, /* 721 */
	{ MA,	0,	NULL,			NULL		}, /* 722 */
	{ MA,	0,	NULL,			NULL		}, /* 723 */
	{ MA,	0,	NULL,			NULL		}, /* 724 */
	{ MA,	0,	NULL,			NULL		}, /* 725 */
	{ MA,	0,	NULL,			NULL		}, /* 726 */
	{ MA,	0,	NULL,			NULL		}, /* 727 */
	{ MA,	0,	NULL,			NULL		}, /* 728 */
	{ MA,	0,	NULL,			NULL		}, /* 729 */
	{ MA,	0,	NULL,			NULL		}, /* 730 */
	{ MA,	0,	NULL,			NULL		}, /* 731 */
	{ MA,	0,	NULL,			NULL		}, /* 732 */
	{ MA,	0,	NULL,			NULL		}, /* 733 */
	{ MA,	0,	NULL,			NULL		}, /* 734 */
	{ MA,	0,	NULL,			NULL		}, /* 735 */
	{ MA,	0,	NULL,			NULL		}, /* 736 */
	{ MA,	0,	NULL,			NULL		}, /* 737 */
	{ MA,	0,	NULL,			NULL		}, /* 738 */
	{ MA,	0,	NULL,			NULL		}, /* 739 */
	{ MA,	0,	NULL,			NULL		}, /* 740 */
	{ MA,	0,	NULL,			NULL		}, /* 741 */
	{ MA,	0,	NULL,			NULL		}, /* 742 */
	{ MA,	0,	NULL,			NULL		}, /* 743 */
	{ MA,	0,	NULL,			NULL		}, /* 744 */
	{ MA,	0,	NULL,			NULL		}, /* 745 */
	{ MA,	0,	NULL,			NULL		}, /* 746 */
	{ MA,	0,	NULL,			NULL		}, /* 747 */
	{ MA,	0,	NULL,			NULL		}, /* 748 */
	{ MA,	0,	NULL,			NULL		}, /* 749 */
	{ MA,	0,	NULL,			NULL		}, /* 750 */
	{ MA,	0,	NULL,			NULL		}, /* 751 */
	{ MA,	0,	NULL,			NULL		}, /* 752 */
	{ MA,	0,	NULL,			NULL		}, /* 753 */
	{ MA,	0,	NULL,			NULL		}, /* 754 */
	{ MA,	0,	NULL,			NULL		}, /* 755 */
	{ MA,	0,	NULL,			NULL		}, /* 756 */
	{ MA,	0,	NULL,			NULL		}, /* 757 */
	{ MA,	0,	NULL,			NULL		}, /* 758 */
	{ MA,	0,	NULL,			NULL		}, /* 759 */
	{ MA,	0,	NULL,			NULL		}, /* 760 */
	{ MA,	0,	NULL,			NULL		}, /* 761 */
	{ MA,	0,	NULL,			NULL		}, /* 762 */
	{ MA,	0,	NULL,			NULL		}, /* 763 */
	{ MA,	0,	NULL,			NULL		}, /* 764 */
	{ MA,	0,	NULL,			NULL		}, /* 765 */
	{ MA,	0,	NULL,			NULL		}, /* 766 */
	{ MA,	0,	NULL,			NULL		}, /* 767 */
	{ MA,	0,	NULL,			NULL		}, /* 768 */
	{ MA,	0,	NULL,			NULL		}, /* 769 */
	{ MA,	0,	NULL,			NULL		}, /* 770 */
	{ MA,	0,	NULL,			NULL		}, /* 771 */
	{ MA,	0,	NULL,			NULL		}, /* 772 */
	{ MA,	0,	NULL,			NULL		}, /* 773 */
	{ MA,	0,	NULL,			NULL		}, /* 774 */
	{ MA,	0,	NULL,			NULL		}, /* 775 */
	{ MA,	0,	NULL,			NULL		}, /* 776 */
	{ MA,	0,	NULL,			NULL		}, /* 777 */
	{ MA,	0,	NULL,			NULL		}, /* 778 */
	{ MA,	0,	NULL,			NULL		}, /* 779 */
	{ MA,	0,	NULL,			NULL		}, /* 780 */
	{ MA,	0,	NULL,			NULL		}, /* 781 */
	{ MA,	0,	NULL,			NULL		}, /* 782 */
	{ MA,	0,	NULL,			NULL		}, /* 783 */
	{ MA,	0,	NULL,			NULL		}, /* 784 */
	{ MA,	0,	NULL,			NULL		}, /* 785 */
	{ MA,	0,	NULL,			NULL		}, /* 786 */
	{ MA,	0,	NULL,			NULL		}, /* 787 */
	{ MA,	0,	NULL,			NULL		}, /* 788 */
	{ MA,	0,	NULL,			NULL		}, /* 789 */
	{ MA,	0,	NULL,			NULL		}, /* 790 */
	{ MA,	0,	NULL,			NULL		}, /* 791 */
	{ MA,	0,	NULL,			NULL		}, /* 792 */
	{ MA,	0,	NULL,			NULL		}, /* 793 */
	{ MA,	0,	NULL,			NULL		}, /* 794 */
	{ MA,	0,	NULL,			NULL		}, /* 795 */
	{ MA,	0,	NULL,			NULL		}, /* 796 */
	{ MA,	0,	NULL,			NULL		}, /* 797 */
	{ MA,	0,	NULL,			NULL		}, /* 798 */
	{ MA,	0,	NULL,			NULL		}, /* 799 */
	{ MA,	0,	NULL,			NULL		}, /* 800 */
	{ MA,	0,	NULL,			NULL		}, /* 801 */
	{ MA,	0,	NULL,			NULL		}, /* 802 */
	{ MA,	0,	NULL,			NULL		}, /* 803 */
	{ MA,	0,	NULL,			NULL		}, /* 804 */
	{ MA,	0,	NULL,			NULL		}, /* 805 */
	{ MA,	0,	NULL,			NULL		}, /* 806 */
	{ MA,	0,	NULL,			NULL		}, /* 807 */
	{ MA,	0,	NULL,			NULL		}, /* 808 */
	{ MA,	0,	NULL,			NULL		}, /* 809 */
	{ MA,	0,	NULL,			NULL		}, /* 810 */
	{ MA,	0,	NULL,			NULL		}, /* 811 */
	{ MA,	0,	NULL,			NULL		}, /* 812 */
	{ MA,	0,	NULL,			NULL		}, /* 813 */
	{ MA,	0,	NULL,			NULL		}, /* 814 */
	{ MA,	0,	NULL,			NULL		}, /* 815 */
	{ MA,	0,	NULL,			NULL		}, /* 816 */
	{ MA,	0,	NULL,			NULL		}, /* 817 */
	{ MA,	0,	NULL,			NULL		}, /* 818 */
	{ MA,	0,	NULL,			NULL		}, /* 819 */
	{ MA,	0,	NULL,			NULL		}, /* 820 */
	{ MA,	0,	NULL,			NULL		}, /* 821 */
	{ MA,	0,	NULL,			NULL		}, /* 822 */
	{ MA,	0,	NULL,			NULL		}, /* 823 */
	{ MA,	0,	NULL,			NULL		}, /* 824 */
	{ MA,	0,	NULL,			NULL		}, /* 825 */
	{ MA,	0,	NULL,			NULL		}, /* 826 */
	{ MA,	0,	NULL,			NULL		}, /* 827 */
	{ MA,	0,	NULL,			NULL		}, /* 828 */
	{ MA,	0,	NULL,			NULL		}, /* 829 */
	{ MA,	0,	NULL,			NULL		}, /* 830 */
	{ MA,	0,	NULL,			NULL		}, /* 831 */
	{ MA,	0,	NULL,			NULL		}, /* 832 */
	{ MA,	0,	NULL,			NULL		}, /* 833 */
	{ MA,	0,	NULL,			NULL		}, /* 834 */
	{ MA,	0,	NULL,			NULL		}, /* 835 */
	{ MA,	0,	NULL,			NULL		}, /* 836 */
	{ MA,	0,	NULL,			NULL		}, /* 837 */
	{ MA,	0,	NULL,			NULL		}, /* 838 */
	{ MA,	0,	NULL,			NULL		}, /* 839 */
	{ MA,	0,	NULL,			NULL		}, /* 840 */
	{ MA,	0,	NULL,			NULL		}, /* 841 */
	{ MA,	0,	NULL,			NULL		}, /* 842 */
	{ MA,	0,	NULL,			NULL		}, /* 843 */
	{ MA,	0,	NULL,			NULL		}, /* 844 */
	{ MA,	0,	NULL,			NULL		}, /* 845 */
	{ MA,	0,	NULL,			NULL		}, /* 846 */
	{ MA,	0,	NULL,			NULL		}, /* 847 */
	{ MA,	0,	NULL,			NULL		}, /* 848 */
	{ MA,	0,	NULL,			NULL		}, /* 849 */
	{ MA,	0,	NULL,			NULL		}, /* 850 */
	{ MA,	0,	NULL,			NULL		}, /* 851 */
	{ MA,	0,	NULL,			NULL		}, /* 852 */
	{ MA,	0,	NULL,			NULL		}, /* 853 */
	{ MA,	0,	NULL,			NULL		}, /* 854 */
	{ MA,	0,	NULL,			NULL		}, /* 855 */
	{ MA,	0,	NULL,			NULL		}, /* 856 */
	{ MA,	0,	NULL,			NULL		}, /* 857 */
	{ MA,	0,	NULL,			NULL		}, /* 858 */
	{ MA,	0,	NULL,			NULL		}, /* 859 */
	{ MA,	0,	NULL,			NULL		}, /* 860 */
	{ MA,	0,	NULL,			NULL		}, /* 861 */
	{ MA,	0,	NULL,			NULL		}, /* 862 */
	{ MA,	0,	NULL,			NULL		}, /* 863 */
	{ MA,	0,	NULL,			NULL		}, /* 864 */
	{ MA,	0,	NULL,			NULL		}, /* 865 */
	{ MA,	0,	NULL,			NULL		}, /* 866 */
	{ MA,	0,	NULL,			NULL		}, /* 867 */
	{ MA,	0,	NULL,			NULL		}, /* 868 */
	{ MA,	0,	NULL,			NULL		}, /* 869 */
	{ MA,	0,	NULL,			NULL		}, /* 870 */
	{ MA,	0,	NULL,			NULL		}, /* 871 */
	{ MA,	0,	NULL,			NULL		}, /* 872 */
	{ MA,	0,	NULL,			NULL		}, /* 873 */
	{ MA,	0,	NULL,			NULL		}, /* 874 */
	{ MA,	0,	NULL,			NULL		}, /* 875 */
	{ MA,	0,	NULL,			NULL		}, /* 876 */
	{ MA,	0,	NULL,			NULL		}, /* 877 */
	{ MA,	0,	NULL,			NULL		}, /* 878 */
	{ MA,	0,	NULL,			NULL		}, /* 879 */
	{ MA,	0,	NULL,			NULL		}, /* 880 */
	{ MA,	0,	NULL,			NULL		}, /* 881 */
	{ MA,	0,	NULL,			NULL		}, /* 882 */
	{ MA,	0,	NULL,			NULL		}, /* 883 */
	{ MA,	0,	NULL,			NULL		}, /* 884 */
	{ MA,	0,	NULL,			NULL		}, /* 885 */
	{ MA,	0,	NULL,			NULL		}, /* 886 */
	{ MA,	0,	NULL,			NULL		}, /* 887 */
	{ MA,	0,	NULL,			NULL		}, /* 888 */
	{ MA,	0,	NULL,			NULL		}, /* 889 */
	{ MA,	0,	NULL,			NULL		}, /* 890 */
	{ MA,	0,	NULL,			NULL		}, /* 891 */
	{ MA,	0,	NULL,			NULL		}, /* 892 */
	{ MA,	0,	NULL,			NULL		}, /* 893 */
	{ MA,	0,	NULL,			NULL		}, /* 894 */
	{ MA,	0,	NULL,			NULL		}, /* 895 */
	{ MA,	0,	NULL,			NULL		}, /* 896 */
	{ MA,	0,	NULL,			NULL		}, /* 897 */
	{ MA,	0,	NULL,			NULL		}, /* 898 */
	{ MA,	0,	NULL,			NULL		}, /* 899 */
	{ MA,	0,	NULL,			NULL		}, /* 900 */
	{ MA,	0,	NULL,			NULL		}, /* 901 */
	{ MA,	0,	NULL,			NULL		}, /* 902 */
	{ MA,	0,	NULL,			NULL		}, /* 903 */
	{ MA,	0,	NULL,			NULL		}, /* 904 */
	{ MA,	0,	NULL,			NULL		}, /* 905 */
	{ MA,	0,	NULL,			NULL		}, /* 906 */
	{ MA,	0,	NULL,			NULL		}, /* 907 */
	{ MA,	0,	NULL,			NULL		}, /* 908 */
	{ MA,	0,	NULL,			NULL		}, /* 909 */
	{ MA,	0,	NULL,			NULL		}, /* 910 */
	{ MA,	0,	NULL,			NULL		}, /* 911 */
	{ MA,	0,	NULL,			NULL		}, /* 912 */
	{ MA,	0,	NULL,			NULL		}, /* 913 */
	{ MA,	0,	NULL,			NULL		}, /* 914 */
	{ MA,	0,	NULL,			NULL		}, /* 915 */
	{ MA,	0,	NULL,			NULL		}, /* 916 */
	{ MA,	0,	NULL,			NULL		}, /* 917 */
	{ MA,	0,	NULL,			NULL		}, /* 918 */
	{ MA,	0,	NULL,			NULL		}, /* 919 */
	{ MA,	0,	NULL,			NULL		}, /* 920 */
	{ MA,	0,	NULL,			NULL		}, /* 921 */
	{ MA,	0,	NULL,			NULL		}, /* 922 */
	{ MA,	0,	NULL,			NULL		}, /* 923 */
	{ MA,	0,	NULL,			NULL		}, /* 924 */
	{ MA,	0,	NULL,			NULL		}, /* 925 */
	{ MA,	0,	NULL,			NULL		}, /* 926 */
	{ MA,	0,	NULL,			NULL		}, /* 927 */
	{ MA,	0,	NULL,			NULL		}, /* 928 */
	{ MA,	0,	NULL,			NULL		}, /* 929 */
	{ MA,	0,	NULL,			NULL		}, /* 930 */
	{ MA,	0,	NULL,			NULL		}, /* 931 */
	{ MA,	0,	NULL,			NULL		}, /* 932 */
	{ MA,	0,	NULL,			NULL		}, /* 933 */
	{ MA,	0,	NULL,			NULL		}, /* 934 */
	{ MA,	0,	NULL,			NULL		}, /* 935 */
	{ MA,	0,	NULL,			NULL		}, /* 936 */
	{ MA,	0,	NULL,			NULL		}, /* 937 */
	{ MA,	0,	NULL,			NULL		}, /* 938 */
	{ MA,	0,	NULL,			NULL		}, /* 939 */
	{ MA,	0,	NULL,			NULL		}, /* 940 */
	{ MA,	0,	NULL,			NULL		}, /* 941 */
	{ MA,	0,	NULL,			NULL		}, /* 942 */
	{ MA,	0,	NULL,			NULL		}, /* 943 */
	{ MA,	0,	NULL,			NULL		}, /* 944 */
	{ MA,	0,	NULL,			NULL		}, /* 945 */
	{ MA,	0,	NULL,			NULL		}, /* 946 */
	{ MA,	0,	NULL,			NULL		}, /* 947 */
	{ MA,	0,	NULL,			NULL		}, /* 948 */
	{ MA,	0,	NULL,			NULL		}, /* 949 */
	{ MA,	0,	NULL,			NULL		}, /* 950 */
	{ MA,	0,	NULL,			NULL		}, /* 951 */
	{ MA,	0,	NULL,			NULL		}, /* 952 */
	{ MA,	0,	NULL,			NULL		}, /* 953 */
	{ MA,	0,	NULL,			NULL		}, /* 954 */
	{ MA,	0,	NULL,			NULL		}, /* 955 */
	{ MA,	0,	NULL,			NULL		}, /* 956 */
	{ MA,	0,	NULL,			NULL		}, /* 957 */
	{ MA,	0,	NULL,			NULL		}, /* 958 */
	{ MA,	0,	NULL,			NULL		}, /* 959 */
	{ MA,	0,	NULL,			NULL		}, /* 960 */
	{ MA,	0,	NULL,			NULL		}, /* 961 */
	{ MA,	0,	NULL,			NULL		}, /* 962 */
	{ MA,	0,	NULL,			NULL		}, /* 963 */
	{ MA,	0,	NULL,			NULL		}, /* 964 */
	{ MA,	0,	NULL,			NULL		}, /* 965 */
	{ MA,	0,	NULL,			NULL		}, /* 966 */
	{ MA,	0,	NULL,			NULL		}, /* 967 */
	{ MA,	0,	NULL,			NULL		}, /* 968 */
	{ MA,	0,	NULL,			NULL		}, /* 969 */
	{ MA,	0,	NULL,			NULL		}, /* 970 */
	{ MA,	0,	NULL,			NULL		}, /* 971 */
	{ MA,	0,	NULL,			NULL		}, /* 972 */
	{ MA,	0,	NULL,			NULL		}, /* 973 */
	{ MA,	0,	NULL,			NULL		}, /* 974 */
	{ MA,	0,	NULL,			NULL		}, /* 975 */
	{ MA,	0,	NULL,			NULL		}, /* 976 */
	{ MA,	0,	NULL,			NULL		}, /* 977 */
	{ MA,	0,	NULL,			NULL		}, /* 978 */
	{ MA,	0,	NULL,			NULL		}, /* 979 */
	{ MA,	0,	NULL,			NULL		}, /* 980 */
	{ MA,	0,	NULL,			NULL		}, /* 981 */
	{ MA,	0,	NULL,			NULL		}, /* 982 */
	{ MA,	0,	NULL,			NULL		}, /* 983 */
	{ MA,	0,	NULL,			NULL		}, /* 984 */
	{ MA,	0,	NULL,			NULL		}, /* 985 */
	{ MA,	0,	NULL,			NULL		}, /* 986 */
	{ MA,	0,	NULL,			NULL		}, /* 987 */
	{ MA,	0,	NULL,			NULL		}, /* 988 */
	{ MA,	0,	NULL,			NULL		}, /* 989 */
	{ MA,	0,	NULL,			NULL		}, /* 990 */
	{ MA,	0,	NULL,			NULL		}, /* 991 */
	{ MA,	0,	NULL,			NULL		}, /* 992 */
	{ MA,	0,	NULL,			NULL		}, /* 993 */
	{ MA,	0,	NULL,			NULL		}, /* 994 */
	{ MA,	0,	NULL,			NULL		}, /* 995 */
	{ MA,	0,	NULL,			NULL		}, /* 996 */
	{ MA,	0,	NULL,			NULL		}, /* 997 */
	{ MA,	0,	NULL,			NULL		}, /* 998 */
	{ MA,	0,	NULL,			NULL		}, /* 999 */
	{ MA,	0,	NULL,			NULL		}, /* 1000 */
	{ MA,	0,	NULL,			NULL		}, /* 1001 */
	{ MA,	0,	NULL,			NULL		}, /* 1002 */
	{ MA,	0,	NULL,			NULL		}, /* 1003 */
	{ MA,	0,	NULL,			NULL		}, /* 1004 */
	{ MA,	0,	NULL,			NULL		}, /* 1005 */
	{ MA,	0,	NULL,			NULL		}, /* 1006 */
	{ MA,	0,	NULL,			NULL		}, /* 1007 */
	{ MA,	0,	NULL,			NULL		}, /* 1008 */
	{ MA,	0,	NULL,			NULL		}, /* 1009 */
	{ MA,	0,	NULL,			NULL		}, /* 1010 */
	{ MA,	0,	NULL,			NULL		}, /* 1011 */
	{ MA,	0,	NULL,			NULL		}, /* 1012 */
	{ MA,	0,	NULL,			NULL		}, /* 1013 */
	{ MA,	0,	NULL,			NULL		}, /* 1014 */
	{ MA,	0,	NULL,			NULL		}, /* 1015 */
	{ MA,	0,	NULL,			NULL		}, /* 1016 */
	{ MA,	0,	NULL,			NULL		}, /* 1017 */
	{ MA,	0,	NULL,			NULL		}, /* 1018 */
	{ MA,	0,	NULL,			NULL		}, /* 1019 */
	{ MA,	0,	NULL,			NULL		}, /* 1020 */
	{ MA,	0,	NULL,			NULL		}, /* 1021 */
	{ MA,	0,	NULL,			NULL		}, /* 1022 */
	{ MA,	0,	NULL,			NULL		}, /* 1023 */
	{ 0,	0,	printargs,		"ni_syscall"	}, /* 1024 */
	{ 1,	TP,	sys_exit,		"exit"		}, /* 1025 */
	{ 3,	TD,	sys_read,		"read"		}, /* 1026 */
	{ 3,	TD,	sys_write,		"write"		}, /* 1027 */
	{ 3,	TD|TF,	sys_open,		"open"		}, /* 1028 */
	{ 1,	TD,	sys_close,		"close"		}, /* 1029 */
	{ 2,	TD|TF,	sys_creat,		"creat"		}, /* 1030 */
	{ 2,	TF,	sys_link,		"link"		}, /* 1031 */
	{ 1,	TF,	sys_unlink,		"unlink"	}, /* 1032 */
	{ 3,	TF|TP,	sys_execve,		"execve"	}, /* 1033 */
	{ 1,	TF,	sys_chdir,		"chdir"		}, /* 1034 */
	{ 1,	TD,	sys_fchdir,		"fchdir"	}, /* 1035 */
	{ 2,	TF,	sys_utimes,		"utimes"	}, /* 1036 */
	{ 3,	TF,	sys_mknod,		"mknod"		}, /* 1037 */
	{ 2,	TF,	sys_chmod,		"chmod"		}, /* 1038 */
	{ 3,	TF,	sys_chown,		"chown"		}, /* 1039 */
	{ 3,	TD,	sys_lseek,		"lseek"		}, /* 1040 */
	{ 0,	0,	sys_getpid,		"getpid"	}, /* 1041 */
	{ 0,	0,	sys_getppid,		"getppid"	}, /* 1042 */
	{ 5,	TF,	sys_mount,		"mount"		}, /* 1043 */
	{ 1,	TF,	sys_umount2,		"umount"	}, /* 1044 */
	{ 1,	0,	sys_setuid,		"setuid"	}, /* 1045 */
	{ 0,	NF,	sys_getuid,		"getuid"	}, /* 1046 */
	{ 0,	NF,	sys_geteuid,		"geteuid"	}, /* 1047 */
	{ 4,	0,	sys_ptrace,		"ptrace"	}, /* 1048 */
	{ 2,	TF,	sys_access,		"access"	}, /* 1049 */
	{ 0,	0,	sys_sync,		"sync"		}, /* 1050 */
	{ 1,	TD,	sys_fsync,		"fsync"		}, /* 1051 */
	{ 1,	TD,	sys_fdatasync,		"fdatasync"	}, /* 1052 */
	{ 2,	TS,	sys_kill,		"kill"		}, /* 1053 */
	{ 2,	TF,	sys_rename,		"rename"	}, /* 1054 */
	{ 2,	TF,	sys_mkdir,		"mkdir"		}, /* 1055 */
	{ 1,	TF,	sys_rmdir,		"rmdir"		}, /* 1056 */
	{ 1,	TD,	sys_dup,		"dup"		}, /* 1057 */
	{ 1,	TD,	sys_pipe,		"pipe"		}, /* 1058 */
	{ 1,	0,	sys_times,		"times"		}, /* 1059 */
	{ 1,	TM,	sys_brk,		"brk"		}, /* 1060 */
	{ 1,	0,	sys_setgid,		"setgid"	}, /* 1061 */
	{ 0,	NF,	sys_getgid,		"getgid"	}, /* 1062 */
	{ 0,	NF,	sys_getegid,		"getegid"	}, /* 1063 */
	{ 1,	TF,	sys_acct,		"acct"		}, /* 1064 */
	{ 3,	TD,	sys_ioctl,		"ioctl"		}, /* 1065 */
	{ 3,	TD,	sys_fcntl,		"fcntl"		}, /* 1066 */
	{ 1,	0,	sys_umask,		"umask"		}, /* 1067 */
	{ 1,	TF,	sys_chroot,		"chroot"	}, /* 1068 */
	{ 2,	0,	sys_ustat,		"ustat"		}, /* 1069 */
	{ 2,	TD,	sys_dup2,		"dup2"		}, /* 1070 */
	{ 2,	0,	sys_setreuid,		"setreuid"	}, /* 1071 */
	{ 2,	0,	sys_setregid,		"setregid"	}, /* 1072 */
	{ 3,	0,	printargs,		"getresuid"	}, /* 1073 */
	{ 3,	0,	sys_setresuid,		"setresuid"	}, /* 1074 */
	{ 3,	0,	sys_getresuid,		"getresgid"	}, /* 1075 */
	{ 3,	0,	printargs,		"setresgid"	}, /* 1076 */
	{ 2,	0,	sys_getgroups,		"getgroups"	}, /* 1077 */
	{ 2,	0,	sys_setgroups,		"setgroups"	}, /* 1078 */
	{ 1,	0,	sys_getpgid,		"getpgid"	}, /* 1079 */
	{ 2,	0,	sys_setpgid,		"setpgid"	}, /* 1080 */
	{ 0,	0,	sys_setsid,		"setsid"	}, /* 1081 */
	{ 1,	0,	sys_getsid,		"getsid"	}, /* 1082 */
	{ 2,	0,	sys_sethostname,	"sethostname"	}, /* 1083 */
	{ 2,	0,	sys_setrlimit,		"setrlimit"	}, /* 1084 */
	{ 2,	0,	sys_getrlimit,		"getrlimit"	}, /* 1085 */
	{ 2,	0,	sys_getrusage,		"getrusage"	}, /* 1086 */
	{ 2,	0,	sys_gettimeofday,	"gettimeofday"	}, /* 1087 */
	{ 2,	0,	sys_settimeofday,	"settimeofday"	}, /* 1088 */
	{ 5,	TD,	sys_select,		"select"	}, /* 1089 */
	{ 3,	TD,	sys_poll,		"poll"		}, /* 1090 */
	{ 2,	TF,	sys_symlink,		"symlink"	}, /* 1091 */
	{ 3,	TF,	sys_readlink,		"readlink"	}, /* 1092 */
	{ 1,	TF,	sys_uselib,		"uselib"	}, /* 1093 */
	{ 2,	TF,	sys_swapon,		"swapon"	}, /* 1094 */
	{ 1,	TF,	sys_swapoff,		"swapoff"	}, /* 1095 */
	{ 4,	0,	sys_reboot,		"reboot"	}, /* 1096 */
	{ 2,	TF,	sys_truncate,		"truncate"	}, /* 1097 */
	{ 2,	TD,	sys_ftruncate,		"ftruncate"	}, /* 1098 */
	{ 2,	TD,	sys_fchmod,		"fchmod"	}, /* 1099 */
	{ 3,	TD,	sys_fchown,		"fchown"	}, /* 1100 */
	{ 2,	0,	sys_getpriority,	"getpriority"	}, /* 1101 */
	{ 3,	0,	sys_setpriority,	"setpriority"	}, /* 1102 */
	{ 2,	TF,	sys_statfs,		"statfs"	}, /* 1103 */
	{ 2,	TD,	sys_fstatfs,		"fstatfs"	}, /* 1104 */
	{ 3,	0,	sys_gettid,		"gettid"	}, /* 1105 */
	{ 3,	TI,	sys_semget,		"semget"	}, /* 1106 */
	{ 3,	TI,	printargs,		"semop"		}, /* 1107 */
	{ 4,	TI,	sys_semctl,		"semctl"	}, /* 1108 */
	{ 2,	TI,	sys_msgget,		"msgget"	}, /* 1109 */
	{ 4,	TI,	sys_msgsnd,		"msgsnd"	}, /* 1110 */
	{ 5,	TI,	sys_msgrcv,		"msgrcv"	}, /* 1111 */
	{ 3,	TI,	sys_msgctl,		"msgctl"	}, /* 1112 */
	{ 3,	TI,	sys_shmget,		"shmget"	}, /* 1113 */
	{ 3,	TI,	sys_shmat,		"shmat"		}, /* 1114 */
	{ 1,	TI,	sys_shmdt,		"shmdt"		}, /* 1115 */
	{ 3,	TI,	sys_shmctl,		"shmctl"	}, /* 1116 */
	{ 3,	0,	sys_syslog,		"syslog"	}, /* 1117 */
	{ 3,	0,	sys_setitimer,		"setitimer"	}, /* 1118 */
	{ 2,	0,	sys_getitimer,		"getitimer"	}, /* 1119 */
	{ 2,	TF,	sys_stat,		"stat"		}, /* 1120 */
	{ 2,	TF,	sys_lstat,		"lstat"		}, /* 1121 */
	{ 2,	TD,	sys_fstat,		"fstat"		}, /* 1122 */
	{ 0,	0,	sys_vhangup,		"vhangup"	}, /* 1123 */
	{ 3,	TF,	sys_chown,		"lchown"	}, /* 1124 */
	{ 5,	0,	sys_vm86,		"vm86"		}, /* 1125 */
	{ 4,	TP,	sys_wait4,		"wait4"		}, /* 1126 */
	{ 1,	0,	sys_sysinfo,		"sysinfo"	}, /* 1127 */
	{ 5,	TP,	sys_clone,		"clone"		}, /* 1128 */
	{ 2,	0,	sys_setdomainname,	"setdomainname"	}, /* 1129 */
	{ 1,	0,	sys_uname,		"uname"		}, /* 1130 */
	{ 1,	0,	sys_adjtimex,		"adjtimex"	}, /* 1131 */
	{ 2,	0,	sys_create_module,	"create_module"	}, /* 1132 */
	{ 4,	0,	sys_init_module,	"init_module"	}, /* 1133 */
	{ 2,	0,	sys_delete_module,	"delete_module"	}, /* 1134 */
	{ 1,	0,	sys_get_kernel_syms,	"get_kernel_syms"}, /* 1135 */
	{ 5,	0,	sys_query_module,	"query_module"	}, /* 1136 */
	{ 4,	TF,	sys_quotactl,		"quotactl"	}, /* 1137 */
	{ 0,	0,	sys_bdflush,		"bdflush"	}, /* 1138 */
	{ 3,	0,	sys_sysfs,		"sysfs"		}, /* 1139 */
	{ 1,	0,	sys_personality,	"personality"	}, /* 1140 */
	{ 5,	0,	sys_afs_syscall,	"afs_syscall"	}, /* 1141 */
	{ 1,	NF,	sys_setfsuid,		"setfsuid"	}, /* 1142 */
	{ 1,	NF,	sys_setfsgid,		"setfsgid"	}, /* 1143 */
	{ 3,	TD,	sys_getdents,		"getdents"	}, /* 1144 */
	{ 2,	TD,	sys_flock,		"flock"		}, /* 1145 */
	{ 5,	TD,	sys_readv,		"readv"		}, /* 1146 */
	{ 5,	TD,	sys_writev,		"writev"	}, /* 1147 */
	{ 4,	TD,	sys_pread,		"pread"		}, /* 1148 */
	{ 4,	TD,	sys_pwrite,		"pwrite"	}, /* 1149 */
	{ 1,	0,	printargs,		"_sysctl"	}, /* 1150 */
	{ 6,	TD|TM,	sys_mmap,		"mmap"		}, /* 1151 */
	{ 2,	TM,	sys_munmap,		"munmap"	}, /* 1152 */
	{ 2,	TM,	sys_mlock,		"mlock"		}, /* 1153 */
	{ 1,	TM,	sys_mlockall,		"mlockall"	}, /* 1154 */
	{ 3,	TM,	sys_mprotect,		"mprotect"	}, /* 1155 */
	{ 5,	TM,	sys_mremap,		"mremap"	}, /* 1156 */
	{ 3,	TM,	sys_msync,		"msync"		}, /* 1157 */
	{ 2,	TM,	sys_munlock,		"munlock"	}, /* 1158 */
	{ 0,	TM,	sys_munlockall,		"munlockall"	}, /* 1159 */
	{ 2,	0,	sys_sched_getparam,	"sched_getparam"}, /* 1160 */
	{ 2,	0,	sys_sched_setparam,	"sched_setparam"}, /* 1161 */
	{ 2,	0,	sys_sched_getscheduler,	"sched_getscheduler"}, /* 1162 */
	{ 3,	0,	sys_sched_setscheduler,	"sched_setscheduler"}, /* 1163 */
	{ 0,	0,	sys_sched_yield,	"sched_yield"	}, /* 1164 */
	{ 1,	0,	sys_sched_get_priority_max,"sched_get_priority_max"}, /* 1165 */
	{ 1,	0,	sys_sched_get_priority_min,"sched_get_priority_min"}, /* 1166 */
	{ 2,	0,	sys_sched_rr_get_interval,"sched_rr_get_interval"}, /* 1167 */
	{ 2,	0,	sys_nanosleep,		"nanosleep"	}, /* 1168 */
	{ 3,	0,	sys_nfsservctl,		"nfsservctl"	}, /* 1169 */
	{ 5,	0,	sys_prctl,		"prctl"		}, /* 1170 */
	{ 1,	0,	sys_getpagesize,	"getpagesize"	}, /* 1171 */
	{ 6,	TD|TM,	sys_mmap_pgoff,		"mmap2"		}, /* 1172 */
	{ 5,	0,	printargs,		"pciconfig_read"}, /* 1173 */
	{ 5,	0,	printargs,		"pciconfig_write"}, /* 1174 */
	{ MA,	0,	printargs,		"perfmonctl"	}, /* 1175 */
	{ 2,	TS,	sys_sigaltstack,	"sigaltstack"	}, /* 1176 */
	{ 4,	TS,	sys_rt_sigaction,	"rt_sigaction"	}, /* 1177 */
	{ 2,	TS,	sys_rt_sigpending,	"rt_sigpending"	}, /* 1178 */
	{ 4,	TS,	sys_rt_sigprocmask,	"rt_sigprocmask"}, /* 1179 */
	{ 3,	TS,	sys_rt_sigqueueinfo,	"rt_sigqueueinfo"}, /* 1180 */
	{ 0,	TS,	sys_sigreturn,		"rt_sigreturn"	}, /* 1181 */
	{ 2,	TS,	sys_rt_sigsuspend,	"rt_sigsuspend"	}, /* 1182 */
	{ 4,	TS,	sys_rt_sigtimedwait,	"rt_sigtimedwait"}, /* 1183 */
	{ 2,	TF,	sys_getcwd,		"getcwd"	}, /* 1184 */
	{ 2,	0,	sys_capget,		"capget"	}, /* 1185 */
	{ 2,	0,	sys_capset,		"capset"	}, /* 1186 */
	{ 4,	TD|TN,	sys_sendfile,		"sendfile"	}, /* 1187 */
	{ 5,	TN,	printargs,		"getpmsg"	}, /* 1188 */
	{ 5,	TN,	printargs,		"putpmsg"	}, /* 1189 */
	{ 3,	TN,	sys_socket,		"socket"	}, /* 1190 */
	{ 3,	TN,	sys_bind,		"bind"		}, /* 1191 */
	{ 3,	TN,	sys_connect,		"connect"	}, /* 1192 */
	{ 2,	TN,	sys_listen,		"listen"	}, /* 1193 */
	{ 3,	TN,	sys_accept,		"accept"	}, /* 1194 */
	{ 3,	TN,	sys_getsockname,	"getsockname"	}, /* 1195 */
	{ 3,	TN,	sys_getpeername,	"getpeername"	}, /* 1196 */
	{ 4,	TN,	sys_socketpair,		"socketpair"	}, /* 1197 */
	{ 4,	TN,	sys_send,		"send"		}, /* 1198 */
	{ 6,	TN,	sys_sendto,		"sendto"	}, /* 1199 */
	{ 4,	TN,	sys_recv,		"recv"		}, /* 1200 */
	{ 6,	TN,	sys_recvfrom,		"recvfrom"	}, /* 1201 */
	{ 2,	TN,	sys_shutdown,		"shutdown"	}, /* 1202 */
	{ 5,	TN,	sys_setsockopt,		"setsockopt"	}, /* 1203 */
	{ 5,	TN,	sys_getsockopt,		"getsockopt"	}, /* 1204 */
	{ 3,	TN,	sys_sendmsg,		"sendmsg"	}, /* 1205 */
	{ 3,	TN,	sys_recvmsg,		"recvmsg"	}, /* 1206 */
	{ 2,	TF,	sys_pivotroot,		"pivot_root"	}, /* 1207 */
	{ 3,	TM,	sys_mincore,		"mincore"	}, /* 1208 */
	{ 3,	TM,	sys_madvise,		"madvise"	}, /* 1209 */
	{ 2,	TF,	sys_stat,		"stat"		}, /* 1210 */
	{ 2,	TF,	sys_lstat,		"lstat"		}, /* 1211 */
	{ 2,	TD,	sys_fstat,		"fstat"		}, /* 1212 */
	{ 6,	TP,	sys_clone,		"clone2"	}, /* 1213 */
	{ 3,	TD,	sys_getdents64,		"getdents64"	}, /* 1214 */
	{ 2,	0,	printargs,		"getunwind"	}, /* 1215 */
	{ 3,	TD,	sys_readahead,		"readahead"	}, /* 1216 */
	{ 5,	TF,	sys_setxattr,		"setxattr"	}, /* 1217 */
	{ 5,	TF,	sys_setxattr,		"lsetxattr"	}, /* 1218 */
	{ 5,	TD,	sys_fsetxattr,		"fsetxattr"	}, /* 1219 */
	{ 4,	TF,	sys_getxattr,		"getxattr"	}, /* 1220 */
	{ 4,	TF,	sys_getxattr,		"lgetxattr"	}, /* 1221 */
	{ 4,	TD,	sys_fgetxattr,		"fgetxattr"	}, /* 1222 */
	{ 3,	TF,	sys_listxattr,		"listxattr"	}, /* 1223 */
	{ 3,	TF,	sys_listxattr,		"llistxattr"	}, /* 1224 */
	{ 3,	TD,	sys_flistxattr,		"flistxattr"	}, /* 1225 */
	{ 2,	TF,	sys_removexattr,	"removexattr"	}, /* 1226 */
	{ 2,	TF,	sys_removexattr,	"lremovexattr"	}, /* 1227 */
	{ 2,	TD,	sys_fremovexattr,	"fremovexattr"	}, /* 1228 */
	{ 2,	TS,	sys_kill,		"tkill"		}, /* 1229 */
	{ 6,	0,	sys_futex,		"futex"		}, /* 1230 */
	{ 3,	0,	sys_sched_setaffinity,	"sched_setaffinity"},/* 1231 */
	{ 3,	0,	sys_sched_getaffinity,	"sched_getaffinity"},/* 1232 */
	{ 1,	0,	sys_set_tid_address,	"set_tid_address"}, /* 1233 */
	{ 4,	TD,	sys_fadvise64,		"fadvise64"	}, /* 1234 */
	{ 3,	TS,	sys_tgkill,		"tgkill"	}, /* 1235 */
	{ 1,	TP,	sys_exit,		"exit_group"	}, /* 1236 */
	{ 3,	0,	sys_lookup_dcookie,	"lookup_dcookie"}, /* 1237 */
	{ 2,	0,	sys_io_setup,		"io_setup"	}, /* 1238 */
	{ 1,	0,	sys_io_destroy,		"io_destroy"	}, /* 1239 */
	{ 5,	0,	sys_io_getevents,		"io_getevents"	}, /* 1240 */
	{ 3,	0,	sys_io_submit,		"io_submit"	}, /* 1241 */
	{ 3,	0,	sys_io_cancel,		"io_cancel"	}, /* 1242 */
	{ 1,	TD,	sys_epoll_create,	"epoll_create"	}, /* 1243 */
	{ 4,	TD,	sys_epoll_ctl,		"epoll_ctl"	}, /* 1244 */
	{ 4,	TD,	sys_epoll_wait,		"epoll_wait"	}, /* 1245 */
	{ 0,	0,	sys_restart_syscall,	"restart_syscall"}, /* 1246 */
	{ 5,	TI,	sys_semtimedop,		"semtimedop"	}, /* 1247 */
	{ 3,	0,	sys_timer_create,	"timer_create"	}, /* 1248 */
	{ 4,	0,	sys_timer_settime,	"timer_settime"	}, /* 1249 */
	{ 2,	0,	sys_timer_gettime,	"timer_gettime"	}, /* 1250 */
	{ 1,	0,	sys_timer_getoverrun,	"timer_getoverrun"}, /* 1251 */
	{ 1,	0,	sys_timer_delete,	"timer_delete"	}, /* 1252 */
	{ 2,	0,	sys_clock_settime,	"clock_settime"	}, /* 1253 */
	{ 2,	0,	sys_clock_gettime,	"clock_gettime"	}, /* 1254 */
	{ 2,	0,	sys_clock_getres,	"clock_getres"	}, /* 1255 */
	{ 4,	0,	sys_clock_nanosleep,	"clock_nanosleep"}, /* 1256 */
	{ MA,	0,	printargs,		"fstatfs64"	}, /* 1257 */
	{ MA,	0,	printargs,		"statfs64"	}, /* 1258 */
	{ 6,	TM,	sys_mbind,		"mbind"		}, /* 1259 */
	{ 5,	TM,	sys_get_mempolicy,	"get_mempolicy"	}, /* 1260 */
	{ 3,	TM,	sys_set_mempolicy,	"set_mempolicy"	}, /* 1261 */
	{ 4,	0,	sys_mq_open,		"mq_open"	}, /* 1262 */
	{ 1,	0,	sys_mq_unlink,		"mq_unlink"	}, /* 1263 */
	{ 5,	0,	sys_mq_timedsend,	"mq_timedsend"	}, /* 1264 */
	{ 5,	0,	sys_mq_timedreceive,	"mq_timedreceive" }, /* 1265 */
	{ 2,	0,	sys_mq_notify,		"mq_notify"	}, /* 1266 */
	{ 3,	0,	sys_mq_getsetattr,	"mq_getsetattr"	}, /* 1267 */
	{ 4,	0,	sys_kexec_load,		"kexec_load"	}, /* 1268 */
	{ 5,	0,	sys_vserver,		"vserver"	}, /* 1269 */
	{ 5,	TP,	sys_waitid,		"waitid"	}, /* 1270 */
	{ 5,	0,	sys_add_key,		"add_key"	}, /* 1271 */
	{ 4,	0,	sys_request_key,	"request_key"	}, /* 1272 */
	{ 5,	0,	sys_keyctl,		"keyctl"	}, /* 1273 */
	{ 3,	0,	sys_ioprio_set,		"ioprio_set"	}, /* 1274 */
	{ 2,	0,	sys_ioprio_get,		"ioprio_get"	}, /* 1275 */
	{ 6,	TM,	sys_move_pages,		"move_pages"	}, /* 1276 */
	{ 0,	TD,	sys_inotify_init,	"inotify_init"	}, /* 1277 */
	{ 3,	TD,	sys_inotify_add_watch,	"inotify_add_watch" }, /* 1278 */
	{ 2,	TD,	sys_inotify_rm_watch,	"inotify_rm_watch" }, /* 1279 */
	{ 4,	TM,	sys_migrate_pages,	"migrate_pages"	}, /* 1280 */
	{ 4,	TD|TF,	sys_openat,		"openat"	}, /* 1281 */
	{ 3,	TD|TF,	sys_mkdirat,		"mkdirat"	}, /* 1282 */
	{ 4,	TD|TF,	sys_mknodat,		"mknodat"	}, /* 1283 */
	{ 5,	TD|TF,	sys_fchownat,		"fchownat"	}, /* 1284 */
	{ 3,	TD|TF,	sys_futimesat,		"futimesat"	}, /* 1285 */
	{ 4,	TD|TF,	sys_newfstatat,		"newfstatat"	}, /* 1286 */
	{ 3,	TD|TF,	sys_unlinkat,		"unlinkat"	}, /* 1287 */
	{ 4,	TD|TF,	sys_renameat,		"renameat"	}, /* 1288 */
	{ 5,	TD|TF,	sys_linkat,		"linkat"	}, /* 1289 */
	{ 3,	TD|TF,	sys_symlinkat,		"symlinkat"	}, /* 1290 */
	{ 4,	TD|TF,	sys_readlinkat,		"readlinkat"	}, /* 1291 */
	{ 3,	TD|TF,	sys_fchmodat,		"fchmodat"	}, /* 1292 */
	{ 3,	TD|TF,	sys_faccessat,		"faccessat"	}, /* 1293 */
	{ 6,	TD,	sys_pselect6,		"pselect6"	}, /* 1294 */
	{ 5,	TD,	sys_ppoll,		"ppoll"		}, /* 1295 */
	{ 1,	TP,	sys_unshare,		"unshare"	}, /* 1296 */
	{ 2,	0,	sys_set_robust_list,	"set_robust_list" }, /* 1297 */
	{ 3,	0,	sys_get_robust_list,	"get_robust_list" }, /* 1298 */
	{ 6,	TD,	sys_splice,		"splice"	}, /* 1299 */
	{ 4,	TD,	sys_sync_file_range,	"sync_file_range" }, /* 1300 */
	{ 4,	TD,	sys_tee,		"tee"		}, /* 1301 */
	{ 4,	TD,	sys_vmsplice,		"vmsplice"	}, /* 1302 */
	{ MA,	0,	NULL,			NULL		}, /* 1303 */
	{ 3,	0,	sys_getcpu,		"getcpu"	}, /* 1304 */
	{ 6,	TD,	sys_epoll_pwait,	"epoll_pwait"	}, /* 1305 */
	{ MA,	0,	NULL,			NULL		}, /* 1306 */
	{ 3,	TD|TS,	sys_signalfd,		"signalfd"	}, /* 1307 */
	{ 4,	TD,	sys_timerfd,		"timerfd"	}, /* 1308 */
	{ 1,	TD,	sys_eventfd,		"eventfd"	}, /* 1309 */
	{ 4,	TD,	sys_preadv,		"preadv"	}, /* 1319 */
	{ 4,	TD,	sys_pwritev,		"pwritev"	}, /* 1320 */
	{ 4,	TP|TS,	sys_rt_tgsigqueueinfo,	"rt_tgsigqueueinfo"}, /* 1321 */
	{ 5,	TN,	sys_recvmmsg,		"recvmmsg"	}, /* 1322 */
	{ 2,	TD,	sys_fanotify_init,	"fanotify_init"	}, /* 1323 */
	{ 5,	TD|TF,	sys_fanotify_mark,	"fanotify_mark"	}, /* 1324 */
	{ 4,	0,	sys_prlimit64,		"prlimit64"	}, /* 1325 */
	{ 5,	TD|TF,	sys_name_to_handle_at,	"name_to_handle_at"}, /* 1326 */
	{ 3,	TD,	sys_open_by_handle_at,	"open_by_handle_at"}, /* 1327 */
	{ 2,	0,	sys_clock_adjtime,	"clock_adjtime"	}, /* 1328 */
	{ 1,	TD,	sys_syncfs,		"syncfs"	}, /* 1329 */
	{ 2,	TD,	sys_setns,		"setns"		}, /* 1330 */
	{ 4,	TN,	sys_sendmmsg,		"sendmmsg"	}, /* 1331 */
	{ 6,	0,	sys_process_vm_readv,	"process_vm_readv"	}, /* 1332 */
	{ 6,	0,	sys_process_vm_writev,	"process_vm_writev"	}, /* 1333 */
	{ 4,	TN,	sys_accept4,		"accept4"	}, /* 1334 */
	{ 3,	TD,	sys_finit_module,	"finit_module"	}, /* 1335 */
