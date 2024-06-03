
kernel_repos = {

#format :::
#	"site:branch" - "transport" : "path"
# set KERNEL_AUTOFETCH one of the following sites one of below such as irv:5.15_git, irv:5.15_rsync or ihl:5.15_git etc


"irv:5.15" :

	{
		"rsync" : "bcacpe-irv-1:/auto/hudson_workspace_new/CommEngine_Dev_KUDU_CleanPlusCFE/out/kernel/linux-5.15",
	  	"git" : "stbgit.stb.broadcom.net:29418/bvg/linux.git"
	},
"irv:4.19" :

	{
		"rsync" : "bcacpe-irv-1:/auto/hudson_workspace_new/CommEngine_Dev_KUDU_CleanPlusCFE/out/kernel/linux-4.19",
	  	"git" : "gitbcacpe@git-irv-03.broadcom.com:linux-stable.git"
	},


"ihl:5.15" :

	{
		"rsync" : "bcacpe-irv-1:/auto/hudson_workspace_new/CommEngine_Dev_KUDU_CleanPlusCFE/out",
		"git" : "bld-ihl-bvg-06.ihl.broadcom.net:/lwork/CPE/common/kernel/linux-5.15"
	}


}
