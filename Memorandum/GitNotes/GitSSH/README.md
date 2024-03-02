# Git与Github的SSH连接

## 初始化Git信息

`git config --global user.name "your name"`

`git config --global user.email "your email"`

## 生成SSH-Key

`ssh-keygen -t rsa -C "your email"`

登录`Github`并在`Setting -> SSH and GPG keys`中点击`New SSH key`，粘贴生成的`id_rsa.pub`即可。

## 检查SSH连接

`ssh -T git@github.com`

提示`You've successfully authenticated, but GitHub does not provide shell access.`即为SSH关联成功。

