

ccache='ccache'
tc_top='/opt/toolchains'
tc_cached='/opt/toolchains/cached'
dshell='/bin/dash'

if [ ! -x $dshell ] 
then
  dshell='/bin/bash'
fi

echo "shell is $dshell"
echo "ccache command is $ccache"
echo "toolchain original location is $tc_top"
echo "cached wrappers will be installed at $tc_cached"
echo
echo "Be sure that $tc_cached is empty"
echo "To use this, export TOOLCHAIN_BASE and set it to $tc_cached"
echo
echo
for arg in $*
do
  tcd=$tc_top/`basename $arg`/usr
  echo $tcd

  if [ ! -d $tcd ]
  then
     echo "$tcd does not exist"
     exit 1
  fi
done
echo
echo "type 'yes' to continue"
read a
if [ "$a" != "yes" ]
then
   echo "aborted"
   exit 1
fi

for arg in $*
do
  tcd=$tc_top/`basename $arg`/usr
  echo $tcd

  if [ ! -d $tcd ]
  then
     echo "$tcd does not exist"
     exit 1
  fi
 # done
 # exit 0;

# for tcd in $tc_top/*/usr
# do
   echo $tcd
   tc=`dirname $tcd`
   tc=`basename $tc` 
   echo $tc
   mkdir -p $tc_cached/$tc/usr/bin || exit 2
   pushd $tc_cached/$tc/usr || exit 2
      for lnk in $tcd/*
      do
         if [ "$tcd/bin" != "$lnk" ]
         then
           ln -s $lnk . || exit 2
         fi
      done
      cd bin
      for gcc in $tcd/bin/*
      do
        if [ -x $gcc ]
        then
           if ls -l $gcc | grep -q -- '-> toolchain-wrapper'
           then
echo "wrap $gcc"
             f=`basename $gcc`
             rm -f $f
             echo "#!$dshell" > $f
             echo >> $f
             echo "exec $ccache $gcc \"\$@\"" >> $f
             chmod +x $f
           else
echo "link $gcc"
             ln -sf $gcc  || exit 2
           fi
        fi
      done
   popd
done



