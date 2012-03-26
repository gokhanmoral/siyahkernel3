/*
 * cgroup_timer_slack.c - control group timer slack subsystem
 *
 * Copyright Nokia Corparation, 2011
 * Author: Kirill A. Shutemov
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/cgroup.h>
#include <linux/init_task.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/rcupdate.h>

struct cgroup_subsys timer_slack_subsys;
struct timer_slack_cgroup {
	struct cgroup_subsys_state css;
	unsigned long min_slack_ns;
	unsigned long max_slack_ns;
};

enum {
	TIMER_SLACK_MIN,
	TIMER_SLACK_MAX,
};

extern int (*timer_slack_check)(struct task_struct *task,
		unsigned long slack_ns);

static struct timer_slack_cgroup *cgroup_to_tslack_cgroup(struct cgroup *cgroup)
{
	struct cgroup_subsys_state *css;

	css = cgroup_subsys_state(cgroup, timer_slack_subsys.subsys_id);
	return container_of(css, struct timer_slack_cgroup, css);
}

static int is_timer_slack_allowed(struct timer_slack_cgroup *tslack_cgroup,
		unsigned long slack_ns)
{
	if (slack_ns < tslack_cgroup->min_slack_ns ||
			slack_ns > tslack_cgroup->max_slack_ns)
		return false;
	return true;
}

static int cgroup_timer_slack_check(struct task_struct *task,
		unsigned long slack_ns)
{
	struct cgroup_subsys_state *css;
	struct timer_slack_cgroup *tslack_cgroup;

	/* XXX: lockdep false positive? */
	rcu_read_lock();
	css = task_subsys_state(task, timer_slack_subsys.subsys_id);
	tslack_cgroup = container_of(css, struct timer_slack_cgroup, css);
	rcu_read_unlock();

	if (!is_timer_slack_allowed(tslack_cgroup, slack_ns))
		return -EPERM;
	return 0;
}

static struct cgroup_subsys_state *
tslack_cgroup_create(struct cgroup_subsys *subsys, struct cgroup *cgroup)
{
	struct timer_slack_cgroup *tslack_cgroup;

	tslack_cgroup = kmalloc(sizeof(*tslack_cgroup), GFP_KERNEL);
	if (!tslack_cgroup)
		return ERR_PTR(-ENOMEM);

	if (cgroup->parent) {
		struct timer_slack_cgroup *parent;
		parent = cgroup_to_tslack_cgroup(cgroup->parent);
		tslack_cgroup->min_slack_ns = parent->min_slack_ns;
		tslack_cgroup->max_slack_ns = parent->max_slack_ns;
	} else {
		tslack_cgroup->min_slack_ns = 0UL;
		tslack_cgroup->max_slack_ns = ULONG_MAX;
	}

	return &tslack_cgroup->css;
}

static void tslack_cgroup_destroy(struct cgroup_subsys *subsys,
		struct cgroup *cgroup)
{
	kfree(cgroup_to_tslack_cgroup(cgroup));
}

/*
 * Adjust ->timer_slack_ns and ->default_max_slack_ns of the task to fit
 * limits of the cgroup.
 */
static void tslack_adjust_task(struct timer_slack_cgroup *tslack_cgroup,
		struct task_struct *tsk)
{
	if (tslack_cgroup->min_slack_ns > tsk->timer_slack_ns)
		tsk->timer_slack_ns = tslack_cgroup->min_slack_ns;
	else if (tslack_cgroup->max_slack_ns < tsk->timer_slack_ns)
		tsk->timer_slack_ns = tslack_cgroup->max_slack_ns;

	if (tslack_cgroup->min_slack_ns > tsk->default_timer_slack_ns)
		tsk->default_timer_slack_ns = tslack_cgroup->min_slack_ns;
	else if (tslack_cgroup->max_slack_ns < tsk->default_timer_slack_ns)
		tsk->default_timer_slack_ns = tslack_cgroup->max_slack_ns;
}

static void tslack_cgroup_attach(struct cgroup_subsys *subsys,
		struct cgroup *cgroup, struct cgroup *prev,
		struct task_struct *tsk)
{
	tslack_adjust_task(cgroup_to_tslack_cgroup(cgroup), tsk);
}

static int tslack_write_set_slack_ns(struct cgroup *cgroup, struct cftype *cft,
		u64 val)
{
	struct timer_slack_cgroup *tslack_cgroup;
	struct cgroup_iter it;
	struct task_struct *task;

	tslack_cgroup = cgroup_to_tslack_cgroup(cgroup);
	if (!is_timer_slack_allowed(cgroup_to_tslack_cgroup(cgroup), val))
		return -EPERM;

	/* Change timer slack value for all tasks in the cgroup */
	cgroup_iter_start(cgroup, &it);
	while ((task = cgroup_iter_next(cgroup, &it)))
		task->timer_slack_ns = val;
	cgroup_iter_end(cgroup, &it);

	return 0;
}

static u64 tslack_read_range(struct cgroup *cgroup, struct cftype *cft)
{
	struct timer_slack_cgroup *tslack_cgroup;

	tslack_cgroup = cgroup_to_tslack_cgroup(cgroup);
	switch (cft->private) {
	case TIMER_SLACK_MIN:
		return tslack_cgroup->min_slack_ns;
	case TIMER_SLACK_MAX:
		return tslack_cgroup->max_slack_ns;
	default:
		BUG();
	};
}

static int validate_change(struct cgroup *cgroup, u64 val, int type)
{
	struct timer_slack_cgroup *tslack_cgroup, *child;
	struct cgroup *cur;

	BUG_ON(type != TIMER_SLACK_MIN || type != TIMER_SLACK_MAX);

	if (cgroup->parent) {
		struct timer_slack_cgroup *parent;
		parent = cgroup_to_tslack_cgroup(cgroup->parent);
		if (!is_timer_slack_allowed(parent, val))
			return -EPERM;
	}

	tslack_cgroup = cgroup_to_tslack_cgroup(cgroup);
	if (type == TIMER_SLACK_MIN && val > tslack_cgroup->max_slack_ns)
		return -EINVAL;
	else if (val < tslack_cgroup->min_slack_ns)
		return -EINVAL;

	list_for_each_entry(cur, &cgroup->children, sibling) {
		child = cgroup_to_tslack_cgroup(cur);
		if (type == TIMER_SLACK_MIN && val > child->min_slack_ns)
			return -EBUSY;
		else if (val < child->max_slack_ns)
			return -EBUSY;
	}

	return 0;
}

static int tslack_write_range(struct cgroup *cgroup, struct cftype *cft,
		u64 val)
{
	struct timer_slack_cgroup *tslack_cgroup;
	struct cgroup_iter it;
	struct task_struct *task;
	int err;

	err = validate_change(cgroup, val, cft->private);
	if (err)
		return err;

	tslack_cgroup = cgroup_to_tslack_cgroup(cgroup);
	if (cft->private == TIMER_SLACK_MIN)
		tslack_cgroup->min_slack_ns = val;
	else
		tslack_cgroup->max_slack_ns = val;

	/*
	 * Adjust timer slack value for all tasks in the cgroup to fit
	 * min-max range.
	 */
	cgroup_iter_start(cgroup, &it);
	while ((task = cgroup_iter_next(cgroup, &it)))
		tslack_adjust_task(tslack_cgroup, task);
	cgroup_iter_end(cgroup, &it);

	return 0;
}

static struct cftype files[] = {
	{
		.name = "set_slack_ns",
		.write_u64 = tslack_write_set_slack_ns,
	},
	{
		.name = "min_slack_ns",
		.private = TIMER_SLACK_MIN,
		.read_u64 = tslack_read_range,
		.write_u64 = tslack_write_range,
	},
	{
		.name = "max_slack_ns",
		.private = TIMER_SLACK_MAX,
		.read_u64 = tslack_read_range,
		.write_u64 = tslack_write_range,
	},
};

static int tslack_cgroup_populate(struct cgroup_subsys *subsys,
		struct cgroup *cgroup)
{
	return cgroup_add_files(cgroup, subsys, files, ARRAY_SIZE(files));
}

struct cgroup_subsys timer_slack_subsys = {
	.name = "timer_slack",
	.module = THIS_MODULE,
#ifdef CONFIG_CGROUP_TIMER_SLACK
	.subsys_id = timer_slack_subsys_id,
#endif
	.create = tslack_cgroup_create,
	.destroy = tslack_cgroup_destroy,
	.attach = tslack_cgroup_attach,
	.populate = tslack_cgroup_populate,
};

static int __init init_cgroup_timer_slack(void)
{
	int err;

	if (timer_slack_check)
		return -EBUSY;
	timer_slack_check = cgroup_timer_slack_check;

	err = cgroup_load_subsys(&timer_slack_subsys);
	if (err)
		timer_slack_check = NULL;
	return err;
}

static void __exit exit_cgroup_timer_slack(void)
{
	BUG_ON(timer_slack_check != cgroup_timer_slack_check);
	timer_slack_check = NULL;
	cgroup_unload_subsys(&timer_slack_subsys);
}

module_init(init_cgroup_timer_slack);
module_exit(exit_cgroup_timer_slack);
MODULE_LICENSE("GPL");
