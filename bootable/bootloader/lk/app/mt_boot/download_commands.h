#ifndef __DOWNLOAD_COMMANDS_H
#define __DOWNLOAD_COMMANDS_H
#include <platform/mt_typedefs.h>

void cmd_install_sig(const char *arg, void *data, unsigned sz);
BOOL cmd_flash_emmc_img(const char *arg, void *data, unsigned sz);
BOOL cmd_flash_emmc_sparse_img(const char *arg, void *data, unsigned sz);
void cmd_flash_emmc(const char *arg, void *data, unsigned sz);
void cmd_erase_emmc(const char *arg, void *data, unsigned sz);
BOOL cmd_flash_nand_img(const char *arg, void *data, unsigned sz);
void cmd_flash_nand(const char *arg, void *data, unsigned sz);
void cmd_erase_nand(const char *arg, void *data, unsigned sz);

#ifdef MTK_EMMC_SUPPORT
#define CHECK_SIZE 4*1024
#else
#include <platform/mtk_nand.h>
#define CHECK_SIZE (4*1024+128)
#define PARTITION_INFO_OFFSET 24
#define YAFFS_TAG_OFFSET 8
#endif

#define __le64 u64
#define __le32 u32
#define __le16 u16

#define __u64 u64
#define __u32 u32
#define __u16 u16
#define __u8 u8

#define EXT4_SUPER_MAGIC 0xEF53

struct ext4_super_block {
  __le32 s_inodes_count;
 __le32 s_blocks_count_lo;
 __le32 s_r_blocks_count_lo;
 __le32 s_free_blocks_count_lo;
  __le32 s_free_inodes_count;
 __le32 s_first_data_block;
 __le32 s_log_block_size;
 __le32 s_obso_log_frag_size;
  __le32 s_blocks_per_group;
 __le32 s_obso_frags_per_group;
 __le32 s_inodes_per_group;
 __le32 s_mtime;
  __le32 s_wtime;
 __le16 s_mnt_count;
 __le16 s_max_mnt_count;
 __le16 s_magic;
 __le16 s_state;
 __le16 s_errors;
 __le16 s_minor_rev_level;
  __le32 s_lastcheck;
 __le32 s_checkinterval;
 __le32 s_creator_os;
 __le32 s_rev_level;
  __le16 s_def_resuid;
 __le16 s_def_resgid;

 __le32 s_first_ino;
 __le16 s_inode_size;
 __le16 s_block_group_nr;
 __le32 s_feature_compat;
  __le32 s_feature_incompat;
 __le32 s_feature_ro_compat;
  __u8 s_uuid[16];
  char s_volume_name[16];
  char s_last_mounted[64];
  __le32 s_algorithm_usage_bitmap;

 __u8 s_prealloc_blocks;
 __u8 s_prealloc_dir_blocks;
 __le16 s_reserved_gdt_blocks;

  __u8 s_journal_uuid[16];
  __le32 s_journal_inum;
 __le32 s_journal_dev;
 __le32 s_last_orphan;
 __le32 s_hash_seed[4];
 __u8 s_def_hash_version;
 __u8 s_reserved_char_pad;
 __le16 s_desc_size;
  __le32 s_default_mount_opts;
 __le32 s_first_meta_bg;
 __le32 s_mkfs_time;
 __le32 s_jnl_blocks[17];

  __le32 s_blocks_count_hi;
 __le32 s_r_blocks_count_hi;
 __le32 s_free_blocks_count_hi;
 __le16 s_min_extra_isize;
 __le16 s_want_extra_isize;
 __le32 s_flags;
 __le16 s_raid_stride;
 __le16 s_mmp_interval;
 __le64 s_mmp_block;
 __le32 s_raid_stripe_width;
 __u8 s_log_groups_per_flex;
 __u8 s_reserved_char_pad2;
 __le16 s_reserved_pad;
 __le64 s_kbytes_written;
 __u32 s_reserved[160];
};

#endif
