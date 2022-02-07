#ifndef CA_DEBUG_LEVEL
#define CA_DEBUG_LEVEL 2
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "ca.h"
#include "am_ca.h"
#include "am_cas.h"

#define MAX_DSC_DEV        32
#define DEV_NAME "/dev/dvb0.ca"

struct _dsc_dev{
	int used;
	int fd;
};
typedef struct _dsc_dev dsc_dev;
static dsc_dev rec_dsc_dev[MAX_DSC_DEV];

int ca_init(void)
{
    memset(&rec_dsc_dev[0], 0, sizeof(rec_dsc_dev));

    return 0;
}

int ca_open (int devno)
{
	int fd;
	char buf[32];

	if (devno >= MAX_DSC_DEV) {
		return -1;
	}
	if (rec_dsc_dev[devno].used)
		return 0;

	snprintf(buf, sizeof(buf), DEV_NAME"%d", devno);
	fd = open(buf, O_RDWR);
	if(fd==-1)
	{
		printf("cannot open \"%s\" (%d:%s)", DEV_NAME, errno, strerror(errno));
		CA_DEBUG(1, "ca_open() failed to open '%s%d' (%d:%s)",
			DEV_NAME, devno, errno, strerror(errno));
		return 0;
	}

	CA_DEBUG(1, "ca_open() '%s%d' success! fd=0x%x", DEV_NAME, devno, fd);
	rec_dsc_dev[devno].fd = fd;
	rec_dsc_dev[devno].used = 1;
	return 0;
}

int ca_alloc_chan (int devno, unsigned int pid, int algo, int dsc_type)
{
	int ret = 0;
	int fd = 0;
	struct ca_sc2_descr_ex desc;
	memset(&desc, 0, sizeof(desc));
	CA_DEBUG(1, "ca_alloc_chan() dev:%d, pid:0x%0x, algo:%d, dsc_type:%d loop:0\n",
		devno, pid, algo, dsc_type);
	desc.cmd = CA_ALLOC;
	desc.params.alloc_params.pid = pid;
	desc.params.alloc_params.algo = algo;
	desc.params.alloc_params.dsc_type = dsc_type;
	desc.params.alloc_params.ca_index = -1;
	desc.params.alloc_params.loop = 0;

	if (devno >= MAX_DSC_DEV || !rec_dsc_dev[devno].used) {
		return -1;
	}
	fd = rec_dsc_dev[devno].fd;
	ret = ioctl(fd, CA_SC2_SET_DESCR_EX, &desc);
	if (ret != 0) {
		CA_DEBUG(1, "ca_alloc_chan() ioctl fail, ret:0x%0x\n", ret);
		return -1;
	}
	CA_DEBUG(1, "ca_alloc_chan() success index:%d\n",desc.params.alloc_params.ca_index);
	return desc.params.alloc_params.ca_index;
}

int ca_free_chan (int devno, int index)
{
	int ret = 0;
	int fd = rec_dsc_dev[devno].fd;
	struct ca_sc2_descr_ex desc;

	CA_DEBUG(1, "ca_free_chan() devno: %d, index:%d\n", devno, index);
	desc.cmd = CA_FREE;
	desc.params.free_params.ca_index = index;

	if (devno >= MAX_DSC_DEV || !rec_dsc_dev[devno].used) {
		return -1;
	}
	fd = rec_dsc_dev[devno].fd;
	ret = ioctl(fd, CA_SC2_SET_DESCR_EX, &desc);
	if (ret != 0) {
		CA_DEBUG(1, "ca_free_chan() ioctl fail devno: %d, index:%d\n", devno, index);
		return -1;
	}
	//CA_DEBUG(1, "ca_free_chan() devno: %d, index:%d\n",devno, index);
	return 0;
}


int ca_set_key (int devno, int index, int parity, int key_index)
{
	int ret = 0;
	int fd = 0;
	struct ca_sc2_descr_ex desc;

	CA_DEBUG(1, "ca_set_key() dev:%d, index:%d, parity:%d, key_index:%d\n",
			devno, index, parity, key_index);
	desc.cmd = CA_KEY;
	desc.params.key_params.ca_index = index;
	desc.params.key_params.parity = parity;
	desc.params.key_params.key_index = key_index;

	if (devno >= MAX_DSC_DEV || !rec_dsc_dev[devno].used) {
		return -1;
	}
	fd = rec_dsc_dev[devno].fd;
	ret = ioctl(fd, CA_SC2_SET_DESCR_EX, &desc);
	if (ret != 0) {
		CA_DEBUG(1, "ca_set_key() ioctl fail, ret:0x%0x\n", ret);
		return -1;
	}
	//CA_DEBUG(1, "ca_set_key(), index:%d, parity:%d, key_index:%d\n",index, parity, key_index);
	return 0;
}


int ca_set_scb(int devno, int index, int scb, int scb_as_is)
{
	int ret = 0;
	int fd = 0;
	struct ca_sc2_descr_ex desc;

	CA_DEBUG(1,"ca_set_scb, index:%d, :%d, :%d\n", index, scb, scb_as_is);

	desc.cmd = CA_SET_SCB;
	desc.params.scb_params.ca_index = index;
	desc.params.scb_params.ca_scb = scb;
	desc.params.scb_params.ca_scb_as_is = scb_as_is;

	if (devno >= MAX_DSC_DEV || !rec_dsc_dev[devno].used) {
		CA_DEBUG(1, " ca_set_scb fail\n");
		return -1;
	}
	fd = rec_dsc_dev[devno].fd;
	ret = ioctl(fd, CA_SC2_SET_DESCR_EX, &desc);
	if (ret != 0) {
		CA_DEBUG(1, " ca_set_scb ioctl fail, ret:0x%0x\n", ret);
		return -1;
	}

	return 0;
}

int ca_close (int devno)
{
	int fd = 0;

	CA_DEBUG(1, "ca_close() dev:%d\n", devno);
	if (devno >= MAX_DSC_DEV || !rec_dsc_dev[devno].used) {
		return -1;
	}
	fd = rec_dsc_dev[devno].fd;
	close(fd);
	rec_dsc_dev[devno].fd = 0;
	rec_dsc_dev[devno].used = 0;
	return 0;
}

