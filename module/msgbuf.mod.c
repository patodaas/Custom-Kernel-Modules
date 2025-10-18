#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x12cfb334, "seq_printf" },
	{ 0x357aaab3, "mutex_lock_interruptible" },
	{ 0xa61fd7aa, "__check_object_size" },
	{ 0x092a35a2, "_copy_to_user" },
	{ 0xb9e81daf, "proc_remove" },
	{ 0xc68d7731, "device_destroy" },
	{ 0xfbc10eaa, "class_destroy" },
	{ 0xd2a864c6, "cdev_del" },
	{ 0x0bc5fb0d, "unregister_chrdev_region" },
	{ 0xcb8b6ec6, "kfree" },
	{ 0xbd03ed67, "random_kmalloc_seed" },
	{ 0x4ac4312d, "kmalloc_caches" },
	{ 0x8d1d7639, "__kmalloc_cache_noprof" },
	{ 0x9f222e1e, "alloc_chrdev_region" },
	{ 0xeb9d7920, "cdev_init" },
	{ 0xf212d1ce, "cdev_add" },
	{ 0x3d568d84, "class_create" },
	{ 0x0cf2b0e8, "device_create" },
	{ 0xf8d7ac5e, "proc_create" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0x092a35a2, "_copy_from_user" },
	{ 0xd22cd56f, "seq_read" },
	{ 0x388dee05, "seq_lseek" },
	{ 0xae030cd0, "single_release" },
	{ 0x1d177ede, "default_llseek" },
	{ 0xd272d446, "__fentry__" },
	{ 0x96c07e76, "const_pcpu_hot" },
	{ 0xe8213e80, "_printk" },
	{ 0xde338d9a, "_raw_spin_lock" },
	{ 0xde338d9a, "_raw_spin_unlock" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0x5218fe90, "single_open" },
	{ 0xf46d5bf3, "mutex_lock" },
	{ 0xf46d5bf3, "mutex_unlock" },
	{ 0x70eca2ca, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0x12cfb334,
	0x357aaab3,
	0xa61fd7aa,
	0x092a35a2,
	0xb9e81daf,
	0xc68d7731,
	0xfbc10eaa,
	0xd2a864c6,
	0x0bc5fb0d,
	0xcb8b6ec6,
	0xbd03ed67,
	0x4ac4312d,
	0x8d1d7639,
	0x9f222e1e,
	0xeb9d7920,
	0xf212d1ce,
	0x3d568d84,
	0x0cf2b0e8,
	0xf8d7ac5e,
	0xd272d446,
	0x092a35a2,
	0xd22cd56f,
	0x388dee05,
	0xae030cd0,
	0x1d177ede,
	0xd272d446,
	0x96c07e76,
	0xe8213e80,
	0xde338d9a,
	0xde338d9a,
	0xd272d446,
	0x5218fe90,
	0xf46d5bf3,
	0xf46d5bf3,
	0x70eca2ca,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"seq_printf\0"
	"mutex_lock_interruptible\0"
	"__check_object_size\0"
	"_copy_to_user\0"
	"proc_remove\0"
	"device_destroy\0"
	"class_destroy\0"
	"cdev_del\0"
	"unregister_chrdev_region\0"
	"kfree\0"
	"random_kmalloc_seed\0"
	"kmalloc_caches\0"
	"__kmalloc_cache_noprof\0"
	"alloc_chrdev_region\0"
	"cdev_init\0"
	"cdev_add\0"
	"class_create\0"
	"device_create\0"
	"proc_create\0"
	"__stack_chk_fail\0"
	"_copy_from_user\0"
	"seq_read\0"
	"seq_lseek\0"
	"single_release\0"
	"default_llseek\0"
	"__fentry__\0"
	"const_pcpu_hot\0"
	"_printk\0"
	"_raw_spin_lock\0"
	"_raw_spin_unlock\0"
	"__x86_return_thunk\0"
	"single_open\0"
	"mutex_lock\0"
	"mutex_unlock\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "E19ADF07CC4B0440E51AD44");
