// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Western Digital Corporation or its affiliates.
 *
 * Authors: Damien Le Moal (damien.lemoal@wdc.com)
 */

#include "cdl.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>
#include <dirent.h>

/*
 * Read a log page.
 */
int cdl_ata_read_log(struct cdl_dev *dev, uint8_t log,
		     uint16_t page, struct cdl_sg_cmd *cmd,
		     size_t bufsz)
{
	/*
	 * READ LOG DMA EXT in ATA 16 passthrough command.
	 * +=============================================================+
	 * |  Bit|  7  |  6  |   5   |   4   |   3   |   2   |  1  |  0  |
	 * |Byte |     |     |       |       |       |       |     |     |
	 * |=====+===================+===================================|
	 * | 0   |              Operation Code (85h)                     |
	 * |-----+-------------------------------------------------------|
	 * | 1   |  Multiple count   |            Protocol         | ext |
	 * |-----+-------------------------------------------------------|
	 * | 2   |  off_line |ck_cond| t_type| t_dir |byt_blk| t_length  |
	 * |-----+-------------------------------------------------------|
	 * | 3   |                 features (15:8)                       |
	 * |-----+-------------------------------------------------------|
	 * | 4   |                 features (7:0)                        |
	 * |-----+-------------------------------------------------------|
	 * | 5   |                 count (15:8)                          |
	 * |-----+-------------------------------------------------------|
	 * | 6   |                 count (7:0)                           |
	 * |-----+-------------------------------------------------------|
	 * | 7   |                 LBA (31:24 (15:8 if ext == 0)         |
	 * |-----+-------------------------------------------------------|
	 * | 8   |                 LBA (7:0)                             |
	 * |-----+-------------------------------------------------------|
	 * | 9   |                 LBA (39:32)                           |
	 * |-----+-------------------------------------------------------|
	 * | 10  |                 LBA (15:8)                            |
	 * |-----+-------------------------------------------------------|
	 * | 11  |                 LBA (47:40)                           |
	 * |-----+-------------------------------------------------------|
	 * | 12  |                 LBA (23:16)                           |
	 * |-----+-------------------------------------------------------|
	 * | 13  |                 Device                                |
	 * |-----+-------------------------------------------------------|
	 * | 14  |                 Command (0x47)                        |
	 * |-----+-------------------------------------------------------|
	 * | 15  |                 Control                               |
	 * +=============================================================+
	 */
	cdl_init_cmd(cmd, 16, SG_DXFER_FROM_DEV, bufsz);
	cmd->cdb[0] = 0x85; /* ATA 16 */
	/* DMA protocol, ext=1 */
	cmd->cdb[1] = (0x6 << 1) | 0x01;
	/* off_line=0, ck_cond=0, t_type=0, t_dir=1, byt_blk=1, t_length=10 */
	cmd->cdb[2] = 0x0e;
	cdl_sg_set_be16(&cmd->cdb[5], bufsz / 512);
	cmd->cdb[8] = log;
	cdl_sg_set_be16(&cmd->cdb[9], page);
	cmd->cdb[14] = 0x47; /* READ LOG DMA EXT */

	/* Execute the command */
	return cdl_exec_cmd(dev, cmd);
}

/*
 * Write a log page.
 */
int cdl_ata_write_log(struct cdl_dev *dev, uint8_t log,
		      uint16_t page, uint8_t *buf, size_t bufsz)
{
	struct cdl_sg_cmd cmd;

	/*
	 * WRITE LOG DMA EXT in ATA 16 passthrough command.
	 * +=============================================================+
	 * |  Bit|  7  |  6  |   5   |   4   |   3   |   2   |  1  |  0  |
	 * |Byte |     |     |       |       |       |       |     |     |
	 * |=====+===================+===================================|
	 * | 0   |              Operation Code (85h)                     |
	 * |-----+-------------------------------------------------------|
	 * | 1   |  Multiple count   |            Protocol         | ext |
	 * |-----+-------------------------------------------------------|
	 * | 2   |  off_line |ck_cond| t_type| t_dir |byt_blk| t_length  |
	 * |-----+-------------------------------------------------------|
	 * | 3   |                 features (15:8)                       |
	 * |-----+-------------------------------------------------------|
	 * | 4   |                 features (7:0)                        |
	 * |-----+-------------------------------------------------------|
	 * | 5   |                 count (15:8)                          |
	 * |-----+-------------------------------------------------------|
	 * | 6   |                 count (7:0)                           |
	 * |-----+-------------------------------------------------------|
	 * | 7   |                 LBA (31:24 (15:8 if ext == 0)         |
	 * |-----+-------------------------------------------------------|
	 * | 8   |                 LBA (7:0)                             |
	 * |-----+-------------------------------------------------------|
	 * | 9   |                 LBA (39:32)                           |
	 * |-----+-------------------------------------------------------|
	 * | 10  |                 LBA (15:8)                            |
	 * |-----+-------------------------------------------------------|
	 * | 11  |                 LBA (47:40)                           |
	 * |-----+-------------------------------------------------------|
	 * | 12  |                 LBA (23:16)                           |
	 * |-----+-------------------------------------------------------|
	 * | 13  |                 Device                                |
	 * |-----+-------------------------------------------------------|
	 * | 14  |                 Command (0x57)                        |
	 * |-----+-------------------------------------------------------|
	 * | 15  |                 Control                               |
	 * +=============================================================+
	 */
	cdl_init_cmd(&cmd, 16, SG_DXFER_TO_DEV, bufsz);
	memcpy(cmd.buf, buf, bufsz);

	cmd.cdb[0] = 0x85; /* ATA 16 */
	/* DMA protocol, ext=1 */
	cmd.cdb[1] = (0x6 << 1) | 0x01;
	/* off_line=0, ck_cond=0, t_type=0, t_dir=1, byt_blk=1, t_length=10 */
	cmd.cdb[2] = 0x0e;
	cdl_sg_set_be16(&cmd.cdb[5], bufsz / 512);
	cmd.cdb[8] = log;
	cdl_sg_set_be16(&cmd.cdb[9], page);
	cmd.cdb[14] = 0x57; /* WRITE LOG DMA EXT */

	/* Execute the command */
	return cdl_exec_cmd(dev, &cmd);
}

/*
 * Check if the ATA device supports CDL.
 */
static bool cdl_ata_cdl_supported(struct cdl_dev *dev)
{
	struct cdl_sg_cmd cmd;
	int ret;

	/* Supported capabilities log page */
	ret = cdl_ata_read_log(dev, 0x30, 0x03, &cmd, 512);
	if (ret) {
		cdl_dev_err(dev,
			    "Read supported capabilities log page failed\n");
		return false;
	}

	/* QWord content valid ? */
	if (!(cmd.buf[175] & (1 << 7)))
		return false;

	/* CDL Feature set supported ? */
	return cmd.buf[168] & (1 << 0);
}

/*
 * Get the CDL page type used for a command.
 */
int cdl_ata_get_cmd_cdlp(struct cdl_dev *dev, enum cdl_cmd c)
{
	if (!cdl_ata_cdl_supported(dev))
		return CDLP_NONE;

	switch (c) {
	case CDL_READ_16:
		return CDLP_T2A;
	case CDL_WRITE_16:
		return CDLP_T2B;
	case CDL_READ_32:
	case CDL_WRITE_32:
	default:
		return CDLP_NONE;
	}
}

static uint16_t cdl_ata_a2s_limit(uint8_t *buf)
{
	uint32_t limit = cdl_sg_get_le32(buf);

	/* SCSI mode page limits are 2-bytes only, so take care of overflows */
	if (limit / 10000 > 65535)
		return 65535;
	return limit / 10000;
}

/*
 * Read a CDL page from the device.
 */
int cdl_ata_read_page(struct cdl_dev *dev, enum cdl_p cdlp,
		      struct cdl_page *page)
{
	struct cdl_desc *desc = &page->descs[0];
	struct cdl_sg_cmd cmd;
	uint8_t *buf = cmd.buf;
	uint32_t policy;
	int i, ret;

	/* Command duration limits log */
	ret = cdl_ata_read_log(dev, 0x18, 0, &cmd, CDL_ATA_LOG_SIZE);
	if (ret) {
		cdl_dev_err(dev,
			    "Read command duration limits log page failed\n");
		return false;
	}

	/*
	 * Save the log page as we will need it when changing the log page
	 * descriptors.
	 */
	memcpy(dev->ata_cdl_log, cmd.buf, CDL_ATA_LOG_SIZE);

	/* T2A and T2B limits page */
	page->cdlp = cdlp;
	if (cdlp == CDLP_T2A) {
		/* Read descriptors */
		page->perf_vs_duration_guideline = buf[0] & 0x0f;
		buf += 64;
	} else {
		/* Write descriptors */
		buf += 288;
	}

	for (i = 0; i < CDL_MAX_DESC; i++, buf += 32, desc++) {
		policy = cdl_sg_get_le32(buf);
		desc->max_inactive_policy = (policy >> 8) & 0x0f;
		desc->max_active_policy = (policy >> 4) & 0x0f;
		desc->duration_policy = policy & 0x0f;
		desc->max_active_time = cdl_ata_a2s_limit(&buf[4]);
		desc->max_inactive_time = cdl_ata_a2s_limit(&buf[8]);
		desc->duration = cdl_ata_a2s_limit(&buf[16]);

		if (desc->max_inactive_time ||
		    desc->max_active_time ||
		    desc->duration)
			desc->cdltunit = 0x0a; /* 10ms */
		else
			desc->cdltunit = 0;
	}

	return 0;
}

/*
 * Convert SCSI limit to ATA microseconds.
 */
static uint32_t cdl_ata_s2a_limit(uint16_t limit, uint8_t t2cdlunit)
{
	uint64_t l = cdl_t2time(limit, t2cdlunit) / 1000;

	/* Take care of overflows */
	if (l > (~0U))
		return ~0U;
	return l;
}

/*
 * Force device revalidation so that sysfs exposes updated command
 * duration limits.
 */
static void cdl_ata_revalidate(struct cdl_dev *dev)
{
	const char *scan = "- - -";
	char path[PATH_MAX];
	char host[32], *h;
	struct dirent *dirent;
	FILE *f = NULL;
	DIR *d;

	sprintf(path, "/sys/block/%s/device/scsi_device", dev->name);
	d = opendir(path);
	if (!d) {
		cdl_dev_err(dev, "Open %s failed\n", path);
		return;
	}

	while ((dirent = readdir(d))) {
		if (dirent->d_name[0] != '.')
			break;
	}
	if (!dirent) {
		cdl_dev_err(dev, "Read %s failed\n", path);
		goto close;
	}

	strcpy(host, dirent->d_name);
	h = strchr(host, ':');
	if (!h) {
		cdl_dev_err(dev, "Parse %s entry failed\n", path);
		goto close;
	}
	*h = '\0';

	sprintf(path, "/sys/class/scsi_host/host%s/scan", host);

	f = fopen(path, "w");
	if (!f) {
		cdl_dev_err(dev, "Open %s failed\n", path);
		goto close;
	}

	if (!fwrite(scan, strlen(scan), 1, f))
		cdl_dev_err(dev, "Write %s failed\n", path);

close:
	if (d)
		closedir(d);
	if (f)
		fclose(f);
}

/*
 * Write a CDL page to the device.
 */
int cdl_ata_write_page(struct cdl_dev *dev, struct cdl_page *page)
{
	struct cdl_desc *desc = &page->descs[0];
	uint8_t *buf = dev->ata_cdl_log;
	uint8_t cdlp = page->cdlp;
	int i, ret;

	/* T2A and T2B limits page */
	if (cdlp == CDLP_T2A) {
		/* Read descriptors */
		buf[0] = page->perf_vs_duration_guideline & 0x0f;
		buf += 64;
	} else {
		/* Write descriptors */
		buf += 288;
	}

	for (i = 0; i < CDL_MAX_DESC; i++, buf += 32, desc++) {
		cdl_sg_set_le32(&buf[0],
				(desc->max_inactive_policy & 0x0f) << 8 |
				(desc->max_active_policy & 0x0f) << 4 |
				(desc->duration_policy & 0x0f));
		cdl_sg_set_le32(&buf[4],
				cdl_ata_s2a_limit(desc->max_active_time,
						  desc->cdltunit));
		cdl_sg_set_le32(&buf[8],
				cdl_ata_s2a_limit(desc->max_inactive_time,
						  desc->cdltunit));
		cdl_sg_set_le32(&buf[16],
				cdl_ata_s2a_limit(desc->duration,
						  desc->cdltunit));
	}

	ret = cdl_ata_write_log(dev, 0x18, 0,
				dev->ata_cdl_log, CDL_ATA_LOG_SIZE);
	if (ret) {
		cdl_dev_err(dev,
			    "Write command duration limits log page failed\n");
		return ret;
	}

	cdl_ata_revalidate(dev);

	return 0;
}

