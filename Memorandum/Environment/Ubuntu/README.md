# Ubuntu环境配置
在`Ubuntu 22.04 LTS`下的个人环境配置。

## 常用软件
#### xfce4桌面系统
`sudo apt-get install xfce4`

#### i3-gaps桌面管理器  
`sudo add-apt-repository ppa:regolith-linux/release`

`sudo vim /etc/apt/sources.list.d/`

修改 `deb https://ppa.launchpadcontent.net/regolith-linux/release/ubuntu/ jammy main` 为 `deb https://ppa.launchpadcontent.net/regolith-linux/release/ubuntu/ devel main`

`sudo apt-get update`

`sudo apt-get install i3-gaps`

#### lightdm登录管理器  
`sudo apt-get install lightdm`

#### alacritty终端  
`sudo add-apt-repository ppa:aslatter/ppa`

`sudo apt-get update`

`sudo apt-get install alacritty`

## neofetch显示系统信息
`sudo apt-get install neofetch`

#### feh图像查看器  
`sudo apt-get install feh`

#### ranger文件管理器  
`sudo apt-get install ranger`

#### compton终端渲染器  
`sudo apt-get install compton`

#### screen终端托管  
`sudo apt-get install screen`

#### zsh  
`sudo apt-get install zsh`

#### oh-my-zsh  
`github: sh -c "$(wget https://raw.github.com/robbyrussell/oh-my-zsh/master/tools/install.sh -O -)"`

`gitee: sh -c "$(wget https://gitee.com/gloriaied/oh-my-zsh/raw/master/tools/install.sh -O -)"`

#### zsh插件
##### zsh-syntax-highlighting  
`github: git clone https://github.com/zsh-users/zsh-syntax-highlighting.git \${ZSH_CUSTOM:-~/.oh-my-zsh/custom}/plugins/zsh-syntax-highlighting`

`gitee: git clone https://gitee.com/asddfdf/zsh-syntax-highlighting.git \${ZSH_CUSTOM:-~/.oh-my-zsh/custom}/plugins/zsh-syntax-highlighting`

##### zsh-autosuggestions  
`github: git clone git://github.com/zsh-users/zsh-autosuggestions \$ZSH_CUSTOM/plugins/zsh-autosuggestions`

`gitee: git clone https://gitee.com/chenweizhen/zsh-autosuggestions.git \${ZSH_CUSTOM:-~/.oh-my-zsh/custom}/plugins/zsh-autosuggestions`

##### fzf-tab  
`github: git clone https://github.com/Aloxaf/fzf-tab ~ZSH_CUSTOM/plugins/fzf-tab`

##### powerlevel10k  
`github: git clone --depth=1 https://github.com/romkatv/powerlevel10k.git \${ZSH_CUSTOM:-$HOME/.oh-my-zsh/custom}/themes/powerlevel10k`

`gitee: git clone --depth=1 https://gitee.com/romkatv/powerlevel10k.git \${ZSH_CUSTOM:-$HOME/.oh-my-zsh/custom}/themes/powerlevel10k`

#### 修改字体  
在 `https://www.nerdfonts.com/font-downloads` 下载字体，解压后执行 `sudo cp *.ttf /usr/share/fonts`，使用`fc-cache`生成字体缓存即可。

#### ssr代理  
在 `https://github.com/shadowsocksrr/electron-ssr/releases` 下载。

#### proxhchains终端代理  
`sudo apt-get install proxychains`

#### libinput触摸板手势  
`sudo apt-get install libinput-tools`

#### fctix5中文输入法  
`sudo apt install fcitx5 fcitx5-chinese-addons fcitx5-frontend-gtk4 fcitx5-frontend-gtk3 fcitx5-frontend-gtk2 fcitx5-frontend-qt5`

`wget https://github.com/felixonmars/fcitx5-pinyin-zhwiki/releases/download/0.2.4/zhwiki-20220416.dict`

`mkdir -p ~/.local/share/fcitx5/pinyin/dictionaries/`

`mv zhwiki-20220416.dict ~/.local/share/fcitx5/pinyin/dictionaries/`

使用 `fcitx5-configtool` 配置语言和主题即可，黑色风格主题 `https://github.com/Reverier-Xu/FluentDark-fcitx5`。

在 `/etc/profile` 写入如下环境变量：

```
export XMODIFIERS=@im=fcitx
export GTK_IM_MODULE=fcitx
export QT_IM_MODULE=fcitx
```

在`clion`中输入框位置不对的问题，`Github`搜索并更新`jre runtime`即可解决。
