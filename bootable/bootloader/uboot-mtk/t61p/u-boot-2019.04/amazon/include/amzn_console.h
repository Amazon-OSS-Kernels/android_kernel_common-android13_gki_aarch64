/*
 * Copyright (c) 2023 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 */

#ifndef __AMZN_CONSOLE_H
#define __AMZN_CONSOLE_H

#define AMZN_LOG_SIZE  (8*1024)      /* 8K log buffer */
#define AMZN_LOG_MAGIC (0x01ab1260)  /* margic number indicating log initialized */
int amzn_init_log_buf(void);         /* initialize log buffer */
void amzn_save_log(const char *s);   /* save log to the buffer */
void amzn_get_log(char *log_buf);    /* get the content of the log buffer */

int amzn_uart_disable(void);

#endif
