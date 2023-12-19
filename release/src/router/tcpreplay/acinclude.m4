##
## additional m4 macros
##
## (C) 1999 Christoph Bartelmus (lirc@bartelmus.de)
##


dnl check for kernel source

AC_DEFUN([AC_PATH_KERNEL_SOURCE_SEARCH],
[
    kerneldir=missing
    kernelext=ko
    no_kernel=yes

    if test `uname` != "Linux"; then
        kerneldir="not running Linux"
    else
        shortvers="$( uname -r | sed -r 's/(@<:@2-9@:>@\.@<:@0-9@:>@+).*/\1/' )"
        if test x${ac_kerneldir} != x; then
            if test -d ${ac_kerneldir}; then
                kerneldir=`dirname ${ac_kerneldir}/Makefile`/
                no_kernel=no
            fi
        else
            for dir in /lib/modules/`uname -r`/build \
                    /lib/modules/`uname -r`/source \
                    /usr/src/kernel-source-`uname -r` \
                    /usr/src/linux-source-`uname -r` \
                    /usr/src/kernel-source-$shortvers \
                    /usr/src/linux-source-$shortvers \
                    /usr/src/linux
            do
                if test -d $dir; then
                    kerneldir=`dirname $dir/Makefile`/
                    no_kernel=no
                    break
                fi
            done
        fi
    fi

    if test x${no_kernel} != xyes; then
        if test -f ${kerneldir}/Makefile -a -f ${kerneldir}/.config; then
            version=$( sed -n '/^VERSION/s/.*=\ *//p' ${kerneldir}/Makefile )
            patchlevel=$( sed -n -e '/^PATCHLEVEL/s/.*=\ *//p' ${kerneldir}/Makefile )
            kerneluname=$( sed -n '/Linux kernel version/s/.*:\ *//p' ${kerneldir}/.config )
            if test "${version}" -eq 2; then
                if test "${patchlevel}" -lt 5; then
                    kernelext=o
                fi
            fi
        else
            kerneldir="not found"
            no_kernel=yes
        fi
    fi

    ac_cv_have_kernel="no_kernel=${no_kernel} \
            kerneldir=\"${kerneldir}\" \
            kernelext=\"${kernelext}\""

]
)

AC_DEFUN([AC_PATH_KERNEL_SOURCE],
[
    AC_CHECK_PROG(ac_pkss_mktemp,mktemp,yes,no)
    AC_PROVIDE([AC_PATH_KERNEL_SOURCE])
    AC_MSG_CHECKING(for Linux kernel sources)

    AC_ARG_WITH(kernel,
        [  --with-kernel=DIR       Use Linux kernel sources in DIR],

        ac_kerneldir=${withval}
        AC_PATH_KERNEL_SOURCE_SEARCH,

        ac_kerneldir=""
        AC_CACHE_VAL(ac_cv_have_kernel,AC_PATH_KERNEL_SOURCE_SEARCH)
    )

    eval "$ac_cv_have_kernel"

    AC_SUBST(kerneldir)
    AC_SUBST(kernelext)
    AC_MSG_RESULT(${kerneldir})
]
)
