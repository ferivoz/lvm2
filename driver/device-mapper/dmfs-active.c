/*
 * dmfs-active.c
 *
 * Copyright (C) 2001 Sistina Software
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU CC; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <linux/config.h>
#include <linux/fs.h>
#include <linux/seq_file.h>

#include "dm.h"
#include "dmfs.h"


static void *s_start(struct seq_file *s, loff_t *pos)
{
	struct dmfs_i *dmi = s->context;
	if (*pos > 0)
		return NULL;
	down(&dmi->sem);
	return (void *)1;
}

static void *s_next(struct seq_file *s, void *v, loff_t *pos)
{
	(*pos)++;
	return NULL;
}

static void s_stop(struct seq_file *s, void *v)
{
	struct dmfs_i *dmi = s->context;
	up(&dmi->sem);
}

static int s_show(struct seq_file *s, void *v)
{
	struct dmfs_i *dmi = s->context;
	char msg[3] = "0\n";
	if (is_active(dmi->md)) {
		msg[1] = '1';
	}
	seq_puts(s, msg);
	return 0;
}

struct seq_operations dmfs_active_seq_ops = {
	start:	s_start,
	next:	s_next,
	stop:	s_stop,
	show:	s_show,
};

ssize_t dmfs_active_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	struct inode *dir = file->f_dentry->d_parent->d_inode;
	struct dmfs_i *dmi = DMFS_I(dir);
	int written = 0;
	
	if (count == 0)
		goto out;
	if (count != 1 && count != 2)
		return -EINVAL;
	if (buf[0] != '0' && buf[0] != '1')
		return -EINVAL;

	down(&dmi->sem);
	written = count;
	if (is_active(dmi->md)) {
		if (buf[0] == '0')
			dm_deactivate(dmi->md);
	} else {
		if (buf[0] == '1') {
			if (dmi->md->map) {
				dm_activate(dmi->md, dmi->md->map);
			} else {
				written = -EPERM;
			}
		}
	}
	up(&dmi->sem);

out:
	return written;
}


