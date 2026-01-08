#include <assert.h>
#include <string.h>
#include "flash.h"
#include "tbbr_img_def.h"
#include "secureboot_arom_patch.h"
#include "ptable.h"
#include "loader.h"
#include "auth_nvctr.h"
#include "nvctr_flash.h"

/* Log configuration */
#include "log.h"
#define LOG_MODULE                    "NV-Flash"
#define LOG_LEVEL                     LOG_LEVEL_MAIN
/*---------------------------------------------------------------------------*/
static uint32_t flash_nv_ctr_addr = INVALID_FLASH_ADDRESS;
static bool flash_is_nand;
/*---------------------------------------------------------------------------*/
static uint32_t
page_size(void)
{
  const struct flash_operation *flash = flash_get_operation();
  const struct flash_info *info = flash->info();
  return info->page_size;
}
/*---------------------------------------------------------------------------*/
static uint32_t
sector_size(void)
{
  const struct flash_operation *flash = flash_get_operation();
  const struct flash_info *info = flash->info();
  return info->sector_size;
}
/*---------------------------------------------------------------------------*/
static uint32_t
pages_per_sector(void)
{
  return sector_size() / page_size();
}
/*---------------------------------------------------------------------------*/
static int
flash_nv_ctr_read(uint64_t addr, void *data, size_t size)
{
  const struct flash_operation *flash = flash_get_operation();
  return flash->read(addr, data, size);
}
/*---------------------------------------------------------------------------*/
static int
flash_nv_ctr_write(uint64_t addr, const void *data, size_t size)
{
  return
#ifdef USE_QSPI_NAND_FLASH
  flash_is_nand ? flash_nand_write_by_page(addr, data, size) :
#endif
  flash_nor_write_by_page(addr, data, size);
}
/*---------------------------------------------------------------------------*/
static int
flash_nv_ctr_erase(uint64_t addr, size_t size)
{
  return
#ifdef USE_QSPI_NAND_FLASH
  flash_is_nand ? flash_nand_erase_by_sector(addr, size) :
#endif
  flash_nor_erase_by_sector(addr, size);
}
/*---------------------------------------------------------------------------*/
/* NV Counters layout:
   _________________________________________________________________________
 | init_value | counter | swap counter| init_value | counter | swap counter|
 |____________|_________|_____________|____________|_________|_____________|
 |          page 0      |   last page |        page 0        | last page   |
 |______________________|_____________|______________________|_____________|
 |        primary  sector N           |        backup  sector N+1          |
 |____________________________________|____________________________________|

 */
/*---------------------------------------------------------------------------*/
static int
make_backup(void)
{
  flash_otp_nv_ctr_region_t primary_nv_ctr;
  size_t sz, rc;
  uint64_t swap_count_addr;

  if(flash_nv_ctr_addr == INVALID_FLASH_ADDRESS) {
    return_if_error(-1);
  }

  sz = sizeof(primary_nv_ctr.init_value) + sizeof(primary_nv_ctr.nv_counters);
  if(sz != (size_t)flash_nv_ctr_read(flash_nv_ctr_addr,
                                     &primary_nv_ctr, sz)) {
    return_if_error(-1);
  }

  sz = sizeof(primary_nv_ctr.swap_count);
  swap_count_addr = flash_nv_ctr_addr + (pages_per_sector() - 1) * page_size();
  if(sz != (size_t)flash_nv_ctr_read(swap_count_addr, &primary_nv_ctr.swap_count, sz)) {
    return_if_error(-1);
  }

  sz = flash_nv_ctr_erase(flash_nv_ctr_addr + sector_size(), sector_size());
  if(sz != sector_size()) {
    return_if_error(-1);
  }

  sz = sizeof(primary_nv_ctr.init_value) + sizeof(primary_nv_ctr.nv_counters);
  rc = flash_nv_ctr_write(flash_nv_ctr_addr + sector_size(), &primary_nv_ctr, sz);
  if(sz != rc) {
    return_if_error(-1);
  }

  sz = sizeof(primary_nv_ctr.swap_count);
  swap_count_addr = flash_nv_ctr_addr + sector_size() + (pages_per_sector() - 1) * page_size();
  rc = flash_nv_ctr_write(swap_count_addr, &primary_nv_ctr.swap_count, sz);
  if(sz != rc) {
    return_if_error(-1);
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
restore_backup(void)
{
  flash_otp_nv_ctr_region_t backup_nv_ctr;
  size_t sz, rc;
  uint32_t swap_count_addr;

  if(flash_nv_ctr_addr == INVALID_FLASH_ADDRESS) {
    return_if_error(-1);
  }

  sz = sizeof(backup_nv_ctr.init_value) + sizeof(backup_nv_ctr.nv_counters);
  if(sz != (size_t)flash_nv_ctr_read(flash_nv_ctr_addr + sector_size(),
                                     &backup_nv_ctr, sz)) {
    return_if_error(-1);
  }

  sz = sizeof(backup_nv_ctr.swap_count);
  swap_count_addr = flash_nv_ctr_addr + sector_size() + (pages_per_sector() - 1) * page_size();
  if(sz != (size_t)flash_nv_ctr_read(swap_count_addr,
                                     &backup_nv_ctr.swap_count, sz)) {
    return_if_error(-1);
  }

  sz = flash_nv_ctr_erase(flash_nv_ctr_addr, sector_size());
  if(sz != sector_size()) {
    return_if_error(-1);
  }

  sz = sizeof(backup_nv_ctr.init_value) + sizeof(backup_nv_ctr.nv_counters);
  rc = flash_nv_ctr_write(flash_nv_ctr_addr, &backup_nv_ctr, sz);
  if(sz != rc) {
    return_if_error(-1);
  }

  sz = sizeof(backup_nv_ctr.swap_count);
  swap_count_addr = flash_nv_ctr_addr + (pages_per_sector() - 1) * page_size();
  rc = flash_nv_ctr_write(swap_count_addr, &backup_nv_ctr.swap_count, sz);
  if(sz != rc) {
    return_if_error(-1);
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
create_or_restore_layout(void)
{
  flash_otp_nv_ctr_region_t primary_nv_ctr;
  flash_otp_nv_ctr_region_t backup_nv_ctr;
  size_t sz;
  int rc;
  uint32_t swap_count_addr;

  if(flash_nv_ctr_addr == INVALID_FLASH_ADDRESS) {
    return_if_error(-1);
  }

  sz = sizeof(backup_nv_ctr.init_value);
  rc = flash_nv_ctr_read(flash_nv_ctr_addr + sector_size(),
                         &backup_nv_ctr.init_value, sz);
  if(sz != (size_t)rc) {
    return_if_error(-1);
  }

  sz = sizeof(backup_nv_ctr.swap_count);
  swap_count_addr = flash_nv_ctr_addr + sector_size() + (pages_per_sector() - 1) * page_size();
  rc = flash_nv_ctr_read(swap_count_addr, &backup_nv_ctr.swap_count, sz);
  if(sz != (size_t)rc) {
    return_if_error(-1);
  }

  if(backup_nv_ctr.init_value == OTP_NV_COUNTERS_INITIALIZED &&
     backup_nv_ctr.swap_count != 0 &&
     backup_nv_ctr.swap_count != UINT32_MAX) { /* valid backup, restore */
    rc = restore_backup();
    return_if_error(rc);
  } else {
    /* No valid layouts, create from scratch */
    primary_nv_ctr.init_value = OTP_NV_COUNTERS_INITIALIZED;
    primary_nv_ctr.nv_counters[CHECK_NV_CTR_INDEX].nv_ctr = 0;
    primary_nv_ctr.swap_count = 1;

    sz = flash_nv_ctr_erase(flash_nv_ctr_addr, sector_size());
    if(sz != sector_size()) {
      return_if_error(-1);
    }

    sz = sizeof(primary_nv_ctr.init_value) + sizeof(primary_nv_ctr.nv_counters);
    rc = flash_nv_ctr_write(flash_nv_ctr_addr, &primary_nv_ctr, sz);
    if(sz != (size_t)rc) {
      return_if_error(-1);
    }

    sz = sizeof(primary_nv_ctr.swap_count);
    swap_count_addr = flash_nv_ctr_addr + (pages_per_sector() - 1) * page_size();
    rc = flash_nv_ctr_write(swap_count_addr, &primary_nv_ctr.swap_count, sz);
    if(sz != (size_t)rc) {
      return_if_error(-1);
    }

    rc = make_backup();
    return_if_error(rc);
  }

  return 0;
}
/*
   NV Counters layout:
   _________________________________________________________________________
 | init_value | counter | swap counter| init_value | counter | swap counter|
 |____________|_________|_____________|____________|_________|_____________|
 |			page 0		|	page 1	  |      page 0		 | page 1	   |
 |______________________|_____________|______________________|_____________|
 |	   primary	sector 0			  |      backup  sector 1        |
 |____________________________________|____________________________________|

 */
/*---------------------------------------------------------------------------*/
static int
init_nv_ctr_from_partition(void)
{
  flash_otp_nv_ctr_region_t primary_nv_ctr;
  flash_otp_nv_ctr_region_t backup_nv_ctr;
  size_t sz;
  int rc;
  uint32_t swap_count_addr;

  struct ptentry *pt_nv = ptable_find_entry(NV_CTR_PART_NAME);
  if(!pt_nv) {/*Maybe, the board is erased all(empty). */
    LOG_WARN("Not found %s partition\n", NV_CTR_PART_NAME);
    return NO_NV_COUNTER_PARTITION;
  }
  flash_nv_ctr_addr = pt_nv->start;

  sz = sizeof(primary_nv_ctr.init_value);
  if(sz != (size_t)flash_nv_ctr_read(flash_nv_ctr_addr,
                                     &primary_nv_ctr.init_value, sz)) {
    return_if_error(-1);
  }

  sz = sizeof(primary_nv_ctr.swap_count);
  /*swap count store in the last page of a sector. */
  swap_count_addr = flash_nv_ctr_addr + (pages_per_sector() - 1) * page_size();
  if(sz != (size_t)flash_nv_ctr_read(swap_count_addr, &primary_nv_ctr.swap_count, sz)) {
    return_if_error(-1);
  }

  if(primary_nv_ctr.init_value != OTP_NV_COUNTERS_INITIALIZED ||
     (primary_nv_ctr.swap_count == 0 || primary_nv_ctr.swap_count == UINT32_MAX)) {
    rc = create_or_restore_layout();
    return_if_error(rc);
  } else {
    sz = sizeof(uint32_t);
    swap_count_addr = flash_nv_ctr_addr + sector_size() + (pages_per_sector() - 1) * page_size();
    if(sz != (size_t)flash_nv_ctr_read(swap_count_addr, &backup_nv_ctr.swap_count, sz)) {
      return_if_error(-1);
    }

    if(backup_nv_ctr.swap_count == 0 ||
       backup_nv_ctr.swap_count == UINT32_MAX ||
       backup_nv_ctr.swap_count < primary_nv_ctr.swap_count ||
       (backup_nv_ctr.swap_count == (UINT32_MAX - 1) && primary_nv_ctr.swap_count == 1)) {
      rc = make_backup();
      return_if_error(rc);
    }
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
int
plat_init_nv_ctr(void)
{
  const struct flash_operation *flash = flash_get_operation();
  const struct flash_info *info = flash->info();
  int rc;

  if(!strncmp(info->device_type, FLASH_TYPE_QSPI_NAND, MAX_FLASH_DEVICE_TYPE)) {
    flash_is_nand = true;
  }

  if(info == NULL
     || flash == NULL
     || flash->read == NULL || flash->write == NULL) {
    LOG_ERR("Flash device not initialized\n");
    return_if_error(-1);
  }

  /* init_value and nv_counters occupation <= page size. */
  if(page_size() < (sizeof(flash_otp_nv_ctr_region_t) - sizeof(uint32_t))) {
    return_if_error(-1);
  }

  if(ptable_loader() == 0) {
    rc = init_nv_ctr_from_partition();
    return_if_error(rc);
  } else {
    LOG_WARN("No %s partition\n", NV_CTR_PART_NAME);
    return NO_NV_COUNTER_PARTITION;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
int
plat_get_nv_ctr(void *cookie, nvctr_t *nv_ctr)
{
  const char *oid;
  size_t sz;
  flash_otp_nv_ctr_region_t primary_nv_ctr;

  assert(cookie != NULL);
  assert(nv_ctr != NULL);

  oid = (const char *)cookie;
  if(strcmp(oid, TRUSTED_FW_NVCOUNTER_OID) != 0) {
    return_if_error(-1);
  }

  if(flash_nv_ctr_addr == INVALID_FLASH_ADDRESS) {
    nv_ctr->nv_ctr = 0; /*Set default value. Maybe, ptable is empty on board. */
    return 0;
  }

  sz = sizeof(primary_nv_ctr.init_value) + sizeof(primary_nv_ctr.nv_counters);
  if(sz != (size_t)flash_nv_ctr_read(flash_nv_ctr_addr, &primary_nv_ctr, sz)) {
    return_if_error(-1);
  }

  *nv_ctr = primary_nv_ctr.nv_counters[CHECK_NV_CTR_INDEX];

  return 0;
}
/*---------------------------------------------------------------------------*/
int
plat_set_nv_ctr(void *cookie, nvctr_t nv_ctr)
{
  const char *oid;
  flash_otp_nv_ctr_region_t primary_nv_ctr;
  size_t sz;
  int rc;
  uint32_t swap_count_addr;

  assert(cookie != NULL);
  assert(nv_ctr != NULL);

  oid = (const char *)cookie;
  if(strcmp(oid, TRUSTED_FW_NVCOUNTER_OID) != 0) {
    return_if_error(-1);
  }

  struct ptentry *pt_nv = ptable_find_entry(NV_CTR_PART_NAME);
  if(!pt_nv) { /* No nv ctr partition in upload and erase-all mode. */
    LOG_WARN("Not found %s partition, continue...\n", NV_CTR_PART_NAME);
    return 0;
  }

  if(flash_nv_ctr_addr == INVALID_FLASH_ADDRESS) {
    primary_nv_ctr.swap_count = 1;
  } else {
    sz = sizeof(primary_nv_ctr.swap_count);
    swap_count_addr = flash_nv_ctr_addr + (pages_per_sector() - 1) * page_size();
    if(sz != (size_t)flash_nv_ctr_read(swap_count_addr,
                                       &primary_nv_ctr.swap_count, sz)) {
      return_if_error(-1);
    }
    primary_nv_ctr.swap_count++;
  }

  /*Reset flash_nv_ctr_addr from the new ptable. */
  flash_nv_ctr_addr = pt_nv->start;

  sz = flash_nv_ctr_erase(flash_nv_ctr_addr, sector_size());
  if(sz != sector_size()) {
    return_if_error(-1);
  }

  primary_nv_ctr.init_value = OTP_NV_COUNTERS_INITIALIZED;
  primary_nv_ctr.nv_counters[CHECK_NV_CTR_INDEX] = nv_ctr;
  sz = sizeof(primary_nv_ctr.init_value) + sizeof(primary_nv_ctr.nv_counters);
  rc = flash_nv_ctr_write(flash_nv_ctr_addr, &primary_nv_ctr, sz);
  if(sz != (size_t)rc) {
    return_if_error(-1);
  }

  if(primary_nv_ctr.swap_count == 0 ||
     primary_nv_ctr.swap_count == UINT32_MAX) {
    primary_nv_ctr.swap_count = 1;
  }

  sz = sizeof(primary_nv_ctr.swap_count);
  swap_count_addr = flash_nv_ctr_addr + (pages_per_sector() - 1) * page_size();
  rc = flash_nv_ctr_write(swap_count_addr, &primary_nv_ctr.swap_count, sz);
  if(sz != (size_t)rc) {
    return_if_error(-1);
  }

  rc = make_backup();
  return_if_error(rc);

  return 0;
}
/*---------------------------------------------------------------------------*/
