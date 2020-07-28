/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein is
* confidential and proprietary to MediaTek Inc. and/or its licensors. Without
* the prior written permission of MediaTek inc. and/or its licensors, any
* reproduction, modification, use or disclosure of MediaTek Software, and
* information contained herein, in whole or in part, shall be strictly
* prohibited.
*
* MediaTek Inc. (C) 2010. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
* ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
* WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
* NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
* RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
* INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
* TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
* RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
* OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
* SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
* RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
* ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
* RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
* MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
* CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* The following software/firmware and/or related documentation ("MediaTek
* Software") have been modified by MediaTek Inc. All revisions are subject to
* any receiver's applicable license agreements with MediaTek Inc.
*/

#include <time.h>

#define YEAR_BASE 1900
#define EPOCH_YEAR 1970
#define EPOCH_WDAY 4

#define IS_LEAP_YEAR(year) (((year) % 4) == 0 && (((year) % 100) != 0 || ((year) % 400) == 0))

static const int days_per_month[2][12] = {{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
                                          {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};

static const int days_per_year[2] = {365, 366};

tm *gmtime_r(const time_t *timer, tm *timeptr) {
    uint32_t num_days, remain_sec;

    if (timer == NULL || timeptr == NULL) {
        return NULL;
    }

    num_days = (uint32_t)(*timer / 86400UL);
    remain_sec = (uint32_t)(*timer % 86400UL);

    timeptr->tm_sec = (int)(remain_sec % 60);
    timeptr->tm_min = (int)((remain_sec % 3600) / 60);
    timeptr->tm_hour = (int)(remain_sec / 3600);
    timeptr->tm_wday = (int)((num_days + EPOCH_WDAY) % 7); // 1970.1.1 => Thursday

    int year = EPOCH_YEAR;
    while (num_days >= (uint32_t)days_per_year[IS_LEAP_YEAR(year)]) {
        num_days -= days_per_year[IS_LEAP_YEAR(year)];
        year++;
    }
    timeptr->tm_year = year - YEAR_BASE;
    timeptr->tm_yday = num_days;

    int month;
    for (month = 0; num_days >= (uint32_t)days_per_month[IS_LEAP_YEAR(year)][month]; month++) {
        if (month > 11) {
            return NULL;
        }
        num_days -= days_per_month[IS_LEAP_YEAR(year)][month];
    }
    timeptr->tm_mon = month;
    timeptr->tm_mday = num_days + 1;

    timeptr->tm_isdst = 0;
    timeptr->tm_gmtoff = 0;
    timeptr->tm_zone = "UTC";

    return timeptr;
}
