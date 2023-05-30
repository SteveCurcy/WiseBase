#### git是分布式管理工具

### git的基本使用方法

![image-20230530103701172](https://p.ipic.vip/a78tdo.png)

- #### Git 基本命令

  - ##### 配置

    1. **git** **config** 

       - `-- global`
       - `--system`
       - `--local`

       常见Git 配置

       1. 用户名配置 

          ```git
          git config --global user.name "cgs"
          git config --global user.email changguangsheng@iie.ac.cn
          ```

          

       2. instead of 配置

          ```
          git config --global url.git@github.com:.insteadOf https://github.com/
          ```

          

       3. Git 命令别名配置

          ```git
          git config --global alias.cin "commit --amend --no-edit"
          ```

          别名配置，用cin 代替 commit --amend --no-edit

    2. **git** **remote**

       1. 本地和远端仓库的信息

          ```
          git remote -v  查看remote 信息
          添加remote
          git remote add origin_ssh git@github.com:git/git.git
          git remote add origin_http https://github.com/git/git.git
          同一个origin 设置不同的Push和Fetch URL
          git remote add origin https://github.com/git/git.git
          git remote set-url --push origin git@github.com:changguangsheng/gitDemo.git
          ```

       2. HTTP Remote

          ```
          免密配置
          内存：git config --global credential.helper 'cache --timeout-3600'
          ```

       3. SSH Remote

          URL: git@github.com:git/git.git

          免密配置

          SSH可以通过公私钥 的机制，将生成公钥存放在服务端，从而实现免密访问

          目前的Key的类型有四种， dsa、rsa、 ecdsa、 ed25519

          默认使用的是rsa， 推荐使用 ed25519

          ```
          ssh -keygen -t ed25519 -C "enmail@.com"
           
          ```

          

    3. **git init**

       项目初始化。

       ```git
       git init --initial-branch <名称>    初始化的分支 
       ```

       ```git
       git init --bare    创建一个裸仓库（纯Git目录， 没有工作目录）
       ```

       ```git
       git init --template <模版目录> 可以通过模版来创建预先构建好的自定义git目录
       ```

       

  - ##### 提交代码

    1. **git add**

       ``` 
       git add.
       ```

       <img src="https://p.ipic.vip/zftamh.png" alt="image-20230518144110552" style="zoom:50%;" />

    2. **git commit**

       ```go
       git commit -m "add readme"
       
       git cat-file -p 8a6e869343c5f3be0fc5021b1e68da69240c1cf3
        
       git cat-file -p acf407dfb668aa39166fbd11a2015d125401dadf
       ```

       <img src="https://p.ipic.vip/u3txw3.png" alt="image-20230518145252630" style="zoom:50%;" />

       - **Objects**

         Commit/ tree/blob 在git中 统一称为Object， 除此之外还有个tag 的object

         - Blob 存储文件的内容
         - Tree 存储文件的目录
         - Commit 存储提交信息，一个Commit可以对应唯一版本的代码

         **PS：** 如何把这个三个信息串联在一起

         1. 通过Commit 寻找到Tree 信息， 每个Commit 都会存储对应的Tree ID
         2. 通过Tree 存储的信息，获取到对应的目录树信息。
         3. 从Tree 中获得blob的ID， 通过Blob ID获取对应的文件内容。

       - **Refs**

         Refs 文件存储的内容， refs的内容 就是对应的Commit ID。

         ref当作指针， 指向对应的Commit 来表示当前Ref对应的版本。

         refs/heads 前缀表示的是分支， refs/tags前缀表示的是标签

       - **Branch**

         分支用于开发阶段，是可以不断添加Commit 进行迭代的

         ```
         git checkout -b <分支名>创建一个新分支
         ```

       - **Tag**

         标签一般表示的是一个稳定版本， 指向的Commit 一般不会变更

         通过git tag 生成tag

         ```
         git tag v0.0.1 
         ```

       - **Annotation Tag**

         附注标签， 一种特殊的Tag，可以给Tag提供一些额外的信息。

         ```
         git tag -a 命令来完成附注标签的创建
         查看该tag object 的内容
         git tag -a v0.0.2 -m "add feature 1"
         ```

         ![image-20230518151006257](https://p.ipic.vip/t5653a.png)

       - **追溯历史版本**

         - 获取当前版本代码

           通过ref指向的Commit 可以获取唯一的代码版本。

         - 获取历史版本代码

           Commit 里面会存有 parent commit 字段，通过commit 的串联获取历史版本代码。

           1.  修改文件，提交，创建新的commit

              ![image-20230518152050385](https://p.ipic.vip/xbp8ha.png)

           2. 查看最新的commit，新增了parent 信息。

              ![image-20230518152147985](https://p.ipic.vip/4fkwym.png)

       - **修改历史版本**

         1.  commit --amend

            通过这个命令可以修改最近一次commit 信息， 修改之后commit id会变

            ```
            git commit -amend 
            ```

         2. rebase

            通过 git rebase -i HEAD~3 可以实现对最近三个commit 的修改：

            1. 合并commit
            2. 修改具体的commit message
            3. 删除某个commit

         3. filter --branch

            该命令可以指定删除所有提交中的某个文件或者全局修改邮箱地址等操作

       - **Objects**

         - 新增的Object

           修改Commit 后我们可以发现git object 出现了变化

           新增了 commit object

         - 悬空的Object

           没有ref 指向的object

           ```
           git fsck --lost-found
           ```

           ![image-20230518153102957](https://p.ipic.vip/2nu2qa.png)

       - **Git GC**

         - GC

           通过git gc命令，可以删除一些不需要的object， 以及会对object 进行一些 打包压缩来减少仓库的体积。

         - Reflog

           reflog 用于记录操作日志， 防止误操作后数据丢失， 通过reflog来找到丢失的数据，手动将日志设置为过期

         - 指定时间

           ```
           git gc prune=now 指定的是修剪多久之前的对象，默认是默认是两周前
           ```

           <img src="https://p.ipic.vip/kif2mp.png" alt="image-20230518153823776" style="zoom:50%;" />

         

  - ##### 远端同步

    - **拉取代码**

      1.  **git clone**

         拉取完整的仓库到本地目录，可以指定分支，深度。

      2. **git fetch**

         将远端某些分支最新代码拉取到本地， 不会执行merge 操作。

         会修改/refs/remote 内的分支信息， 如果需要和本地代码合并需要手动操作。

      3. **git full **

         拉取远端某分支，并和本地代码进行合并， 操作等同于git fetch + git merge， 

         也可以通过git pull --rebase 完成 git fetch + git rebase 操作。

         可能存在冲突，需要解决冲突。

      > git fetch:是从远程获取最新版本到本地，不会自动merge.
      > 而git pull是从远程获取最新版本并merge到本地仓库.
      > 从安全角度出发，git fetch比git pull更安全，因为我们可以先比较本地与远程的区别后，选择性的合并。
      > git push 默认推送到master，如果有多个分支，则多个分支一起推送到远程.
    
    - **推送代码**
    
      1. **git push**
    
         push 是将本地代码同步至远端的方式。
    
         ```
         git push origin master 命令
         ```

### Git研发流程

- #### 集中式工作流 （Gerrit）

  值依托于master 分支进行研发活动

  - ##### 工作方式

    1. 获取远端master代码
    2. 直接在master分支完成修改
    3. 提交前拉取最新的master 代码和本地代码进行合并， 如果有冲突解决冲突
    4. 提交本地代码到master

- #### 分支管理工作流

  - ##### Git Flow

    分支类型丰富，规范严格

    1. Master ： 主干分支
    2. Develop： 开发分支
    3. Feature： 特性分支
    4. Release： 发布分支
    5. Hotfix： 热修复

    

  - ##### Github Flow

    只有主干分支和开发分支，规则简单

    只有一个主干分支， 基于Pull Request 往主干分支中提交代码。

     

  - ##### Gitlab Flow

    在主干分支和开发分支智商构建环境分支，版本分支， 满足不同发布or 环境的需要





## 参考

[git pull和git fetch的区别](https://www.zhihu.com/question/38305012)
