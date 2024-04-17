# Git 使用指南

git是一个分布式版本控制系统，用于跟踪管理代码和文件的变更。

## Git与Github的SSH连接

### 初始化Git信息

`git config --global user.name "your name"`

`git config --global user.email "your email"`

### 生成SSH-Key

`ssh-keygen -t rsa -C "your email"`

登录`Github`并在`Setting -> SSH and GPG keys`中点击`New SSH key`，粘贴生成的`id_rsa.pub`即可。

### 检查SSH连接

`ssh -T git@github.com`

提示`You've successfully authenticated, but GitHub does not provide shell access.`即为SSH关联成功。

## Git子模块

添加一个远程仓库项目xxx子模块到一个主仓库项目中：`git submodule add xxx ./third_party/xxx` 

执行更新子模块，同步远程仓库的内容：`git submodule update --init --recursive`

可以查看`.gitmodules`中的内容，此处保存了子模块的信息。
