# FILE RESTORE DEMO
学习`rsync`,`librsync` 实现本地文件同步  
(https://github.com/WayneD/rsync)  
(https://github.com/librsync/librsync)  
## Env:linux
## dependence: libtool librsync-dev libssl-dev libacl1-dev libattr1-dev libc6-dev
 
***
# rysnc 阅读源码笔记
通过调试手段梳理rysnc如何实现文件权限、文件属主、文件属组、文件扩展属性、文件ACL、文件修改时间等同步流程

***

## 构建 
 
* 依赖 
    >     sudo apt install -y gcc g++ gawk autoconf automake python3-cmarkgfm
    >     sudo apt install -y acl libacl1-dev
    >     sudo apt install -y attr libattr1-dev
    >     sudo apt install -y libxxhash-dev
    >     sudo apt install -y libzstd-dev
    >     sudo apt install -y liblz4-dev
    >     sudo apt install -y libssl-dev

## 调试流程 

先测试除扩展属性以外的内容 rsync -aA test_dir/orig test_dir/abc

- -a: --archive  archive mode; equals -rlptgoD (no -H,-A,-X)  
    > -r 递归  
    > -l 拷贝软链接  
    > -p 保留权限  
    > -t 保留修改时间  
    > -g 保留属组  
    > -o 保留属主  
    > -D (-devices --specials) 保留设备文件与其他文件
- -A: 同步ACL
***
从`main.c/main`作为入口
1. 传入的参数都在 `options.c/parse_arguments` 中解析  

    -a 的参数解析  
    ```c
    #ifdef SUPPORT_LINKS
        preserve_links = 1;
    #endif
        preserve_perms = 1;
        preserve_mtimes = 1;
        preserve_gid = 1;
        preserve_uid = 1;
        preserve_devices = 1;
        preserve_specials = 1;
    ```  

    -A 的参数解析  
    ```c
    case 'A':
    #ifdef SUPPORT_ACLS
        preserve_acls = 1;
        preserve_perms = 1;
        break;
    #else
    ``` 

    这些配置参数都是定义在`options.c`中的全局变量, 通过对传入参数的解析会进修改这些配置参数进行初始化
2. 初始化`change_dir` 在父进程通过`getcwd`获取一下当前的工作目录，防止有的操作系统在子进程不能获取目录
3. 最后会通过`ret = start_client(argc, argv);`开启客户端
4. `main.c/start_client`会通过`options.c/check_for_hostspec`解析传入参数源路径的格式来判断是不是远程访问, 然后会检查目的路径是本地还是远程。如果是本地`check_for_hostspec`返回值是空。`local_server`置为**1**
    ```c
    if (!path) { /* no hostspec found, so src & dest are local */
        local_server = 1;
        if (filesfrom_host) {
            rprintf(FERROR,
                "--files-from cannot be remote when the transfer is local\n");
            exit_cleanup(RERR_SYNTAX);
        }
        shell_machine = NULL;
        rsync_port = 0;
    }
    ```
5. 接下来执行`do_cmd`会判断`local_server`, 如果为真,会调用`local_child`fork一个子进程作为本地服务端用于本地socket通信,每一个子进程有两个socket, 一个负责接收父进程的数据、一个负责向父进程写数据。父子进程关闭不需要用的fd,类似`pipe`。最后在子进程执行`child_main`执行`start_server`作为服务端。
    ```c
    int to_child_pipe[2];
	int from_child_pipe[2];
    ```
6. 父进程作为客户端执行`client_run` -> `send_file_list` -> `send_file_name` ->`get_acl` -> `send_acl`  
    rsync自己封装了stat结构,将文件系统携带的属性都包含进去了,`get_acl`、`send_acl`都是通过stat_x传递信息。
    ```c
    typedef struct {
        STRUCT_STAT st;
        time_t crtime;
    #ifdef SUPPORT_ACLS
        struct rsync_acl *acc_acl; /* access ACL */
        struct rsync_acl *def_acl; /* default ACL */
    #endif
    #ifdef SUPPORT_XATTRS
        item_list *xattr;
    #endif
    } stat_x;

    typedef struct rsync_acl {
        ida_entries names;
        /* These will be NO_ENTRY if there's no such entry. */
        uchar user_obj;
        uchar group_obj;
        uchar mask_obj;
        uchar other_obj;
    } rsync_acl;
    ```
7. int get_acl(const char *fname, stat_x *sxp)  
   STRUCT_STST st只有文件的st_mode,通过判断文件的类型, 以及`options.c`中的配置来判断是否获取该文件的acl, 
    - 普通文件与目录都是支持acl的
    - 软连接不支持acl
    - 特殊文件与设备文件是通过配置可选的  
    判断完文件类型进入`get_rsync_acl`根据文件类型， 如果是目录还需要额外获取default acl  
    然后判断是否是fake super用户, fake super 是通过配置文件配置的, 默认不开启。执行`sys_acl_get_file`, 该函数是对`acl_get_file`的封装。返回一个`acl_t`类型。rsync需要通过`unpack_smb_acl`将acl_t类型转换到rsync_acl类型。该函数通过对acl_t进行拆解,通过`acl_get_entry`,获取entry_p。然后进一步通过`sys_acl_get_info`获取entry_p的内容。`sys_acl_get_info`是对entry_p结构的拆解来获取具体的单个实例的类型、权限信息,如果实例的类型是认证用户或认证组，还需要额外获取对应id。

    ```c
    int sys_acl_get_info(SMB_ACL_ENTRY_T entry, SMB_ACL_TAG_T *tag_type_p, uint32 *bits_p, id_t *u_g_id_p)
    {
        acl_permset_t permset;

        if (acl_get_tag_type(entry, tag_type_p) != 0
        || acl_get_permset(entry, &permset) != 0)
            return -1;

        *bits_p = (acl_get_perm(permset, ACL_READ) ? 4 : 0)
            | (acl_get_perm(permset, ACL_WRITE) ? 2 : 0)
            | (acl_get_perm(permset, ACL_EXECUTE) ? 1 : 0);

        if (*tag_type_p == SMB_ACL_USER || *tag_type_p == SMB_ACL_GROUP) {
            void *qual;
            if ((qual = acl_get_qualifier(entry)) == NULL)
                return -1;
            *u_g_id_p = *(id_t*)qual;
            acl_free(qual);
        }

        return 0;
    }
    ```
    对于授权的用户和授权的组实体通过一个`item_list`存储,`item_list`中的`items`存放的是`id_access`, 最后所有实体都迭代完成需要将item_list转换到`rsync_acl`中的`ida_entries`中。这样一个文件的acl所有实体都存放到了`rysnc_acl`中。
    
    ```c
    typedef struct {
	    void *items;
	    size_t count;
	    size_t malloced;
    } item_list;

    typedef struct {
	    id_t id;
	    uint32 access;
    } id_access;
    
    typedef struct {
	    id_access *idas;
	    int count;
    } ida_entries;
    ```
    这样acl的信息就都存放到了`stat_x` 的`acc_acl`和`def_acl`中。
8. void send_acl(int f, stat_x *sxp)
    最后进行acl的发送，将stat_x中携带的acl信息发送到服务端中,这里有一个步骤`rsync_acl_strip_perms`需要将所有者、其他人的权限置空, 如果mask位是空，或者mask不为空但是也与`st_mode`的权限相同，那么所属组权限也需要置空。这么做的目的是因为这些权限可以通过文件的st_mode建立。下面对源码逻辑进行注释。

    ```c
    /* Removes the permission-bit entries from the ACL because these
    * can be reconstructed from the file's mode. */
    static void rsync_acl_strip_perms(stat_x *sxp)
    {
        rsync_acl *racl = sxp->acc_acl;

        racl->user_obj = NO_ENTRY; //所有者的权限不受mask影响，可以通过st_mode建立
        if (racl->mask_obj == NO_ENTRY) //如果mask为空， 说明没有扩展的用户和组，这时acl的组权限和st_mode的组权限是一致的,可以通过st_mode还原
            racl->group_obj = NO_ENTRY;
        else {//mask在，说明有扩展用户或组的权限, 这时mask会作为 acl_user acl_group acl_group_obj的最大权限，并且mask等于acl_group_obj的权限，也就是st_mode的组权限和mask是一致的
            int group_perms = (sxp->st.st_mode >> 3) & 7; //获取st_mode组权限
            if (racl->group_obj == group_perms)// acl的组权限和st_mode组权限一致，可以通过st_mode还原,否则acl组权限需要记下来
                racl->group_obj = NO_ENTRY;
    #ifndef HAVE_SOLARIS_ACLS //需要mask
            if (racl->names.count != 0 && racl->mask_obj == group_perms)
                racl->mask_obj = NO_ENTRY;
    #endif
        }
        racl->other_obj = NO_ENTRY; //其他人的权限不受mask影响， 可以通过st_mode建立
    }
    ```
9. 后面是服务端接收的部分`start_server` -> `do_server_recv` -> `change_dir` -> `recv_file_list` -> `recv_file_entry` -> `receive_acl` -> `recv_rsync_acl`   
    服务端会接收发送的数据，针对mask接收端也按照发送端的逻辑进行判断来恢复mask
    ```c
	if (duo_item->racl.names.count && duo_item->racl.mask_obj == NO_ENTRY) {
		/* Mask must be non-empty with lists. */
		if (type == SMB_ACL_TYPE_ACCESS)
			computed_mask_bits = (mode >> 3) & 7;
		else
			computed_mask_bits |= duo_item->racl.group_obj & ~NO_ENTRY;
		duo_item->racl.mask_obj = computed_mask_bits;
	}
    ```
10. 最后会通过`set_acl`将权限赋给临时文件,在该函数中会通过`rsync_acl_equal_enough`对比临时文件和同步文件的acl,如果一致，就不需要设置acl。mask依旧是对比的关键，mask存在时需要对比组权限,这是因为第一个参数是临时文件的acl，是完整的;而第二个参数是需要同步的文件的acl，有部分内容与`mode_t`重复都会被置空,但是当mask存在时`acl_group_obj`就不是真正的文件属组权限了。这时就需要第三个参数，也就是同步文件的mode_t来辅助判断。
    ```c
    static BOOL rsync_acl_equal_enough(const rsync_acl *racl1,
                    const rsync_acl *racl2, mode_t m)
    {
        if ((racl1->mask_obj ^ racl2->mask_obj) & NO_ENTRY)
            return False; /* One has a mask and the other doesn't */

        /* When there's a mask, the group_obj becomes an extended entry. */
        if (racl1->mask_obj != NO_ENTRY) {
            /* A condensed rsync_acl with a mask can only have no
            * group_obj when it was identical to the mask.  This
            * means that it was also identical to the group attrs
            * from the mode. */
            if (racl2->group_obj == NO_ENTRY) {
                if (racl1->group_obj != ((m >> 3) & 7))
                    return False;
            } else if (racl1->group_obj != racl2->group_obj)
                return False;
        }
        return ida_entries_equal(&racl1->names, &racl2->names);
    }

    ```
    如果acl不一致需要将同步的文件acl设置到临时文件,通过文件的mode将user_obj, group_obj, other_obj,mask_obj还原。
