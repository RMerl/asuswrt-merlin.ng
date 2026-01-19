/* dnsmasq is Copyright (c) 2000-2025 Simon Kelley
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991, or
   (at your option) version 3 dated 29 June, 2007.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
     
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dnsmasq.h"
#ifdef HAVE_INOTIFY

#include <sys/inotify.h>
#include <sys/param.h> /* For MAXSYMLINKS */

/* the strategy is to set an inotify on the directories containing
   resolv files, for any files in the directory which are close-write 
   or moved into the directory.
   
   When either of those happen, we look to see if the file involved
   is actually a resolv-file, and if so, call poll-resolv with
   the "force" argument, to ensure it's read.

   This adds one new error condition: the directories containing
   all specified resolv-files must exist at start-up, even if the actual
   files don't. 
*/

static char *inotify_buffer;
#define INOTIFY_SZ (sizeof(struct inotify_event) + NAME_MAX + 1)

/* If path is a symbolic link, return the path it
   points to, made absolute if relative.
   If path doesn't exist or is not a symlink, return NULL.
   Return value is malloc'ed */
static char *my_readlink(char *path)
{
  ssize_t rc, size = 64;
  char *buf;

  while (1)
    {
      buf = safe_malloc(size);
      rc = readlink(path, buf, (size_t)size);
      
      if (rc == -1)
	{
	  /* Not link or doesn't exist. */
	  if (errno == EINVAL || errno == ENOENT)
	    {
	      free(buf);
	      return NULL;
	    }
	  else
	    die(_("cannot access path %s: %s"), path, EC_MISC);
	}
      else if (rc < size-1)
	{
	  char *d;
	  
	  buf[rc] = 0;
	  if (buf[0] != '/' && ((d = strrchr(path, '/'))))
	    {
	      /* Add path to relative link */
	      char *new_buf = safe_malloc((d - path) + strlen(buf) + 2);
	      *(d+1) = 0;
	      strcpy(new_buf, path);
	      strcat(new_buf, buf);
	      free(buf);
	      buf = new_buf;
	    }
	  return buf;
	}

      /* Buffer too small, increase and retry */
      size += 64;
      free(buf);
    }
}

void inotify_dnsmasq_init()
{
  struct resolvc *res;
  inotify_buffer = safe_malloc(INOTIFY_SZ);
  daemon->inotifyfd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
  
  if (daemon->inotifyfd == -1)
    die(_("failed to create inotify: %s"), NULL, EC_MISC);

  if (daemon->port == 0 || option_bool(OPT_NO_RESOLV))
    return;
  
  for (res = daemon->resolv_files; res; res = res->next)
    {
      char *d, *new_path, *path = safe_malloc(strlen(res->name) + 1);
      int links = MAXSYMLINKS;

      strcpy(path, res->name);

      /* Follow symlinks until we reach a non-symlink, or a non-existent file. */
      while ((new_path = my_readlink(path)))
	{
	  if (links-- == 0)
	    die(_("too many symlinks following %s"), res->name, EC_MISC);
	  free(path);
	  path = new_path;
	}

      res->wd = -1;

      if ((d = strrchr(path, '/')))
	{
	  *d = 0; /* make path just directory */
	  res->wd = inotify_add_watch(daemon->inotifyfd, path, IN_CLOSE_WRITE | IN_MOVED_TO);

	  res->file = d+1; /* pointer to filename */
	  *d = '/';
	  
	  if (res->wd == -1 && errno == ENOENT)
	    die(_("directory %s for resolv-file is missing, cannot poll"), res->name, EC_MISC);
	}	  
	 
      if (res->wd == -1)
	die(_("failed to create inotify for %s: %s"), res->name, EC_MISC);
	
    }
}

static struct hostsfile *dyndir_addhosts(struct dyndir *dd, char *file)
{
  /* Check if this file is already known in dd->files */
  struct hostsfile *ah;
  size_t dirlen = strlen(dd->dname);
  
  /* ah->fname always starts with the string in dd->dname */
  for (ah = dd->files; ah; ah = ah->next)
    if (ah->fname[dirlen] == '/' &&
        strcmp(&ah->fname[dirlen+1], file) == 0)
      return ah;
        
  /* Not known, create new hostsfile record for this dyndir */
  if ((ah = whine_malloc(sizeof(struct hostsfile))))
    {
      char *path;

      if (!(path = whine_malloc(dirlen + strlen(file) + 2)))
	{
	  free(ah);
	  return NULL;
	}
      
      strcpy(path, dd->dname);
      strcat(path, "/");
      strcat(path, file);
      
      /* Add this file to the tip of the linked list */
      ah->next = dd->files;
      dd->files = ah;
      
      /* Copy flags, set index and the full file path */
      ah->flags = dd->flags;
      ah->index = daemon->host_index++;
      ah->fname = path;
    }
  
  return ah;
}


/* initialisation for dynamic-dir. Set inotify watch for each directory, and read pre-existing files */
void set_dynamic_inotify(int flag, int total_size, struct crec **rhash, int revhashsz)
{
  struct dyndir *dd;

  for (dd = daemon->dynamic_dirs; dd; dd = dd->next)
    {
      DIR *dir_stream = NULL;
      struct dirent *ent;
      struct stat buf;

      if (!(dd->flags & flag))
	continue;

      if (stat(dd->dname, &buf) == -1)
	{
	  my_syslog(LOG_ERR, _("bad dynamic directory %s: %s"), 
		    dd->dname, strerror(errno));
	  continue;
	}

      if (!(S_ISDIR(buf.st_mode)))
	{
	  my_syslog(LOG_ERR, _("bad dynamic directory %s: %s"), 
		    dd->dname, _("not a directory"));
	  continue;
	}

       if (!(dd->flags & AH_WD_DONE))
	 {
	   dd->wd = inotify_add_watch(daemon->inotifyfd, dd->dname, IN_CLOSE_WRITE | IN_MOVED_TO | IN_DELETE);
	   dd->flags |= AH_WD_DONE;
	 }

       /* Read contents of dir _after_ calling add_watch, in the hope of avoiding
	  a race which misses files being added as we start */
       if (dd->wd == -1 || !(dir_stream = opendir(dd->dname)))
	 {
	   my_syslog(LOG_ERR, _("failed to create inotify for %s: %s"),
		     dd->dname, strerror(errno));
	   continue;
	 }

       while ((ent = readdir(dir_stream)))
	 {
	   size_t lenfile = strlen(ent->d_name);
	   	   
	   /* ignore emacs backups and dotfiles */
	   if (lenfile == 0 || 
	       ent->d_name[lenfile - 1] == '~' ||
	       (ent->d_name[0] == '#' && ent->d_name[lenfile - 1] == '#') ||
	       ent->d_name[0] == '.')
	     continue;

	   if (dd->flags & AH_HOSTS)
	     {
	       struct hostsfile *ah;

	       /* ignore non-regular files */
	       if ((ah = dyndir_addhosts(dd, ent->d_name)) &&
		   stat(ah->fname, &buf) != -1 && S_ISREG(buf.st_mode))
		 total_size = read_hostsfile(ah->fname, ah->index, total_size, rhash, revhashsz);
	     }
#ifdef HAVE_DHCP
	   else if (dd->flags & (AH_DHCP_HST | AH_DHCP_OPT))
	     {
	       char *path;
	       
	       if ((path = whine_malloc(strlen(dd->dname) + lenfile + 2)))
		 {
		   strcpy(path, dd->dname);
		   strcat(path, "/");
		   strcat(path, ent->d_name);
		   
		   /* ignore non-regular files */
		   if (stat(path, &buf) != -1 && S_ISREG(buf.st_mode))
		     option_read_dynfile(path, dd->flags);
		   
		   free(path);
		 }
	     }
#endif		   
	 }
       
       closedir(dir_stream);
    }
}

int inotify_check(time_t now)
{
  int hit = 0;
  struct dyndir *dd;

  (void)now;
  
  while (1)
    {
      int rc;
      char *p;
      struct resolvc *res;
      struct inotify_event *in;

      while ((rc = read(daemon->inotifyfd, inotify_buffer, INOTIFY_SZ)) == -1 && errno == EINTR);
      
      if (rc <= 0)
	break;
      
      for (p = inotify_buffer; rc - (p - inotify_buffer) >= (int)sizeof(struct inotify_event); p += sizeof(struct inotify_event) + in->len) 
	{
	  size_t namelen;

	  in = (struct inotify_event*)p;
	  
	  /* ignore emacs backups and dotfiles */
	  if (in->len == 0 || (namelen = strlen(in->name)) == 0 ||
	      in->name[namelen - 1] == '~' ||
	      (in->name[0] == '#' && in->name[namelen - 1] == '#') ||
	      in->name[0] == '.')
	    continue;

	  for (res = daemon->resolv_files; res; res = res->next)
	    if (res->wd == in->wd && strcmp(res->file, in->name) == 0)
	      hit = 1;

	  for (dd = daemon->dynamic_dirs; dd; dd = dd->next)
	    if (dd->wd == in->wd)
	      {
		if (dd->flags & AH_HOSTS)
		  {
		    struct hostsfile *ah;
		    if ((ah = dyndir_addhosts(dd, in->name)))
		      {
			const unsigned int removed = cache_remove_uid(ah->index);

			/* Is this is a deletion event? */
			if (in->mask & IN_DELETE)
			  my_syslog(LOG_INFO, _("inotify: %s removed"), ah->fname);
			else 
			  my_syslog(LOG_INFO, _("inotify: %s new or modified"), ah->fname);

			if (removed > 0)
			  my_syslog(LOG_INFO, _("inotify: flushed %u names read from %s"), removed, ah->fname);
			
			/* (Re-)load hostsfile only if this event isn't triggered by deletion */
			if (!(in->mask & IN_DELETE))
			  read_hostsfile(ah->fname, ah->index, 0, NULL, 0);
#ifdef HAVE_DHCP
			if (daemon->dhcp || daemon->doing_dhcp6) 
			  {
			    /* Propagate the consequences of loading a new dhcp-host */
			    dhcp_update_configs(daemon->dhcp_conf);
			    lease_update_from_configs(); 
			    lease_update_file(now); 
			    lease_update_dns(1);
			  }
#endif
		      }
		  }
#ifdef HAVE_DHCP
		else if (!(in->mask & IN_DELETE))
		  {
		    char *path;

		    if ((path = whine_malloc(strlen(dd->dname) + in->len + 2)))
		      {
			strcpy(path, dd->dname);
			strcat(path, "/");
			strcat(path, in->name);
			
			my_syslog(LOG_INFO, _("inotify: %s new or modified"), path);

			if ((dd->flags & AH_DHCP_HST) && option_read_dynfile(path, AH_DHCP_HST))
			  {
			    /* Propagate the consequences of loading a new dhcp-host */
			    dhcp_update_configs(daemon->dhcp_conf);
			    lease_update_from_configs(); 
			    lease_update_file(now); 
			    lease_update_dns(1);
			  }
			
			if (dd->flags & AH_DHCP_OPT)
			  option_read_dynfile(path, AH_DHCP_OPT);
		    
			free(path);
		      }
		  }
#endif
		
	      }
	}
    }
  
  return hit;
}

#endif  /* INOTIFY */
