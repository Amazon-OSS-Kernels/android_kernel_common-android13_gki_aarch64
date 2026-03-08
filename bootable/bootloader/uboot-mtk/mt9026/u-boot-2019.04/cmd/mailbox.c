// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Mediatek 2021 - All Rights Reserved
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <mailbox.h>

static void show_mbox_controller(struct udevice *con)
{
	printf("MBOX:\t%s", con->name);
	if (device_active(con))
		printf("  (active %d)", con->seq);
	printf("\n");
}

static int do_mailbox_list(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[])
{
	struct udevice *con;
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_MAILBOX, &uc);
	if (ret)
		return CMD_RET_FAILURE;
	uclass_foreach_dev(con, uc)
		show_mbox_controller(con);

	return 0;
}

static int do_mailbox_send(cmd_tbl_t *cmdtp, int flag, int argc,
			   char *const argv[])
{
	struct ofnode_phandle_args args;
	struct mbox_chan mbox;
	int i, id, ret = 0;
	const char *str_mbox = NULL;
	u8 data[62] = {0};

	if (argc < 4)
		return CMD_RET_USAGE;

	/* parsing args */
	str_mbox = argv[1];
	id = (int)simple_strtoul(argv[2], NULL, 10);

	printf("[start] %s: %s, id=%d\n", __func__, str_mbox, id);

	for (i = 0; i < (argc - 3); i++) {
		data[i] = (u8)simple_strtoul(argv[3 + i], NULL, 16);
		printf("data[%d]=0x%x\n", i, data[i]);
	}

	/* fill of args */
	args.args[0] = id;
	args.args_count = 1;

	/* get mbox channel */
	ret = mbox_get_by_dev_name(str_mbox, &args, &mbox);
	if (ret) {
		printf("%s: mbox_get_by_dev_name %s failed: %d\n",
		       __func__, str_mbox, ret);
		return CMD_RET_FAILURE;
	}

	/* send message */
	ret = mbox_send(&mbox, data);
	if (ret) {
		printf("%s: mbox_send to %s failed: %d\n",
		       __func__, str_mbox, ret);
		return CMD_RET_FAILURE;
	}

	printf("[end] %s: %s, id=%d\n", __func__, str_mbox, id);

	return 0;
}

static int do_mailbox_recv(cmd_tbl_t *cmdtp, int flag, int argc,
			   char *const argv[])
{
	struct ofnode_phandle_args args;
	struct mbox_chan mbox;
	ulong timeout = 0;
	int i, id, ret = 0;
	const char *str_mbox = NULL;
	u8 data[62] = {0};

	if (argc < 3)
		return CMD_RET_USAGE;

	/* parsing args */
	str_mbox = argv[1];
	id = (int)simple_strtoul(argv[2], NULL, 10);
	if (argc > 3)
		timeout = simple_strtoul(argv[3], NULL, 10);

	printf("[start] %s: %s, id=%d, timeout=%lu\n",
	       __func__, str_mbox, id, timeout);

	/* fill of args */
	args.args[0] = id;
	args.args_count = 1;

	/* get mbox channel */
	ret = mbox_get_by_dev_name(str_mbox, &args, &mbox);
	if (ret) {
		printf("%s: mbox_get_by_dev_name %s failed: %d\n",
		       __func__, str_mbox, ret);
		return CMD_RET_FAILURE;
	}

	/* recv message */
	ret = mbox_recv(&mbox, data, timeout);
	if (ret) {
		printf("%s: mbox_recv from %s failed: %d\n",
		       __func__, str_mbox, ret);
		return CMD_RET_FAILURE;
	}

	printf("%s: mbox_recv from %s :\n", __func__, str_mbox);
	for (i = 0; i < sizeof(data); i++) {
		printf("data[%d]=0x%x\n", i, data[i]);
	}

	printf("[end] %s: %s, id=%d, timeout=%lu\n",
	       __func__, str_mbox, id, timeout);

	return 0;
}

static cmd_tbl_t cmd_mailbox_sub[] = {
	U_BOOT_CMD_MKENT(list, 0, 0, do_mailbox_list,
			 "list mailbox contollers", ""),
	U_BOOT_CMD_MKENT(send, 66, 0, do_mailbox_send,
			 "Send mailbox message",
			 "<mbox_controller> <id> <data>\n"
			 "- mbox_controller: Mailbox controller name\n"
			 "- id: ID of the mailbox client\n"
			 "- data: Message total 62 bytes(hex) (ex. AA BB..)\n"),
	U_BOOT_CMD_MKENT(recv, 4, 0, do_mailbox_recv,
			 "Receive mailbox message",
			 "<mbox_controller> <id> [timeout]\n"
			 "- mbox_controller: Mailbox controller name\n"
			 "- id: ID of the mailbox client\n"
			 "- timeout: optional timeout (us)\n"),
};

/**
 * do_remoteproc() - (replace: short desc)
 * @cmdtp:	unused
 * @flag:	unused
 * @argc:	argument count
 * @argv:	argument list
 *
 * parses up the command table to invoke the correct command.
 *
 * Return: 0 if no error, else returns appropriate error value.
 */
static int do_mailbox(cmd_tbl_t *cmdtp, int flag, int argc,
			 char *const argv[])
{
	cmd_tbl_t *c = NULL;

	/* Strip off leading 'rproc' command argument */
	argc--;
	argv++;

	if (argc)
		c = find_cmd_tbl(argv[0], cmd_mailbox_sub,
				 ARRAY_SIZE(cmd_mailbox_sub));
	if (c)
		return c->cmd(cmdtp, flag, argc, argv);

	return CMD_RET_USAGE;
}

U_BOOT_CMD(mbox, 66, 0, do_mailbox,
	   "Mailbox control",
	   "list - list mailbox contollers\n"
	   "mbox send <mbox_controller> <id> <data> - send mailbox message\n"
	   "mbox recv <mbox_controller> <id> [timeout] - recv mailbox message\n"
	   "- mbox_controller: Mailbox controller name\n"
	   "- id: ID of the mailbox client\n"
	   "- data: Message total 62 bytes(hex) (ex. AA BB..)\n"
	   "- timeout: optional timeout (us)\n");


typedef enum {
    LED_MODE_LIGHT = 0,
    LED_MODE_OFF,
    LED_MODE_BREATH,
    LED_MODE_FLICKER_ONCE,     //flicker once
    LED_MODE_FLICKER, //flicker
    LED_MODE_MAX
} E_LED_MODE;

#define LED_PWMGPIO_LIGHT    0xBB                       // Led will be lighting when power on
#define LED_PWMGPIO_DARK    0xBC                        // Led will turn off when power on
#define LED_PWMGPIO_BREATH      0xBD                    // Led will begin to breath when power on
#define LED_PWMGPIO_FLICKER     0xBE                    // Led will begin to flicker when power on
#define LED_PWMGPIO_FLICKER_POWERON    0xBF             // Led will begin to flicker when power on process


static int SetLEDMode(u8 u8LedMode,u8 u8LightRate)
{
    int bRet = 0;
    struct ofnode_phandle_args args;
    struct mbox_chan mbox;
    u8 send_box[10] = {0};

    /* fill of args */
	args.args[0] = 57;
	args.args_count = 1;

    /* get mbox channel */
	bRet = mbox_get_by_dev_name("mailbox_to_pm51", &args, &mbox);
	if (bRet) {
		printf("%s: mbox_get_by_dev_name failed: %d\n",
		       __func__, bRet);
		return CMD_RET_FAILURE;
	}

    if(u8LightRate > 100)
    {
        printf("u8LightRate more than 100,use defalut value\n");
        u8LightRate = 100;
    }

    switch(u8LedMode)
    {
        case LED_PWMGPIO_LIGHT:
            send_box[0] = LED_MODE_LIGHT << 4 | LED_MODE_LIGHT;
            send_box[2] = u8LightRate;
            break;
        case LED_PWMGPIO_DARK:
            send_box[0] = LED_MODE_OFF << 4 | LED_MODE_OFF;
            break;
        case LED_PWMGPIO_BREATH:
            send_box[0] = LED_MODE_BREATH << 4 | LED_MODE_LIGHT;
            send_box[1] = 0;      // stage1 time, 0 means off this stage
            send_box[2] = u8LightRate;
            send_box[6] = 5;
            send_box[7] = 5;
            send_box[8] = 5;
            send_box[9] = 5;
            break;
        case LED_PWMGPIO_FLICKER:
            send_box[0] = LED_MODE_FLICKER << 4 | LED_MODE_LIGHT;
            send_box[1] = 0;     //state1 Time,use 0 meaning don't use stage 1
            send_box[2] = u8LightRate;    //ligth rate(80%) light_rate: 80 (0~100%);
            send_box[6] = 6;     //state1 flink light time: 1s (/100ms)
            send_box[7] = 3;      //state1 flink dark time: 500ms (/100ms)
            break;
        case LED_PWMGPIO_FLICKER_POWERON:
            send_box[0] = LED_MODE_LIGHT << 4 | LED_MODE_FLICKER;
            send_box[1] = 18;     //state1 Time,use 0 meaning don't use stage 1
            send_box[2] = u8LightRate;    //ligth rate(80%) light_rate: 80 (0~100%);
            send_box[3] = 6;     //state1 flink light time: 1s (/100ms)
            send_box[4] = 3;      //state1 flink dark time: 500ms (/100ms)
            break;
        default:
            printf("default to LED_MODE_LIGHT \n");
            break;
    }

    bRet = mbox_send(&mbox, send_box);
    if (bRet) {
        printf("%s: mbox_send failed: %d\n",__func__, bRet);
        return CMD_RET_FAILURE;
    }

    return bRet;
}


int do_led(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    u8 u8LedMode, u8LightRate;

    if (argc < 2)
        return CMD_RET_USAGE;

    u8LedMode = (u8)simple_strtoul(argv[1], NULL, 16);
    u8LightRate = (u8)simple_strtoul(argv[2], NULL, 16);
    printf("u8LedMode 0x%x\n", u8LedMode);
    printf("u8LightRate 0x%x\n", u8LightRate);

    return SetLEDMode(u8LedMode, u8LightRate);
}


U_BOOT_CMD(mbox_led, CONFIG_SYS_MAXARGS, 0, do_led,
      "mailbox control led: \n",
      "mbox_led <LedMode> <LightRate>\n");

