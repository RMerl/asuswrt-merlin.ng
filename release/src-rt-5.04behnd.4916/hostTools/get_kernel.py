import sys
import os
import kernel_repositories

#first argument is repo shortcut
#second argument is commit or the branch


repo_info=sys.argv[1].split("_")
commit_branch=sys.argv[2]
site=""
cmd_to_run=""
ret=-1
if kernel_repositories.kernel_repos.__contains__(repo_info[0]):
	site=kernel_repositories.kernel_repos[repo_info[0]]
else:
	print("invalid kernel repository site (check KERNEL_AUTOFETCH??) " + sys.argv[1])
	sys.ext(-2)

user=os.environ['USER']

if len(site):
	#print(site)
	if site.__contains__(repo_info[1]) :
		repo_path=site[repo_info[1]]
	
	
	if repo_info[1]  == "git" :
		cmd_to_run="git init . && git remote add origin ssh://" + user + "@"+ repo_path + " && git pull --depth=1 origin " + commit_branch + " < /dev/null && touch .git_clone_complete"
	if repo_info[1]  == "rsync" :
		cmd_to_run = "rsync -r " + repo_path + "/* ."


if len(cmd_to_run):
	print("Syncing kernel, using - " + cmd_to_run);
	ret=os.system(cmd_to_run)


if ret > 0:
	ret=-1

sys.exit(ret)

