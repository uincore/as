/**
 * AS - the open source Automotive Software on https://github.com/parai
 *
 * Copyright (C) 2017  AS <parai@foxmail.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */
/* ============================ [ INCLUDES  ] ====================================================== */
#include "Std_Types.h"
#include "lasdevlib.h"
#include <sys/queue.h>
#include <pthread.h>
#include <unistd.h>
#include "asdebug.h"
/* ============================ [ MACROS    ] ====================================================== */
#define AS_LOG_LAS_DEV 0
/* ============================ [ TYPES     ] ====================================================== */
struct LAS_Dev_s {
	char name[LAS_DEVICE_NAME_SIZE];
	int fd;
	void* param; /* should be filled by device control module while open */
	const LAS_DeviceOpsType* ops;
	uint32_t                size2;
	STAILQ_ENTRY(LAS_Dev_s) entry;
};

struct LAS_DevList_s {
	boolean initialized;
	pthread_mutex_t q_lock;
	STAILQ_HEAD(,LAS_Dev_s) head;
};
/* ============================ [ DECLARES  ] ====================================================== */
/* ============================ [ DATAS     ] ====================================================== */
static const LAS_DeviceOpsType* devOps [] =
{
	&rs232_dev_ops,
	NULL
};
static int _fd = 0; /* file identifier start from 0 */
static struct LAS_DevList_s devListH =
{
	.initialized = FALSE,
	.q_lock=PTHREAD_MUTEX_INITIALIZER
};
/* ============================ [ LOCALS    ] ====================================================== */
static struct LAS_Dev_s* getDev(const char* name)
{
	struct LAS_Dev_s *handle,*h;
	handle = NULL;

	(void)pthread_mutex_lock(&devListH.q_lock);
	STAILQ_FOREACH(h,&devListH.head,entry)
	{
		if(0u == strcmp(h->name,name))
		{
			handle = h;
			break;
		}
	}
	(void)pthread_mutex_unlock(&devListH.q_lock);

	return handle;
}

static struct LAS_Dev_s* getDev2(int fd)
{
	struct LAS_Dev_s *handle,*h;
	handle = NULL;

	(void)pthread_mutex_lock(&devListH.q_lock);
	STAILQ_FOREACH(h,&devListH.head,entry)
	{
		if(fd == h->fd)
		{
			handle = h;
			break;
		}
	}
	(void)pthread_mutex_unlock(&devListH.q_lock);

	return handle;
}
static const LAS_DeviceOpsType* search_ops(const char* name)
{
	const LAS_DeviceOpsType *ops,**o;
	o = devOps;
	ops = NULL;
	while(*o != NULL)
	{
		if(name == strstr(name,(*o)->name))
		{
			ops = *o;
			break;
		}
		o++;
	}

	return ops;
}
static void freeDev(struct LAS_DevList_s*h)
{
	struct LAS_Dev_s* d;

	pthread_mutex_lock(&h->q_lock);
	while(FALSE == STAILQ_EMPTY(&h->head))
	{
		d = STAILQ_FIRST(&h->head);
		STAILQ_REMOVE_HEAD(&h->head,entry);
		d->ops->close(d->param);
		free(d);
	}
	pthread_mutex_unlock(&h->q_lock);
}

/* ============================ [ FUNCTIONS ] ====================================================== */
void luai_asdevlib_open(void)
{
	if(!devListH.initialized)
	{
		freeDev(&devListH);
	}
	devListH.initialized = TRUE;
	STAILQ_INIT(&devListH.head);

}

int luai_as_open  (lua_State *L)
{
	int n = lua_gettop(L);  /* number of arguments */

	if(n > 1)
	{
		const char* device_name;
		const LAS_DeviceOpsType* ops;
		size_t ls;
		struct LAS_Dev_s* d;

		device_name = lua_tolstring(L, 1, &ls);
		if((0 == ls) || (ls > LAS_DEVICE_NAME_SIZE))
		{
			 return luaL_error(L,"incorrect argument device name to function '%s'",__func__);
		}

		d = getDev(device_name);
		if(NULL != d)
		{
			return luaL_error(L,"LAS device(%s) is already opened '%s'",device_name,__func__);
		}
		else
		{
			ops = search_ops(device_name);
			if(NULL != ops)
			{
				int rv;
				d = malloc(sizeof(struct LAS_Dev_s));
				strcpy(d->name,device_name);

				rv = ops->open(device_name,L,&d->param);

				if(rv)
				{
					d->fd = _fd++;
					d->ops = ops;
					pthread_mutex_lock(&devListH.q_lock);
					STAILQ_INSERT_TAIL(&devListH.head,d,entry);
					pthread_mutex_unlock(&devListH.q_lock);
					lua_pushinteger(L, d->fd);        /* result OK */
				}
				else
				{
					free(d);
					return luaL_error(L, "%s device <%s> failed!",__func__,device_name);
				}
			}
			else
			{
				return luaL_error(L, "%s device <%s> is not known by lua!",__func__,device_name);
			}
		}

		return 1;
	}
	else
	{
		return luaL_error(L, "%s (\"device name\",mode) API should has 2 arguments",__func__);
	}
}
int luai_as_read  (lua_State *L)
{
	int n = lua_gettop(L);  /* number of arguments */
	if(1==n)
	{
		int fd;
		int is_num;
		struct LAS_Dev_s* d;

		fd = lua_tounsignedx(L, 1,&is_num);
		if(!is_num)
		{
			 return luaL_error(L,"incorrect argument fd to function '%s'",__func__);
		}

		d = getDev2(fd);
		if(NULL == d)
		{
			 return luaL_error(L,"fd(%d) is not existed '%s'",fd,__func__);
		}
		if(d->ops->read != NULL)
		{
			return d->ops->read(d->param,L);
		}
		else
		{
			return luaL_error(L, "%s for %s is not supported",__func__,d->name);
		}
	}
	else
	{
		return luaL_error(L, "%s (fd) API should has 1 arguments",__func__);
	}
}

int luai_as_write  (lua_State *L)
{
	int n = lua_gettop(L);  /* number of arguments */
	if(3 == n)
	{
		int fd;
		int is_num;
		size_t ls,len;
		struct LAS_Dev_s* d;
		char* data;

		fd = lua_tounsignedx(L, 1, &is_num);
		if(!is_num)
		{
			 return luaL_error(L,"incorrect argument fd to function '%s'",__func__);
		}

		data = (char*)lua_tolstring(L, 2, &ls);
		if(0 == ls)
		{
			int i = 0;

			ls = luaL_len ( L , 2 ) ;
			data = malloc(ls);

			lua_pushvalue(L, 2);
			lua_pushnil(L);
			while (lua_next(L, -2))
			{
				lua_pushvalue(L, -2);
				data[i] = (char)lua_tounsignedx(L, -2,&is_num);
				ASLOG(LAS_DEV,"write: data[%d] = %d\n",i,data[i]);
				if(!is_num)
				{
					return luaL_error(L,"invalid data[%d] to function '%s'",i,__func__);
				}
				else
				{
					i ++;
				}

				lua_pop(L, 2);
			}
			lua_pop(L, 1);
		}
		else
		{
			ASLOG(LAS_DEV,"write string is '%s'\n",data);
		}

	    len = lua_tounsignedx(L, 3, &is_num);
		if(!is_num)
		{
			 return luaL_error(L,"incorrect argument size to function '%s'",__func__);
		}

		if(len > ls)
		{
			len = ls;
		}

		d = getDev2(fd);
		if(NULL == d)
		{
			 return luaL_error(L,"fd(%d) is not existed '%s'",fd,__func__);
		}

		if(d->ops->write != NULL)
		{
			lua_pushinteger(L, d->ops->write(d->param,data,len));
			return 1;
		}
		else
		{
			return luaL_error(L, "%s for %s is not supported",__func__,d->name);
		}
	}
	else
	{
		return luaL_error(L, "%s (fd,data,size) API should has 3 arguments",__func__);
	}
}

int luai_as_ioctl  (lua_State *L)
{
	int n = lua_gettop(L);  /* number of arguments */
	if(4==n)
	{
		int fd,type;
		int is_num;
		size_t ls,len;
		struct LAS_Dev_s* d;
		char* data;

		fd = lua_tounsignedx(L, 1, &is_num);
		if(!is_num)
		{
			 return luaL_error(L,"incorrect argument fd to function '%s'",__func__);
		}

		type = lua_tounsignedx(L, 2, &is_num);
		if(!is_num)
		{
			 return luaL_error(L,"incorrect argument type to function '%s'",__func__);
		}

		data = (char*)lua_tolstring(L, 3, &ls);
		if(0 == ls)
		{
			int i = 0;

			ls = luaL_len ( L , 3 ) ;
			data = malloc(ls);

			lua_pushvalue(L, 3);
			lua_pushnil(L);
			while (lua_next(L, -2))
			{
				lua_pushvalue(L, -2);
				data[i] = (char)lua_tounsignedx(L, -2,&is_num);
				ASLOG(LAS_DEV,"ioctl %d: data[%d] = %d\n",type,i,data[i]);
				if(!is_num)
				{
					return luaL_error(L,"invalid data[%d] to function '%s'",i,__func__);
				}
				else
				{
					i ++;
				}

				lua_pop(L, 2);
			}
			lua_pop(L, 1);
		}
		else
		{
			ASLOG(LAS_DEV,"ioctl string is '%s'\n",data);
		}

	    len = lua_tounsignedx(L, 3, &is_num);
		if(!is_num)
		{
			 return luaL_error(L,"incorrect argument size to function '%s'",__func__);
		}

		if(len > ls)
		{
			len = ls;
		}

		d = getDev2(fd);
		if(NULL == d)
		{
			 return luaL_error(L,"fd(%d) is not existed '%s'",fd,__func__);
		}

		if(d->ops->ioctl != NULL)
		{
			lua_pushinteger(L, d->ops->ioctl(d->param,type,data,len));
			return 1;
		}
		else
		{
			return luaL_error(L, "%s for %s is not supported",__func__,d->name);
		}
		return 1;
	}
	else
	{
		return luaL_error(L, "%s (fd) API should has 4 arguments",__func__);
	}
}

int luai_as_close  (lua_State *L)
{
	int n = lua_gettop(L);  /* number of arguments */
	if(1==n)
	{
		int fd;
		int is_num;
		struct LAS_Dev_s* d;

		fd = lua_tounsignedx(L, 1, &is_num);
		if(!is_num)
		{
			 return luaL_error(L,"incorrect argument fd to function '%s'",__func__);
		}
		d = getDev2(fd);
		if(NULL == d)
		{
			 return luaL_error(L,"fd(%d) is not existed '%s'",fd,__func__);
		}

		if(d->ops->close != NULL)
		{
			d->ops->close(d->param);
		}
		else
		{
			return luaL_error(L, "%s for %s is not supported",__func__,d->name);
		}
		return 0;
	}
	else
	{
		return luaL_error(L, "%s (fd) API should has 1 arguments",__func__);
	}
}
