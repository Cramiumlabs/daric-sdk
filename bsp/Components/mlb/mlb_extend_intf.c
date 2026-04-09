#include "mlb_error_code.h"
#include "mlb_platform.h"
#include "mlb_intf.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/_intsup.h>
#include <sys/types.h>

#ifndef LOGD
#ifdef MLB_DEBUG
#define LOGD(fmt, ...) mlb_plat_log("[DBG] " fmt, ##__VA_ARGS__)
#else
#define LOGD(...)
#endif
#endif

#ifndef LOGE
#define LOGE(fmt, ...) mlb_plat_log("[ERR] " fmt, ##__VA_ARGS__)
#endif

typedef struct _mlb_user_t{
    uint16_t uid;
    uint8_t used;
    uint8_t fid_num;
    uint16_t fid_list[MLB_MAX_FID_PER_USER];
}mlb_user_t;

typedef struct mlb_user_list_t{
    uint16_t crc;
    mlb_user_t user_list[MLB_USER_MAX_NUM];
    int active_user;
}mlb_user_list_t;

mlb_user_list_t *user_table = NULL;

uint16_t mlb_blk_crc16(void *buffer, size_t size)
{
    static const uint16_t rtable[256] = {
        0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
        0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
        0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
        0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
        0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
        0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
        0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
        0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
        0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
        0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
        0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
        0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
        0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
        0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
        0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
        0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
        0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
        0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
        0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
        0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
        0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
        0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
        0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
        0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
        0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
        0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
        0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
        0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
        0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
        0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
        0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
        0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
    };
    uint16_t crc = 0;
    uint8_t *data = (uint8_t *)buffer;
    for (size_t i = 0; i < size; i++) {
        crc = (crc >> 8) ^ rtable[(crc ^ data[i]) & 0xff];
    }
    return crc;
}

MRT_T mlb_intf_set_active_user(uint16_t uid)
{
    int i;
    uint16_t calc_crc = 0; 

    if(!user_table) {
        user_table = malloc(sizeof(mlb_user_list_t));
        mlb_plat_flash_read(MLB_USER_DATA_FLASH_OFFSET, user_table, sizeof(mlb_user_list_t));
        calc_crc = mlb_blk_crc16(((uint8_t *)user_table) + 2, sizeof(mlb_user_list_t) - 2);
        if(user_table->crc != calc_crc)
        {
            //set default value
            LOGD("init default user data");
            memset(user_table, 0, sizeof(mlb_user_list_t));
            user_table->active_user = 0;
            user_table->user_list[0].uid = uid;
            user_table->user_list[0].used = 1;
             user_table->user_list[0].fid_num = 0;
            memset(user_table->user_list[0].fid_list, 0xff, MLB_MAX_FID_PER_USER * 2);
            user_table->crc = mlb_blk_crc16(((uint8_t *)user_table) + 2, sizeof(mlb_user_list_t) - 2);
            mlb_plat_flash_block_write(MLB_USER_DATA_FLASH_OFFSET, user_table, sizeof(mlb_user_list_t));
            return MRT_OK;
        }
    }

    for(i = 0; i < MLB_USER_MAX_NUM; i++)
    {
        // set active already exist user
        if(user_table->user_list[i].uid == uid && user_table->user_list[i].used)
        {
            user_table->active_user = i;
            LOGD("set user_list[%d].uid %d", i, uid);
            return MRT_OK;
        }
    }

    for (i=0; i<MLB_USER_MAX_NUM; i++) {
        mlb_user_t *user = &user_table->user_list[i];
        if (!user->used) {
            user->used = 1;
            user->uid = uid;
            user->fid_num = 0;
            memset(user->fid_list, 0xff, sizeof(user->fid_list));
            user_table->active_user = i;
            user_table->crc = mlb_blk_crc16(((uint8_t *)user_table) + 2, sizeof(mlb_user_list_t) - 2);
            mlb_plat_flash_block_write(MLB_USER_DATA_FLASH_OFFSET, user_table, sizeof(mlb_user_list_t));
            return MRT_OK;
        }
    }

    LOGE("user full");

    return MRT_USER_FULL;
}

MRT_T mlb_intf_get_current_user(void)
{
    int active = user_table->active_user;
    return user_table->user_list[active].uid;
}

void mlb_intf_get_all_users(uint16_t *uids, uint16_t *uid_num)
{
    uint16_t tmp_idx = 0;

    for (int i=0; i<MLB_USER_MAX_NUM; i++) {
        if (user_table->user_list[i].used) {
            uids[tmp_idx++]=user_table->user_list[i].uid;
        }
    }

    *uid_num = tmp_idx;
}

MRT_T mlb_intf_get_current_user_fids(uint16_t *fids, uint8_t *fid_num)
{
    int active = user_table->active_user;

    LOGD("current actived user_list[%d].uid %d", active, user_table->user_list[active].uid);

    *fid_num = user_table->user_list[active].fid_num;
    memcpy(fids, user_table->user_list[active].fid_list, MLB_MAX_FID_PER_USER * 2);

    return MRT_OK;
}

MRT_T mlb_intf_get_a_user_fids(uint16_t uid, uint16_t *fids, uint8_t *fid_num)
{
    bool find = false;
    *fid_num = 0;

    for (int i=0; i<MLB_USER_MAX_NUM; i++) {
        mlb_user_t* user = &user_table->user_list[i];
        if (user->uid == uid && user->used) {
            find = true;
            for (int j=0; j<user->fid_num; j++) {
                fids[j] = user->fid_list[j];
                (*fid_num)++;
            }
            break;
        }
    }

    if (!find) {
        return  MRT_USER_NOT_FOUND;
    }

    return MRT_OK;
}

MRT_T mlb_intf_set_user_fid(uint16_t fid)
{
    int active = user_table->active_user;
    int fid_num = user_table->user_list[active].fid_num;

    if(fid_num >= MLB_MAX_FID_PER_USER)
    {
        LOGE("user fid full");
        return MRT_USER_FID_FULL;
    }

    LOGD("set fid %d to user %d", fid, user_table->user_list[active].uid);
    user_table->user_list[active].fid_list[fid_num] = fid;
    user_table->user_list[active].fid_num++;
    LOGD("user_list[%d].fid_num %d", active, user_table->user_list[active].fid_num);
    user_table->crc = mlb_blk_crc16(((uint8_t *)user_table) + 2, sizeof(mlb_user_list_t) - 2);
    mlb_plat_flash_block_write(MLB_USER_DATA_FLASH_OFFSET, user_table, sizeof(mlb_user_list_t));

    return MRT_OK;
}

MRT_T mlb_intf_delete_user_fid(uint16_t fid) 
{
    MRT_T ret = MRT_OK;
    int active = user_table->active_user;
    int fid_num = user_table->user_list[active].fid_num;

    for(int i = 0; i < fid_num; i++)
    {
        if(user_table->user_list[active].fid_list[i] == fid)
        {
             mlb_intf_tpl_remove(fid);
            if(ret != MRT_OK){
                LOGE("remove fid fail %d", ret);
                return ret;
            }
            user_table->user_list[active].fid_list[i] = user_table->user_list[active].fid_list[fid_num - 1];
            user_table->user_list[active].fid_list[fid_num - 1] = 0xff;
            user_table->user_list[active].fid_num--;
            user_table->crc = mlb_blk_crc16(((uint8_t *)user_table) + 2, sizeof(mlb_user_list_t) - 2);
            mlb_plat_flash_block_write(MLB_USER_DATA_FLASH_OFFSET, user_table, sizeof(mlb_user_list_t));
            return MRT_OK;
        }
    }

    LOGE("user fid %d not found", fid);

    return MRT_USER_FID_NOT_FOUND;
}

MRT_T mlb_intf_delete_user_all_fids() {
    MRT_T ret = MRT_OK;

    int active = user_table->active_user;
    mlb_user_t *user = &user_table->user_list[active];
    
    ret = mlb_intf_tpl_remove_fids(user->fid_list, user->fid_num);
    if (ret != MRT_OK) {
        LOGE("Delete fids failed. ret=%d", ret);
        return ret;
    }
    user->fid_num = 0;
    memset(user->fid_list, 0xff, MLB_MAX_FID_PER_USER * 2);
    user_table->crc = mlb_blk_crc16(((uint8_t *)user_table) + 2, sizeof(mlb_user_list_t) - 2);
    ret = mlb_plat_flash_block_write(MLB_USER_DATA_FLASH_OFFSET, user_table, sizeof(mlb_user_list_t));
    if (ret != MRT_OK) {
        LOGE("Update user info failed. ret=%d", ret);
    }
    return ret;
}

MRT_T mlb_intf_delete_user(uint16_t uid)
{
    MRT_T ret = MRT_OK;
    int active = user_table->active_user;

    if(uid == user_table->user_list[active].uid)
    {
        LOGE("user %d actived, not allow delete", uid);
        return MRT_USER_DELETE_ERROR;
    }

    for(int i = 0; i < MLB_USER_MAX_NUM; i++)
    {
        mlb_user_t *user = &user_table->user_list[i];
        if(user->used && user->uid == uid)
        {
            ret = mlb_intf_tpl_remove_fids(user_table->user_list[i].fid_list, user_table->user_list[i].fid_num);
            if(ret != MRT_OK) 
            {
                LOGE("delete user %d fids error", uid);
                return MRT_USER_DELETE_ERROR;
            }
            user->used = 0;
            user->uid = 0;
            user->fid_num = 0;
            memset(user->fid_list, 0xff, MLB_MAX_FID_PER_USER * 2);
            user_table->crc = mlb_blk_crc16(((uint8_t *)user_table) + 2, sizeof(mlb_user_list_t) - 2);
            mlb_plat_flash_block_write(MLB_USER_DATA_FLASH_OFFSET, user_table, sizeof(mlb_user_list_t));
            return MRT_OK;
        }
    }

    LOGE("user %d not found", uid);

    return MRT_USER_NOT_FOUND;
}