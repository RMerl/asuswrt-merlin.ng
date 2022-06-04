#!/bin/bash

# input parameters :
#   $1 path to source directory
#   $2 path to destination directory
#   $3 optional list of files 


# this script will:
# if provided two first parameters:  
#    create in dest directory same drectory tree as in src directory
#    for each *./h/c/s file in src directory tree
# if provided 3 parameters 
#    create for each file from the list link in destination directory to file from src directory
# in both cases
#  if link to src file not exists new link will be created
#  if link exists will compare file time stamps and update link time 
#            stamp in case that file was modified after link creation time
 
# uncomment for debug
#set -x

#list of src directories 
dir_list=

#src and dest path
src_path=$1
dest_path=$2
DIR_ARGS=2
args=("$@")

[ -z "$LN_FORCE" ] && LN_FORCE=true
#will fill dir_list 
get_dir_list()
{
	cd $src_path
	dir_list=`du|awk '{print $2}' | sed -e 's/.\///' |grep -v "\."` 
}

#check and create  link for file  ($1 - file name) 
check_create_link()
{
	file=$1

	if [ ! -e $dest_path/$file ]
    	then
		mkdir -p $dest_path
        echo "  LN $file->$dest_path/$file"
		[ "$LN_FORCE" == true ] && LN_PARAM="-snf" || LN_PARAM="-s"
		ln $LN_PARAM $src_path/$file $dest_path/$file
	else
		src_file_time=`stat -c %Y $src_path/$file`
		link_file_time=`stat -c %Y $dest_path/$file`
	    [ $src_file_time -gt $link_file_time ] && echo "LN UPDATE $file->$dest_path/$file" ; unlink  $dest_path/$file; ln -s $src_path/$file $dest_path/$file 
	fi
}

#create links for directory tree
make_dir_links()
{
	#get src dir list
	get_dir_list 

	#create dest dir tree
	cd $dest_path
	for i in $dir_list
    	do
		mkdir -p $i
	done
		
	#create/update links
	cd $src_path	
	for i in `find . -name "*.[chsS]" -print` 
	do
		check_create_link $i
	done
}

#create links for file list
make_file_links()
{
	#for all files in file list 
	for ((i=2; i < $#; i++)) {
		#create link for file from file list (bash array) 
	    check_create_link ${args[$i]}
	}
}

main()
{
#if provided 3 parameters create links for file list
if [ $# -gt $DIR_ARGS ]
	then
	make_file_links $@
#create links for directory tree
else 
	make_dir_links $@
fi

}

main $@


